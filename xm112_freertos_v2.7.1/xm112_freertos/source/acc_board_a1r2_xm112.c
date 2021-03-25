// Copyright (c) Acconeer AB, 2018-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "irq/nvic.h"
#include "rstc.h"
#include "samv71.h"

#include "acc_ms_system.h"

#include "acc_definitions.h"
#include "acc_device.h"
#include "acc_device_gpio.h"
#include "acc_device_i2c.h"
#include "acc_device_memory.h"
#include "acc_device_os.h"
#include "acc_device_pm.h"
#include "acc_device_spi.h"
#include "acc_device_temperature.h"
#include "acc_device_uart.h"
#include "acc_driver_24cxx.h"
#include "acc_driver_ds7505.h"
#include "acc_driver_gpio_same70.h"
#include "acc_driver_i2c_same70.h"
#include "acc_driver_os.h"
#include "acc_driver_os_freertos.h"
#include "acc_driver_pm_same70.h"
#include "acc_driver_spi_same70.h"
#ifdef ACC_CFG_ENABLE_TRACECLOCK
#include "acc_driver_traceclock_cmx.h"
#endif
#include "acc_board.h"
#include "acc_board_a1r2_xm112.h"
#include "acc_driver_uart_same70.h"
#include "acc_log.h"
#include "acc_ms_system.h"

/**
 * @brief The module name
 *
 * Must exist if acc_log.h is used.
 */
#define MODULE "acc_board_a1r2_xm112"


/* Backup register 0..3 are cleared when waking up
 * on WKUP0 (PA0 = SENS_INT) or WKUP1 (=PA1 = NC),
 * so avoid them for now even if it is probably not relevant*/
#define GPBR_ERROR_COUNTER_REGISTER 4
#define GPBR_SERVICE_MODE_REGISTER  5
#define GPBR_SERVICE_MODE_VALUE     0xACC01CED


extern uint8_t acc_debug_uart_port;


#define XM11x_SENSOR_COUNT (1)

#define XM11x_SENSOR_REFERENCE_FREQUENCY (24000000)
#define XM11x_SPI_SPEED                  (48000000)
#define XM11x_SPI_MASTER_BUS             (1)
#define XM11x_SPI_SLAVE_BUS              (0)
#define XM11x_SPI_CS                     (0)
#define XM11x_SPI_MASTER_BUF_SIZE        (1024)
#define XM11x_SPI_SLAVE_BUF_SIZE         (8)

#define XM11x_SENS_INT_PIN   0   // PA0
#define XM11x_SENS_EN_PIN    106 // PD10
#define XM11x_LED_PIN        67  // PC3
#define XM11x_MODULE_INT_PIN 66  // PC2
#define XM11x_PS_ENABLE_PIN  98  // PD2
#define XM11x_PWR_SIGNAL_PIN 30  // PA30

#define XM11x_GPIO_PINS 144

#define XM11x_I2C_DEVICE_ID 0x52

#define XM11x_I2C_24CXX_DEVICE_ID   0x51
#define XM11x_I2C_24CXX_MEMORY_SIZE 0x4000

#define XM11x_I2C_DS7505_DEVICE_ID 0x48


#define XM11x_GD_MAGIC_NUMBER (0xACC01337)


#define NVIC_IPR_REGISTER_COUNT 60

static acc_app_integration_semaphore_t uart_complete_semaphores[UART_IFACE_COUNT];

#define UART_TRANSFER_TIMEOUT 1000

#define SPI_MASTER_TRANSFER_TIMEOUT 1000

/**
 * @brief The sensor SPI pins
 */
static acc_driver_spi_same70_config_t sensor_spi_config = PINS_SPI1_NPCS0;

/**
 * @brief The slave SPI pins
 */
static acc_driver_spi_same70_config_t slave_spi_config = PINS_SPI0_NPCS0;

static acc_device_handle_t             i2c_0_device_handle;
static acc_device_handle_t             i2c_2_device_handle;
static acc_device_handle_t             spi_master_handle;
static acc_app_integration_semaphore_t spi_master_transfer_complete_semaphore;
static acc_device_handle_t             spi_slave_handle;
static gpio_t                          gpios[XM11x_GPIO_PINS];

