// Copyright (c) Acconeer AB, 2018-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "acc_driver_pm_same70.h"

#include "chip_pins.h"
#include "FreeRTOS.h"
#include "samv71.h"
#include "task.h"
#include "irq/irq.h"
#include "irq/nvic.h"
#include "peripherals/rtt.h"
#include "portmacro.h"

#include "acc_device_gpio.h"
#include "acc_device_os.h"
#include "acc_device_pm.h"
#include "acc_device_uart.h"
#ifdef ACC_CFG_ENABLE_TRACECLOCK
#include "acc_driver_traceclock_cmx.h"
#endif
#include "acc_log.h"

/**
 * @brief The module name
 */
#define MODULE "driver_pm_same70"

#define WKUP_GPIO_NOT_REGISTERED (0xFF)

/* To save more power in sleep mode, we use the slow clock as main clock source */
#define USE_SLOW_CLK_IN_SLEEP

/* Until we have a way of waking up the system from backup mode, we disable it */
#define DISABLE_BACKUP_MODE

static acc_device_pm_power_state_t requested_low_power_state = ACC_POWER_STATE_RUNNING;
static acc_device_pm_power_state_t actual_low_power_state = ACC_POWER_STATE_RUNNING;
static acc_device_pm_power_state_t current_low_power_state = ACC_POWER_STATE_RUNNING;

static uint8_t registered_req_wkup_gpio = WKUP_GPIO_NOT_REGISTERED;

/* Counter used to count how many clients that prevents the system from entering low power mode */
static uint32_t wake_lock_counter = 0;

/* Mutex to protect the wake_lock counter */
static acc_app_integration_mutex_t wake_lock_mutex = NULL;


#ifdef USE_ACCONEER_TICKLESS_IDLE

/* This holds the expected tick value after the xTaskIncrementTick()
 * have incremented the tick. This is needed since FreeRTOS does not
 * update the tick count directly, see tasks.c. This is used to make
 * sure that xTaskIncrementTick() isn't called more that it should.
 */
static volatile uint32_t ulExpectedTickValue;

static uint32_t systick_clock;

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define portNVIC_SYSTICK_CTRL_REG			( * ( ( volatile uint32_t * ) 0xe000e010 ) )
#define portNVIC_SYSTICK_LOAD_REG			( * ( ( volatile uint32_t * ) 0xe000e014 ) )
#define portNVIC_SYSTICK_CURRENT_VALUE_REG	( * ( ( volatile uint32_t * ) 0xe000e018 ) )

/* ...then bits in the registers. */
#define portNVIC_SYSTICK_INT_BIT			( 1UL << 1UL )
#define portNVIC_SYSTICK_ENABLE_BIT			( 1UL << 0UL )
#define portNVIC_SYSTICK_COUNT_FLAG_BIT		( 1UL << 16UL )

// Use core clock as systick source
#define portNVIC_SYSTICK_CLK_BIT	( 1UL << 2UL )


#ifdef ACC_CFG_INCLUDE_SEGGER_SYSVIEW
void SEGGER_SYSVIEW_PrintfTarget                  (const char* s, ...);
#else
#define SEGGER_SYSVIEW_PrintfTarget(...)
#endif


static uint32_t clear_rtt_interrupt(void)
{
	// To prevent several executions of the interrupt handler, the interrupt must be disabled in the
	// interrupt handler and re-enabled when the RTT_SR is cleared.
	rtt_disable_interrupt(RTT, RTT_MR_ALMIEN | RTT_MR_RTTINCIEN);
	uint32_t status = rtt_get_status(RTT);
	nvic_clear_pending(ID_RTT);
	return status;
}


static void set_rtt_alarm(uint32_t alarm_value)
{
	/* Ensure any pending interrupt is cleared before writing new value. */
	clear_rtt_interrupt();

	rtt_write_alarm_time(RTT, alarm_value);

	/* Enable RTT Alarm interrupt */
	rtt_enable_interrupt(RTT, RTT_MR_ALMIEN);
}


void rtt_alarm_handler(uint32_t source, void* user_arg)
{
	/* We should never end up here as we clear the interrupt
	 * after sleeping before enabling interrupts again.
	 */
	(void) source;
	(void) user_arg;
	configASSERT(pdFALSE);
}

