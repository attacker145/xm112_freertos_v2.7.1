// Copyright (c) Acconeer AB, 2016-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#ifndef ACC_DRIVER_OS_H_
#define ACC_DRIVER_OS_H_

#include <stdbool.h>

#include "acc_app_integration.h"

// These functions are to be used by drivers only, do not use them directly
extern void                                (*acc_device_os_init_func)(void);
extern void                                (*acc_device_os_stack_setup_func)(size_t stack_size);
extern size_t                              (*acc_device_os_stack_get_usage_func)(size_t stack_size);
extern void                                (*acc_device_os_sleep_us_func)(uint32_t time_usec);
extern void                                (*acc_device_os_sleep_ms_func)(uint32_t time_msec);
extern void                                *(*acc_device_os_mem_alloc_func)(size_t);
extern void                                (*acc_device_os_mem_free_func)(void *);
extern acc_app_integration_thread_id_t     (*acc_device_os_get_thread_id_func)(void);
extern uint32_t                            (*acc_device_os_get_time_func)(void);
extern acc_app_integration_mutex_t         (*acc_device_os_mutex_create_func)(void);
extern void                                (*acc_device_os_mutex_lock_func)(acc_app_integration_mutex_t mutex);
extern void                                (*acc_device_os_mutex_unlock_func)(acc_app_integration_mutex_t mutex);
extern void                                (*acc_device_os_mutex_destroy_func)(acc_app_integration_mutex_t mutex);
extern acc_app_integration_thread_handle_t (*acc_device_os_thread_create_func)(void (*func)(void *param), void *param, const char *name);
extern void                                (*acc_device_os_thread_exit_func)(void);
extern void                                (*acc_device_os_thread_cleanup_func)(acc_app_integration_thread_handle_t handle);
extern acc_app_integration_semaphore_t     (*acc_device_os_semaphore_create_func)(void);
extern bool                                (*acc_device_os_semaphore_wait_func)(acc_app_integration_semaphore_t sem, uint16_t timeout_ms);
extern void                                (*acc_device_os_semaphore_signal_func)(acc_app_integration_semaphore_t sem);
extern void                                (*acc_device_os_semaphore_signal_from_interrupt_func)(acc_app_integration_semaphore_t sem);
extern void                                (*acc_device_os_semaphore_destroy_func)(acc_app_integration_semaphore_t sem);


#endif