static bool sensor_active = false;

static acc_board_xm112_config_t config;

static acc_app_integration_semaphore_t isr_semaphore;

static acc_ms_sensor_interrupt_callback_t isr_callback;

/**
 * This function uses a function pointer to simplify Acconeer testing
 **/
static void acc_board_get_config_default(acc_board_xm112_config_t *config);


acc_board_get_config_t acc_board_get_config = acc_board_get_config_default;


bool acc_ms_system_is_sensor_interrupt_active(void)
{
	uint_fast8_t level;

	acc_device_gpio_read(XM11x_SENS_INT_PIN, &level);
	return level;
}


void acc_ms_system_register_sensor_interrupt_callback(acc_ms_sensor_interrupt_callback_t callback)
{
	isr_callback = callback;
}


static void isr_sensor(void)
{
	acc_os_semaphore_signal_from_interrupt(isr_semaphore);
	if (isr_callback != NULL)
	{
		isr_callback();
	}
}


static bool setup_isr(void)
{
	isr_semaphore = acc_os_semaphore_create();

	if (isr_semaphore == NULL)
	{
		return false;
	}

	if (!acc_device_gpio_register_isr(XM11x_SENS_INT_PIN, ACC_DEVICE_GPIO_EDGE_RISING, &isr_sensor))
	{
		return false;
	}

	return true;
}


static void set_led(bool enable);


static void acc_board_deinit(void);


static bool is_service_mode(void);


static void xm11x_wait_for_spi_transfer_complete(acc_device_handle_t dev_handle)
{
	if (dev_handle == spi_master_handle)
	{
		acc_os_semaphore_wait(spi_master_transfer_complete_semaphore, SPI_MASTER_TRANSFER_TIMEOUT);
	}
}


static void xm11x_spi_transfer_complete_callback(acc_device_handle_t dev_handle)
{
	if (dev_handle == spi_master_handle)
	{
		acc_os_semaphore_signal_from_interrupt(spi_master_transfer_complete_semaphore);
	}
}


static void xm11x_wait_for_uart_transfer_complete(uint_fast8_t port)
{
	acc_os_semaphore_wait(uart_complete_semaphores[port], UART_TRANSFER_TIMEOUT);
}


static void xm11x_uart_transfer_complete_callback(uint_fast8_t port)
{
	acc_os_semaphore_signal_from_interrupt(uart_complete_semaphores[port]);
}


bool acc_board_gpio_init(void)
{
	return true;
}


void acc_board_get_config_default(acc_board_xm112_config_t *config)
{
	memset(config, 0, sizeof(*config));
	//config->uart_config[0].open         = true;
	//config->uart_config[0].baudrate     = 115200;
	//config->uart_config[0].use_as_debug = true;

	config->uart_config[2].open         = true;
	config->uart_config[2].baudrate     = 115200;
	config->uart_config[2].use_as_debug = true;
}


bool acc_board_init(void)
{
	acc_board_get_config(&config);

	acc_driver_os_freertos_register();
	acc_os_init();

	// Initialize interrupt priority for all external interrupts to
	// the most urgent priority allowed in FreeRTOS.
	uint32_t prioReg = (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8U - configPRIO_BITS));
	prioReg = (prioReg << 24) | (prioReg << 16) | (prioReg << 8) | prioReg;
	for (int i = 0; i < NVIC_IPR_REGISTER_COUNT; i++)
	{
		NVIC->NVIC_IPR[i] = prioReg;
	}

#ifdef ACC_CFG_ENABLE_TRACECLOCK
	acc_driver_traceclock_cmx_register();