/**
 * Update the FreeRTOS tick count using the value in the RTT.
 *
 * This function is called both after a tickless idle period and from the systick
 * interrupt handler. It will update the FreeRTOS tick using vTaskStepTick() with a
 * value of at most xExpectedIdleTimeTicks - 1 and then using xTaskIncrementTick()
 * for the remaining time out of sync. This is needed in order to set the
 * uxPendedTicks in tasks.c so that the idle loop which we sometimes returns to
 * will step up the tick without waiting for next SysTick interrupt.
 *
 * In case a debugging session is active the tick counter might be off by a large
 * amount, but after this function the FreeRTOS tick will be in sync with the RTT
 * again. This also means that the actual time between two ticks might be much shorter
 * when debugging, affecting calls to vTaskDelay().
 *
 * It asserts that the internal tick time is not in the future.
 *
 * @param[in] xExpectedIdleTimeTicks The max number of ticks that is expected to elapsed
 *
 * @return pdTRUE if a context switch is required in order to force the scheduler to run.
 */
static BaseType_t update_tick_count(uint32_t ulExpectedIdleTimeTicks)
{
	BaseType_t pended = pdFALSE;

	/* Should be xTaskGetTickCountFromISR() but that does not work
	 * due to late init of interrupt priorities.
	 */
	uint32_t ulTickNow = xTaskGetTickCount();
	uint32_t ulDiff = ulExpectedTickValue - ulTickNow;
	// Make sure that FreeRTOS tick counter is always same or less than ulExpectedTickValue
	configASSERT(ulDiff < INT32_MAX);

	uint32_t ulTimeNow = rtt_read_timer_value(RTT);
	uint32_t ulMissingTicks = ulTimeNow - ulExpectedTickValue;
	// Make sure that FreeRTOS time is not in the future
	configASSERT(ulMissingTicks < INT32_MAX);

	if (ulMissingTicks > 0)
	{
		uint32_t ulTicksToStep = MIN(ulMissingTicks, ulExpectedIdleTimeTicks) - 1;

		if (ulTicksToStep > 0)
		{
			vTaskStepTick(ulTicksToStep);
		}

		for (uint32_t i = ulTicksToStep; i < ulMissingTicks; i++)
		{
			if( xTaskIncrementTick() != pdFALSE )
			{
				pended = pdTRUE;
			}
		}
		// Update ulExpectedTickValue to current time as it should now be in sync
		ulExpectedTickValue = ulTimeNow;
	}

	return pended;
}


void SysTick_Handler(void)
{
#ifdef ACC_CFG_ENABLE_TRACECLOCK
	acc_driver_traceclock_cmx_systick_handler();
#endif

	/* Protect incrementing the tick with an interrupt safe critical section. */
	portDISABLE_INTERRUPTS();
	traceSYSTICKTIMER_EXPIRED();
	traceISR_ENTER();

	if (update_tick_count(1))
	{
		/* A context switch is required.  Context switching is performed in
		 * the PendSV interrupt.  Pend the PendSV interrupt.
		 */
		portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;
		traceISR_EXIT_TO_SCHEDULER();
	}
	else
	{
		traceISR_EXIT();
	}

	portENABLE_INTERRUPTS();
}


/* Override the default definition of vPortSetupTimerInterrupt() that is weakly
 * defined in the FreeRTOS Cortex-M7 port layer with a version that uses the
 * realtime timer (RTT) when sleeping and SysTick to generate the tick interrupt.
 */
void vPortSetupTimerInterrupt( void )
{
	/* Enable the RTT. */
	pmc_configure_peripheral(ID_RTT, NULL, true);

	/* Disable the RTT itself. */
	rtt_disable(RTT);

	/* Default RTT configuration */
	irq_add_handler(ID_RTT, rtt_alarm_handler, NULL);

	/* Set RTT alarm as fast wakeup input source */
	PMC->PMC_FSMR |= PMC_FSMR_RTTAL;
	SUPC->SUPC_WUMR |= SUPC_WUMR_RTTEN_ENABLE;

	rtt_disable_interrupt(RTT, RTT_MR_ALMIEN);

	/* Use slow clock as source for RTT */
	rtt_sel_source(RTT, false);

	/* Enable the RTT itself. */
	rtt_enable(RTT);

	/* Calculate prescaler based on actual clock source used to match tick speed*/
	uint32_t rtt_prescaler_value = (pmc_get_slow_clock() / (configTICK_RATE_HZ));
	rtt_init(RTT, rtt_prescaler_value);

	portNVIC_SYSTICK_CTRL_REG = 0UL;
	portNVIC_SYSTICK_CURRENT_VALUE_REG = 0UL;

	/* Configure SysTick to interrupt at the requested rate. */
	systick_clock = pmc_get_processor_clock();
	portNVIC_SYSTICK_LOAD_REG = ( systick_clock / configTICK_RATE_HZ ) - 1UL;
	portNVIC_SYSTICK_CTRL_REG = ( portNVIC_SYSTICK_CLK_BIT | portNVIC_SYSTICK_INT_BIT | portNVIC_SYSTICK_ENABLE_BIT );
}


