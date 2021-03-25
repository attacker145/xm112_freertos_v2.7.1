// Copyright (c) Acconeer AB, 2016-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "semphr.h"
#include "task.h"

#if defined(ACC_CFG_INCLUDE_SEGGER_SYSVIEW)
#include "SEGGER_SYSVIEW.h"
#endif

#include "acc_app_integration.h"
#include "acc_driver_os.h"
#include "acc_driver_os_freertos.h"
#include "acc_log.h"


#define MODULE "os"


/**
 * @brief Perform any os specific initialization
 */
static void acc_driver_os_init(void)
{
	static bool init_done;

	if (init_done)
	{
		return;
	}

#if defined(ACC_CFG_INCLUDE_SEGGER_SYSVIEW)
	SEGGER_SYSVIEW_Conf();
#endif

	init_done = true;
}


/**
 * @brief Exit current thread
 */
static void acc_driver_os_thread_exit(void)
{
	int min_stack_left = uxTaskGetStackHighWaterMark(NULL) * sizeof(int);

	ACC_LOG_INFO("Minimum stack left was %d bytes", min_stack_left);
}


/**
 * @brief Messure current threads stack usage
 */
static size_t acc_driver_os_stack_get_usage(size_t stack_size)
{
	(void)(stack_size);  // Suppress -Werror=unused-parameter

	int min_stack_left = uxTaskGetStackHighWaterMark(NULL) * sizeof(int);
	return 14000 - min_stack_left; //Needs to be updated if stack size changes!!!
}


static uint32_t get_current_time(void)
{
	TickType_t ticks = xTaskGetTickCount();
	uint32_t   ms    = ((uint64_t)ticks * 1000) / (uint64_t)configTICK_RATE_HZ;

	return ms;
}


void acc_driver_os_freertos_register(void)
{
	acc_device_os_init_func                            = acc_driver_os_init;
	acc_device_os_stack_get_usage_func                 = acc_driver_os_stack_get_usage;
	acc_device_os_sleep_ms_func                        = acc_app_integration_sleep_ms;
	acc_device_os_mem_alloc_func                       = pvPortMalloc;
	acc_device_os_mem_free_func                        = vPortFree;
	acc_device_os_get_time_func                        = get_current_time;
	acc_device_os_mutex_create_func                    = acc_app_integration_mutex_create;
	acc_device_os_mutex_lock_func                      = acc_app_integration_mutex_lock;
	acc_device_os_mutex_unlock_func                    = acc_app_integration_mutex_unlock;
	acc_device_os_mutex_destroy_func                   = acc_app_integration_mutex_destroy;
	acc_device_os_thread_create_func                   = acc_app_integration_thread_create;
	acc_device_os_thread_exit_func                     = acc_driver_os_thread_exit;
	acc_device_os_thread_cleanup_func                  = acc_app_integration_thread_cleanup;
	acc_device_os_semaphore_create_func                = acc_app_integration_semaphore_create;
	acc_device_os_semaphore_wait_func                  = acc_app_integration_semaphore_wait;
	acc_device_os_semaphore_signal_func                = acc_app_integration_semaphore_signal;
	acc_device_os_semaphore_signal_from_interrupt_func = acc_app_integration_semaphore_signal;
	acc_device_os_semaphore_destroy_func               = acc_app_integration_semaphore_destroy;
}