#endif

	acc_driver_uart_same70_register(xm11x_wait_for_uart_transfer_complete, xm11x_uart_transfer_complete_callback);
	for (int i = 0; i < UART_IFACE_COUNT; i++)
	{
		if (config.uart_config[i].open)
		{
			uart_complete_semaphores[i] = acc_os_semaphore_create();
			if (NULL == uart_complete_semaphores[i])
			{
				ACC_LOG_ERROR("Unable to create semaphore");
				acc_board_deinit();
				return false;
			}

			acc_device_uart_init(i, config.uart_config[i].baudrate, ACC_DEVICE_UART_OPTIONS_ALT_PINS_1);
			if (config.uart_config[i].use_as_debug)
			{
				acc_debug_uart_port = i;
			}
		}
	}

	ACC_LOG_INFO("Error counter is now %" PRIu32, GPBR->SYS_GPBR[GPBR_ERROR_COUNTER_REGISTER]);

	acc_driver_gpio_same70_register(XM11x_GPIO_PINS, gpios);
	acc_device_gpio_init();
	set_led(false);

	// Hibernation is not supported on this board
	acc_board_hibernate_enter_func = NULL;
	acc_board_hibernate_exit_func  = NULL;

	spi_master_transfer_complete_semaphore = acc_os_semaphore_create();
	if (NULL == spi_master_transfer_complete_semaphore)
	{
		ACC_LOG_ERROR("Unable to create semaphore");
		acc_board_deinit();
		return false;
	}

	acc_driver_spi_same70_register(xm11x_wait_for_spi_transfer_complete, xm11x_spi_transfer_complete_callback);

	acc_device_spi_configuration_t master_configuration;

	master_configuration.bus           = XM11x_SPI_MASTER_BUS;
	master_configuration.configuration = &sensor_spi_config;
	master_configuration.device        = XM11x_SPI_CS;
	master_configuration.master        = true;
	master_configuration.speed         = XM11x_SPI_SPEED;
	master_configuration.buffer_size   = XM11x_SPI_MASTER_BUF_SIZE;

	spi_master_handle = acc_device_spi_create(&master_configuration);
	if (NULL == spi_master_handle)
	{
		ACC_LOG_ERROR("Unable to create SPI master");
		acc_board_deinit();
		return false;
	}

	acc_device_spi_configuration_t slave_configuration;

	slave_configuration.bus           = XM11x_SPI_SLAVE_BUS;
	slave_configuration.configuration = &slave_spi_config;
	slave_configuration.device        = XM11x_SPI_CS;
	slave_configuration.master        = false;
	slave_configuration.speed         = XM11x_SPI_SPEED;
	slave_configuration.buffer_size   = XM11x_SPI_SLAVE_BUF_SIZE;

	spi_slave_handle = acc_device_spi_create(&slave_configuration);
	if (NULL == spi_slave_handle)
	{
		ACC_LOG_ERROR("Unable to create SPI slave");
		acc_board_deinit();
		return false;
	}

	acc_driver_i2c_same70_register();

	acc_device_i2c_configuration_t i2c_0_configuration;

	i2c_0_configuration.bus                = 0;
	i2c_0_configuration.master             = false;
	i2c_0_configuration.mode.slave.address = XM11x_I2C_DEVICE_ID;

	i2c_0_device_handle = acc_device_i2c_create(i2c_0_configuration);
	if (NULL == i2c_0_device_handle)
	{
		ACC_LOG_ERROR("Unable to create I2C slave");
		acc_board_deinit();
		return false;
	}

	acc_device_i2c_configuration_t i2c_2_configuration;

	i2c_2_configuration.bus                   = 2;
	i2c_2_configuration.master                = true;
	i2c_2_configuration.mode.master.frequency = 100000;

	i2c_2_device_handle = acc_device_i2c_create(i2c_2_configuration);
	if (NULL == i2c_2_device_handle)
	{
		ACC_LOG_ERROR("Unable to create I2C master");
		acc_board_deinit();
		return false;
	}

	acc_driver_24cxx_register(i2c_2_device_handle, XM11x_I2C_24CXX_DEVICE_ID, XM11x_I2C_24CXX_MEMORY_SIZE);
	acc_device_memory_init();

	acc_driver_ds7505_register(i2c_2_device_handle, XM11x_I2C_DS7505_DEVICE_ID);
	acc_device_temperature_init();

	uint32_t magic_number = 0;

	if (acc_device_memory_read(0, &magic_number, sizeof(magic_number)))
	{
		ACC_LOG_INFO("Magic number read: 0x%8" PRIx32, magic_number);

		if (magic_number != XM11x_GD_MAGIC_NUMBER)
		{
			ACC_LOG_INFO("Magic number not matched, unknown revision");
		}
	}
	else
	{
		ACC_LOG_ERROR("XM11x data could not be read");
		if (!is_service_mode())
		{
			acc_board_deinit();
			return false;
		}
	}

	acc_device_gpio_set_initial_pull(XM11x_SENS_INT_PIN, 0);
	acc_device_gpio_set_initial_pull(XM11x_SENS_EN_PIN, 0);
	acc_device_gpio_set_initial_pull(XM11x_PS_ENABLE_PIN, 0);
	acc_device_gpio_set_initial_pull(XM11x_PWR_SIGNAL_PIN, 0);

	if (!acc_device_gpio_write(XM11x_SENS_EN_PIN, 0))
	{
		ACC_LOG_ERROR("Unable to deactivate SENS_EN");
		acc_board_deinit();
		return false;
	}

	if (!acc_device_gpio_write(XM11x_PS_ENABLE_PIN, 0))
	{
		ACC_LOG_ERROR("Unable to deactivate PS_ENABLE");
		acc_board_deinit();
		return false;
	}

	if (!acc_device_gpio_input(XM11x_SENS_INT_PIN))
	{
		ACC_LOG_ERROR("Unable to configure SENS_INT as input");
		acc_board_deinit();
		return false;
	}

	if (!acc_device_gpio_input(XM11x_MODULE_INT_PIN))
	{
		ACC_LOG_ERROR("Unable to deactivate module interrupt pin");
		acc_board_deinit();
		return false;
	}

	if (!acc_device_gpio_input(XM11x_PWR_SIGNAL_PIN))
	{
		ACC_LOG_ERROR("Unable to configure pwr_signal_pin as input");
		acc_board_deinit();
		return false;
	}

	acc_driver_pm_same70_register(XM11x_PWR_SIGNAL_PIN);

	if (!acc_device_pm_init())
	{
		ACC_LOG_ERROR("Unable to initialize pm device");
		acc_board_deinit();
		return false;
	}

	if (!setup_isr())
	{
		ACC_LOG_ERROR("Unable to setup isr");
		acc_board_deinit();
		return false;
	}

	return true;
}