void vPortSuppressTicksAndSleep( uint32_t xExpectedIdleTimeTicks )
{
	/* Enter a critical section but don't use the taskENTER_CRITICAL()
	 * method as that will mask interrupts that should exit sleep mode.
	 */
	__asm volatile( "cpsid i" ::: "memory" );
	__asm volatile( "dsb" );
	__asm volatile( "isb" );

	SEGGER_SYSVIEW_PrintfTarget("Expected idle=%d", (int)xExpectedIdleTimeTicks);

	/* If a context switch is pending or a task is waiting for the scheduler
	 * to be unsuspended then abandon the low power entry.
	 */
	if (eTaskConfirmSleepModeStatus() == eAbortSleep)
	{
		SEGGER_SYSVIEW_PrintfTarget("Abort");
		/* Re-enable interrupts - see comments above the cpsid instruction() above. */
		__asm volatile( "cpsie i" ::: "memory" );
	}
	else if(xExpectedIdleTimeTicks < 4)
	{
#ifdef ACC_CFG_ENABLE_TRACECLOCK
		acc_driver_traceclock_cmx_tickless_enter();
#endif
		// Use sys tick for shorter delays in order to avoid race
		// with the RTT alarm functionality
		portNVIC_SYSTICK_CTRL_REG &= ~portNVIC_SYSTICK_ENABLE_BIT;
		portNVIC_SYSTICK_CURRENT_VALUE_REG = 0;
		portNVIC_SYSTICK_LOAD_REG = ( systick_clock / configTICK_RATE_HZ * xExpectedIdleTimeTicks) - 1UL;
		portNVIC_SYSTICK_CTRL_REG |= portNVIC_SYSTICK_ENABLE_BIT;

		__asm volatile( "dsb" ::: "memory" );
		__asm volatile( "wfi" );
		__asm volatile( "isb" );

		portNVIC_SYSTICK_CTRL_REG &= ~portNVIC_SYSTICK_ENABLE_BIT;
		portNVIC_SYSTICK_LOAD_REG = ( systick_clock / configTICK_RATE_HZ ) - 1UL;
		portNVIC_SYSTICK_CTRL_REG |= portNVIC_SYSTICK_ENABLE_BIT;
#ifdef ACC_CFG_ENABLE_TRACECLOCK
		acc_driver_traceclock_cmx_tickless_exit();
#endif
		update_tick_count(xExpectedIdleTimeTicks);

		/* Exit with interrupts enabled. */
		__asm volatile( "cpsie i" ::: "memory" );
	}
	else
	{
		// Stop SYSTICK
		portNVIC_SYSTICK_CTRL_REG &= ~portNVIC_SYSTICK_ENABLE_BIT;
#ifdef ACC_CFG_ENABLE_TRACECLOCK
		acc_driver_traceclock_cmx_tickless_enter();
#endif

		/* Set an alarm for the expected idle time ticks. The actual tick
		 * count is used here since that is the time FreeRTOS used when
		 * calculating the xExpectedIdleTimeTicks */
		set_rtt_alarm(xTaskGetTickCount() + xExpectedIdleTimeTicks);
		irq_enable(ID_RTT);

		/* Sleep until something happens. configPRE_SLEEP_PROCESSING() can
		 * set its parameter to 0 to indicate that its implementation contains
		 * its own wait for interrupt or wait for event instruction, and so wfi
		 * should not be executed again.  However, the original expected idle
		 * time variable must remain unmodified, so a copy is taken.
		 */
		uint32_t xModifiableIdleTime = xExpectedIdleTimeTicks;
		configPRE_SLEEP_PROCESSING( &xModifiableIdleTime ); // Calls acc_device_pm_pre_sleep() -> acc_driver_pm_same70_pre_sleep()
		if( xModifiableIdleTime > 0 )
		{
			__asm volatile( "dsb" ::: "memory" );
			__asm volatile( "wfi" );
			__asm volatile( "isb" );
		}
		configPOST_SLEEP_PROCESSING( xExpectedIdleTimeTicks ); // Calls acc_device_pm_post_sleep() -> acc_driver_pm_same70_post_sleep()
		/* Clear the RTT interrupt to avoid the interrupt handler to execute */
		irq_disable(ID_RTT);
		clear_rtt_interrupt();

		/* Re-enable interrupts to allow the interrupt that brought the MCU
		out of sleep mode to execute immediately.  see comments above
		__disable_interrupt() call above. */
		__asm volatile( "cpsie i" ::: "memory" );
		__asm volatile( "dsb" );
		__asm volatile( "isb" );

		/* Disable interrupts again because the clock is about to be stopped
		and interrupts that execute while the clock is stopped will increase
		any slippage between the time maintained by the RTOS and calendar
		time. */
		__asm volatile( "cpsid i" ::: "memory" );
		__asm volatile( "dsb" );
		__asm volatile( "isb" );

		/* Start normal tick again using SysTick after resetting the current
		 * value register to maximize the delay until the next SysTick interrupt.
		 * Note that the SysTick interrupt is generated when the current value
		 * counts down from 1 to 0, and that any software write to the current
		 * value register sets it to zero. */
		portNVIC_SYSTICK_CURRENT_VALUE_REG = 0;
#ifdef ACC_CFG_ENABLE_TRACECLOCK
		acc_driver_traceclock_cmx_tickless_exit();
#endif
		portNVIC_SYSTICK_CTRL_REG |= portNVIC_SYSTICK_ENABLE_BIT;

		/* Wind the tick forward for the elapsed time before enabling interrupts
		 * in order to get an accurate time.
		 * The return value is not needed as we return to the scheduler
		 */
		update_tick_count(xExpectedIdleTimeTicks);

		/* Exit with interrupts enabled. */
		__asm volatile( "cpsie i" ::: "memory" );
	}
}

