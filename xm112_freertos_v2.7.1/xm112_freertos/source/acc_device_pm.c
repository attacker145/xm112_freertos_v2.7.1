// Copyright (c) Acconeer AB, 2018-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#include <stdbool.h>
#include <stdint.h>

#include "acc_device_pm.h"


/* These functions are to be used by drivers only, do not use them directly */
bool (*acc_device_pm_init_func)(void) = NULL;
void (*acc_device_pm_pre_sleep_func)(uint32_t *sleep_ticks) = NULL;
void (*acc_device_pm_post_sleep_func)(uint32_t sleep_ticks) = NULL;
void (*acc_device_pm_set_lowest_power_state_func)(acc_device_pm_power_state_t req_power_state) = NULL;
void (*acc_device_pm_wake_lock_func)(void) = NULL;
void (*acc_device_pm_wake_unlock_func)(void) = NULL;

void acc_device_pm_set_lowest_power_state(acc_device_pm_power_state_t req_power_state)
{
	if (acc_device_pm_set_lowest_power_state_func)
	{
		 acc_device_pm_set_lowest_power_state_func(req_power_state);
	}
}


void acc_device_pm_pre_sleep(uint32_t *sleep_ticks)
{
	if (acc_device_pm_pre_sleep_func)
	{
		 acc_device_pm_pre_sleep_func(sleep_ticks);
	}
}


void acc_device_pm_post_sleep(uint32_t sleep_ticks)
{
	if (acc_device_pm_post_sleep_func)
	{
		 acc_device_pm_post_sleep_func(sleep_ticks);
	}
}


bool acc_device_pm_init(void)
{
	if (!acc_device_pm_init_func)
	{
		return false;
	}

	return acc_device_pm_init_func();
}

void acc_device_pm_wake_lock(void)
{
	if (acc_device_pm_wake_lock_func)
	{
		acc_device_pm_wake_lock_func();
	}
}

void acc_device_pm_wake_unlock(void)
{
	if (acc_device_pm_wake_unlock_func)
	{
		acc_device_pm_wake_unlock_func();
	}
}