static void acc_board_deinit(void)
{
	if (NULL != i2c_2_device_handle)
	{
		acc_device_i2c_destroy(&i2c_2_device_handle);
	}

	if (NULL != spi_slave_handle)
	{
		acc_device_spi_destroy(&spi_slave_handle);
	}

	if (NULL != spi_master_handle)
	{
		acc_device_spi_destroy(&spi_master_handle);
	}

	if (NULL != spi_master_transfer_complete_semaphore)
	{
		acc_os_semaphore_destroy(spi_master_transfer_complete_semaphore);
		spi_master_transfer_complete_semaphore = NULL;
	}

	if (NULL != isr_semaphore)
	{
		acc_os_semaphore_destroy(isr_semaphore);
		isr_semaphore = NULL;
	}

	for (int i = 0; i < UART_IFACE_COUNT; i++)
	{
		if (NULL != uart_complete_semaphores[i])
		{
			acc_os_semaphore_destroy(uart_complete_semaphores[i]);
		}
	}
}


void acc_board_start_sensor(acc_sensor_id_t sensor)
{
	(void)sensor;

	if (sensor_active)
	{
		ACC_LOG_ERROR("Sensor already active.");
		return;
	}

	if (!acc_device_gpio_write(XM11x_PS_ENABLE_PIN, 1))
	{
		ACC_LOG_ERROR("Unable to activate PS_ENABLE");
		return;
	}

	if (!acc_device_gpio_write(XM11x_SENS_EN_PIN, 1))
	{
		ACC_LOG_ERROR("Unable to activate SENS_EN");
		return;
	}

	// Crystal stabilization time is 1-2 ms
	// Sleep 3 ms just to be safe (sleep functions don't have to be accurate)
	acc_os_sleep_ms(3);

	// Clear pending interrupts
	while (acc_os_semaphore_wait(isr_semaphore, 0));

	sensor_active = true;
}