#endif

static uint32_t pllr0, mckr, fmr;

__inline static void save_clock_settings(void)
{
	mckr = PMC->PMC_MCKR;
	fmr  = EEFC->EEFC_FMR;
	pllr0 = PMC->CKGR_PLLAR;
}


static acc_device_pm_power_state_t determine_power_state_from_gpio(uint_fast8_t gpio_level)
{
	if (gpio_level == 0)
	{
		return requested_low_power_state;
	}
	else
	{
		return ACC_POWER_STATE_RUNNING;
	}
}


static void req_wkup_gpio_isr(void)
{
	uint_fast8_t value;

	if (acc_device_gpio_read(registered_req_wkup_gpio, &value))
	{
		actual_low_power_state = determine_power_state_from_gpio(value);
	}
}


static void acc_driver_pm_same70_pre_sleep(uint32_t *sleep_ticks)
{
	if (wake_lock_counter == 0)
	{
		current_low_power_state = actual_low_power_state;
	}
	else
	{
		current_low_power_state = ACC_POWER_STATE_RUNNING;
	}

	save_clock_settings();

	switch (current_low_power_state) {
	case ACC_POWER_STATE_RUNNING:
		break;

	case ACC_POWER_STATE_SLEEP:
#ifdef USE_SLOW_CLK_IN_SLEEP
		/* Configure slow RC oscillator */
		pmc_switch_mck_to_slck();
		pmc_disable_plla();
		pmc_disable_external_osc();
#else
		/* Set Fast RC oscillator frequency to 12MHz */
		PMC->CKGR_MOR = (PMC->CKGR_MOR & ~CKGR_MOR_MOSCRCF_Msk) | (CKGR_MOR_KEY_PASSWD | CKGR_MOR_MOSCRCF_12_MHz);

		/* Select the Main RC oscillator as Main Clock */
		pmc_select_internal_osc();
		pmc_switch_mck_to_main();

		/* Disable PLL */
		pmc_disable_plla();

		/* Set prescaler and divider to lowest possible */
		pmc_set_mck_divider(PMC_MCKR_MDIV_PCK_DIV4);
		pmc_set_mck_prescaler(PMC_MCKR_PRES_CLOCK_DIV64);
#endif

		/* Clear SLEEPDEEP bit of Cortex System Control Register */
		SCB->SCB_SCR &= (uint32_t)~SCB_SCR_SLEEPDEEP;

		break;

	case ACC_POWER_STATE_DEEPSLEEP:
	{
		/* Select the Main RC oscillator as Main Clock */
		pmc_select_internal_osc();
		pmc_switch_mck_to_main();

		/* Disable PLLs */
		pmc_disable_plla();

		/* Signal to the sleep handler that WFI should not be entered */
		*sleep_ticks = 0;

		/* Flash in wait mode (deep sleep) */
		uint32_t fsmr = PMC->PMC_FSMR;
		fsmr &= ~PMC_FSMR_FLPM_Msk;
		fsmr |= PMC_FSMR_FLPM_FLASH_DEEP_POWERDOWN;
		PMC->PMC_FSMR = fsmr;

		/* Set flash wait state to 0 */
		EEFC->EEFC_FMR = fmr & (~EEFC_FMR_FWS_Msk);

		/* Set LPM to 1 to go to wait mode in wfe */
		PMC->PMC_FSMR |= PMC_FSMR_LPM;

		/* Set HCLK = MCK by configuring MDIV to 0 in the PMC Master Clock register (PMC_MCKR) */
		/* Set prescaler 0, don't forget to set this back in post_sleep */
		pmc_set_mck_divider(PMC_MCKR_MDIV_EQ_PCK);
		pmc_set_mck_prescaler(PMC_MCKR_PRES_CLOCK);

		/* Set the WAITMODE bit in the PMC Clock Generator Main Oscillator register (CKGR_MOR) */
		PMC->CKGR_MOR |= (CKGR_MOR_KEY_PASSWD | CKGR_MOR_WAITMODE);

		/* Waiting for Master Clock Ready MCKRDY = 1 */
		while (!(PMC->PMC_SR & PMC_SR_MCKRDY));

		/* Note from SAME70 Datasheet about entry into Wait mode:
		 * Internal main clock resynchronization cycles are necessary between
		 * writing the MOSCRCEN bit and the entry in Wait mode.
		 * Depending on the user application, waiting for MOSCRCEN bit
		 * to be cleared is recommended to ensure that the core will not
		 * execute undesired instructions.
		 */
		for (uint16_t i = 0; i < 500; i++)
		{
			__asm volatile( "nop" );
		}

		while (!(PMC->CKGR_MOR & CKGR_MOR_MOSCRCEN));
		/* After the On-Chip RC Oscillator has been disabled, the system will enter Wait mode */

		break;
	}

	case ACC_POWER_STATE_BACKUP:
#ifndef DISABLE_BACKUP_MODE
		/* Configure fast RC oscillator */
		pmc_switch_mck_to_slck();
		pmc_disable_plla();
		pmc_disable_external_osc();

		PMC->PMC_FSMR |= PMC_FSMR_LPM;

		/* Set SLEEPDEEP bit of Cortex System Control Register */
		SCB->SCB_SCR |= SCB_SCR_SLEEPDEEP;

		/* Stop the voltage regulator */
		SUPC->SUPC_CR = SUPC_CR_KEY_PASSWD | SUPC_CR_VROFF_STOP_VREG;
#endif
		break;
	}
}


