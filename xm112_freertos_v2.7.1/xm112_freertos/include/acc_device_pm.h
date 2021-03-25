// Copyright (c) Acconeer AB, 2018-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#ifndef ACC_DEVICE_PM_H_
#define ACC_DEVICE_PM_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


/**
 * @brief The requested sleep state of the system
 *
 */
typedef enum {
	/** No explicit sleep state active */
	ACC_POWER_STATE_RUNNING,
	/** System is in WFI, with clocks lowered. RAM in retention and regulators enabled. Wake on any interrupt */
	ACC_POWER_STATE_SLEEP,
	/** System in deep sleep and peripherals are powered down. RAM in retention and regulators enabled. Wake only on dedicated signals */
	ACC_POWER_STATE_DEEPSLEEP,
	/** System is in the lowest power state. All blocks are off and RAM is off. Wake only on dedicated signals */
	ACC_POWER_STATE_BACKUP,
} acc_device_pm_power_state_t;

/* These functions are to be used by drivers only, do not use them directly */
extern bool (*acc_device_pm_init_func)(void);
extern void (*acc_device_pm_pre_sleep_func)(uint32_t *sleep_ticks);
extern void (*acc_device_pm_post_sleep_func)(uint32_t sleep_ticks);
extern void (*acc_device_pm_set_lowest_power_state_func)(acc_device_pm_power_state_t req_power_state);
extern void (*acc_device_pm_wake_lock_func)(void);
extern void (*acc_device_pm_wake_unlock_func)(void);


/**
 * @brief Initialize the power management device
 *
 * @return Status
 */
extern bool acc_device_pm_init(void);


/**
 * @brief Prepare the system for going to sleep
 *
 * @param[in] sleep_ticks pointer to number of ticks the system is supposed to sleep at maximum.
 *                        If the sleep itself is handled in this function sleep_ticks is set to 0
 */
extern void acc_device_pm_pre_sleep(uint32_t *sleep_ticks);


/**
 * @brief Restore the system after having been to sleep
 *
 * @param[in] sleep_ticks The number of ticks the system is supposed to sleep at maximum.
 */
extern void acc_device_pm_post_sleep(uint32_t sleep_ticks);


/**
 * @brief Set the power state the system can go at the lowest
 *
 * @param[in] req_power_state power state
 */
extern void acc_device_pm_set_lowest_power_state(acc_device_pm_power_state_t req_power_state);


/**
 * @brief Prevent the system from entering any low power state.
 *
 * Implemented as a counter that needs to be decreased using acc_device_pm_wake_unlock
 */
extern void acc_device_pm_wake_lock(void);


/**
 * @brief Remove the blocker from entering any low power state.
 *
 */
extern void acc_device_pm_wake_unlock(void);

#endif