void acc_board_stop_sensor(acc_sensor_id_t sensor)
{
	(void)sensor;

	if (!sensor_active)
	{
		ACC_LOG_ERROR("Sensor already inactive.");
		return;
	}

	sensor_active = false;

	if (!acc_device_gpio_write(XM11x_SENS_EN_PIN, 0))
	{
		ACC_LOG_WARNING("Unable to deactivate SENS_EN");
	}

	// t_wait according to integration specification at least 200 us
	// but timer resolution is in ms.
	acc_os_sleep_ms(1);

	if (!acc_device_gpio_write(XM11x_PS_ENABLE_PIN, 0))
	{
		ACC_LOG_WARNING("Unable to deactivate PS_ENABLE");
	}
}


void acc_board_sensor_transfer(acc_sensor_id_t sensor_id, uint8_t *buffer, size_t buffer_length)
{
	(void)sensor_id;

	acc_device_spi_lock(acc_device_spi_get_bus(spi_master_handle));

	acc_device_spi_transfer(spi_master_handle, buffer, buffer_length);

	acc_device_spi_unlock(acc_device_spi_get_bus(spi_master_handle));
}


bool acc_board_wait_for_sensor_interrupt(acc_sensor_id_t sensor_id, uint32_t timeout_ms)
{
	(void)sensor_id;
	return acc_os_semaphore_wait(isr_semaphore, timeout_ms);
}


uint32_t acc_board_get_sensor_count(void)
{
	return 1;
}


float acc_board_get_ref_freq(void)
{
	return XM11x_SENSOR_REFERENCE_FREQUENCY;
}


acc_device_handle_t acc_board_get_spi_slave_handle(void)
{
	return spi_slave_handle;
}


acc_device_handle_t acc_board_get_i2c_slave_handle(void)
{
	return i2c_0_device_handle;
}


void set_led(bool enable)
{
	if (enable)
	{
		// Driving the pin low enables LED
		acc_device_gpio_write(XM11x_LED_PIN, 0);
	}
	else
	{
		// Driving the pin high disables LED
		acc_device_gpio_write(XM11x_LED_PIN, 1);
	}
}


static bool is_interrupt_context(void)
{
	uint32_t ipsr;

	asm ("mrs %0, ipsr" : "=r" (ipsr));
	return ipsr != 0;
}


static bool is_debugger_active(void)
{
	return CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk;
}


static bool is_service_mode(void)
{
	return GPBR->SYS_GPBR[GPBR_SERVICE_MODE_REGISTER] == GPBR_SERVICE_MODE_VALUE;
}


void system_fatal_error_handler(const char *reason)
{
	if (is_service_mode())
	{
		// We are in "service mode", ignore all errors and hope for the best
		return;
	}

	static mutex_t recursion = 0;
	if (!mutex_try_lock(&recursion))
	{
		/* We have ended here recursively, lets just reset
		 * silently as it seems like we are getting errors
		 * when executing code below
		 */
		if (is_debugger_active())
		{
			asm ("bkpt #3" ::: "memory");
		}

		rstc_reset_all();
	}

	/* Increase a non volatile counter that we also print
	 * in order to give an idea on how often this has happened.
	 */
	GPBR->SYS_GPBR[GPBR_ERROR_COUNTER_REGISTER]++;

	// Avoid logging in interrupt context as that will fail
	if (!is_interrupt_context())
	{
		ACC_LOG_ERROR("Error %s\n", reason);
		ACC_LOG_ERROR("error counter=%" PRIu32 ", rebooting\n",
		              GPBR->SYS_GPBR[GPBR_ERROR_COUNTER_REGISTER]);
	}

	if (is_debugger_active())
	{
		asm ("bkpt #3" ::: "memory");
	}

	rstc_reset_all();
	while (1);
}


void vApplicationMallocFailedHook(void)
{
	system_fatal_error_handler(__func__);
}