static void acc_driver_pm_same70_post_sleep(uint32_t sleep_ticks)
{
	(void)sleep_ticks; // Ignore parameter

	switch (current_low_power_state) {
	case ACC_POWER_STATE_RUNNING:
		break;

	case ACC_POWER_STATE_SLEEP:
		/* Restore clock settings */
		pmc_select_external_osc(false);

		/* Restore PLLA */
		if (pllr0 & CKGR_PLLAR_MULA_Msk)
		{
			PMC->CKGR_PLLAR = CKGR_PLLAR_ONE | pllr0;
			while (!(PMC->PMC_SR & PMC_SR_LOCKA));
		}

		/* Restore MCK prescaler and div */
		pmc_set_mck_divider(mckr & PMC_MCKR_MDIV_Msk);
		pmc_set_mck_prescaler(mckr & PMC_MCKR_PRES_Msk);

		/* Restore MCK source to PLLA */
		pmc_switch_mck_to_pll();

		break;

	case ACC_POWER_STATE_DEEPSLEEP:
	{
		/* Restore Flash in idle mode */
		uint32_t fsmr = PMC->PMC_FSMR;
		fsmr &= ~PMC_FSMR_FLPM_Msk;
		fsmr |= PMC_FSMR_FLPM_FLASH_IDLE;
		PMC->PMC_FSMR = fsmr;

		/* Restore flash wait state */
		EEFC->EEFC_FMR = fmr;

		/* Restore LPM to 0 */
		PMC->PMC_FSMR &= ~PMC_FSMR_LPM;

		/* Restore clock settings */
		pmc_select_external_osc(false);

		/* Restore PLLA */
		if (pllr0 & CKGR_PLLAR_MULA_Msk)
		{
			PMC->CKGR_PLLAR = CKGR_PLLAR_ONE | pllr0;
			while (!(PMC->PMC_SR & PMC_SR_LOCKA));
		}

		/* Restore MCK prescaler and div */
		pmc_set_mck_divider(mckr & PMC_MCKR_MDIV_Msk);
		pmc_set_mck_prescaler(mckr & PMC_MCKR_PRES_Msk);

		/* Restore MCK source to PLLA */
		pmc_switch_mck_to_pll();

		break;
	}

	case ACC_POWER_STATE_BACKUP:
#ifndef DISABLE_BACKUP_MODE
		board_cfg_clocks();

		/* Reset SLEEPDEEP bit of Cortex System Control Register */
		SCB->SCB_SCR &= (uint32_t)~SCB_SCR_SLEEPDEEP;
#endif
		break;
	}
}


/**
 * @brief Initialize the SAME70 power management driver
 *
 * @return true if successful, false otherwise
 */
static bool acc_driver_pm_same70_init(void)
{
	wake_lock_mutex = acc_os_mutex_create();
	if (wake_lock_mutex == NULL)
	{
		ACC_LOG_ERROR("failed to create mutex");
		return false;
	}

	acc_os_mutex_lock(wake_lock_mutex);
	wake_lock_counter = 0;
	acc_os_mutex_unlock(wake_lock_mutex);

	if (registered_req_wkup_gpio == WKUP_GPIO_NOT_REGISTERED)
	{
		ACC_LOG_ERROR("driver not registered prior to calling init");
		return false;
	}

	/* Force reading of GPIO to determine sleep level at boot */
	req_wkup_gpio_isr();

	if (!acc_device_gpio_register_isr(registered_req_wkup_gpio, ACC_DEVICE_GPIO_EDGE_BOTH, &req_wkup_gpio_isr))
	{
		ACC_LOG_ERROR("Unable to set req wkup ISR");
		return false;
	}

	/* Set MCU GPIO (PA30) as fast wakeup source on rising edge */
	PMC->PMC_FSMR |= PMC_FSMR_FSTT11;
	PMC->PMC_FSPR |= PMC_FSPR_FSTP11;
	SUPC->SUPC_WUIR |= (SUPC_WUIR_WKUPEN11_ENABLE | SUPC_WUIR_WKUPT11_HIGH);

	return true;
}


static void acc_driver_pm_set_lowest_power_state(acc_device_pm_power_state_t power_state)
{
	requested_low_power_state = power_state;
}


static void acc_driver_pm_wake_lock(void)
{
	acc_os_mutex_lock(wake_lock_mutex);
	wake_lock_counter++;
	acc_os_mutex_unlock(wake_lock_mutex);
}


static void acc_driver_pm_wake_unlock(void)
{
	acc_os_mutex_lock(wake_lock_mutex);

	if (wake_lock_counter > 0)
	{
		wake_lock_counter--;
	}

	acc_os_mutex_unlock(wake_lock_mutex);
}


/**
 * @brief Request driver to register with appropriate device(s)
 */
void acc_driver_pm_same70_register(uint8_t req_wkup_gpio)
{
	registered_req_wkup_gpio = req_wkup_gpio;

	acc_device_pm_init_func                   = acc_driver_pm_same70_init;
	acc_device_pm_pre_sleep_func              = acc_driver_pm_same70_pre_sleep;
	acc_device_pm_post_sleep_func             = acc_driver_pm_same70_post_sleep;
	acc_device_pm_set_lowest_power_state_func = acc_driver_pm_set_lowest_power_state;
	acc_device_pm_wake_lock_func              = acc_driver_pm_wake_lock;
	acc_device_pm_wake_unlock_func            = acc_driver_pm_wake_unlock;
}
