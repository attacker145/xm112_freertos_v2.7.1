// Copyright (c) Acconeer AB, 2018-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#include <stdbool.h>
#include <stdio.h>

#include "acc_device_os.h"

#include "acc_app_integration.h"

static bool init_done;

void                                (*acc_device_os_init_func)(void) = NULL;
void                                (*acc_device_os_stack_setup_func)(size_t stack_size) = NULL;
size_t                              (*acc_device_os_stack_get_usage_func)(size_t stack_size) = NULL;
void                                (*acc_device_os_sleep_us_func)(uint32_t time_usec) = NULL;
void                                (*acc_device_os_sleep_ms_func)(uint32_t time_msec) = NULL;
void                                *(*acc_device_os_mem_alloc_func)(size_t) = NULL;
void                                (*acc_device_os_mem_free_func)(void *) = NULL;
acc_app_integration_thread_id_t     (*acc_device_os_get_thread_id_func)(void) = NULL;
uint32_t                            (*acc_device_os_get_time_func)(void) = NULL;
acc_app_integration_mutex_t         (*acc_device_os_mutex_create_func)(void) = NULL;
void                                (*acc_device_os_mutex_lock_func)(acc_app_integration_mutex_t mutex) = NULL;
void                                (*acc_device_os_mutex_unlock_func)(acc_app_integration_mutex_t mutex) = NULL;
void                                (*acc_device_os_mutex_destroy_func)(acc_app_integration_mutex_t mutex) = NULL;
acc_app_integration_thread_handle_t (*acc_device_os_thread_create_func)(void (*func)(void *param), void *param, const char *name) = NULL;
void                                (*acc_device_os_thread_exit_func)(void) = NULL;
void                                (*acc_device_os_thread_cleanup_func)(acc_app_integration_thread_handle_t handle) = NULL;
acc_app_integration_semaphore_t     (*acc_device_os_semaphore_create_func)(void) = NULL;
bool                                (*acc_device_os_semaphore_wait_func)(acc_app_integration_semaphore_t sem, uint16_t timeout_ms) = NULL;
void                                (*acc_device_os_semaphore_signal_func)(acc_app_integration_semaphore_t sem) = NULL;
void                                (*acc_device_os_semaphore_signal_from_interrupt_func)(acc_app_integration_semaphore_t sem) = NULL;
void                                (*acc_device_os_semaphore_destroy_func)(acc_app_integration_semaphore_t sem) = NULL;


void acc_os_init(void)
{
	if (init_done)
	{
		return;
	}

	if (acc_device_os_init_func != NULL)
	{
		acc_device_os_init_func();
	}

	init_done = true;
}


void acc_os_stack_setup(size_t stack_size)
{
	if (init_done && acc_device_os_stack_setup_func != NULL)
	{
		acc_device_os_stack_setup_func(stack_size);
	}
}


size_t acc_os_stack_get_usage(size_t stack_size)
{
	size_t result = 0;

	if (init_done && acc_device_os_stack_get_usage_func != NULL)
	{
		result = acc_device_os_stack_get_usage_func(stack_size);
	}

	return result;
}


void acc_os_sleep_us(uint32_t time_usec)
{
	if (init_done && acc_device_os_sleep_us_func != NULL)
	{
		acc_device_os_sleep_us_func(time_usec);
	}
}


void acc_os_sleep_ms(uint32_t time_msec)
{
	if (init_done && acc_device_os_sleep_ms_func != NULL)
	{
		acc_device_os_sleep_ms_func(time_msec);
	}
}


void *acc_os_mem_alloc_debug(size_t size, const char *file, uint16_t line)
{
	(void)file;
	(void)line;

	void *result = NULL;

	if (init_done && acc_device_os_mem_alloc_func != NULL)
	{
		result = acc_device_os_mem_alloc_func(size);
	}

	return result;
}


void *acc_os_mem_calloc_debug(size_t num, size_t size, const char *file, uint16_t line)
{
	if (num == 0)
	{
		return NULL;
	}

	size_t total_size = num * size;
	void   *mem       = acc_os_mem_alloc_debug(total_size, file, line);

	if (mem == NULL)
	{
		return NULL;
	}

	memset(mem, 0, total_size);

	return mem;
}


void acc_os_mem_free(void *ptr)
{
	if (init_done && acc_device_os_mem_free_func != NULL)
	{
		acc_device_os_mem_free_func(ptr);
	}
}


acc_app_integration_thread_id_t acc_os_get_thread_id(void)
{
	acc_app_integration_thread_id_t result = {0};

	if (init_done && acc_device_os_get_thread_id_func != NULL)
	{
		result = acc_device_os_get_thread_id_func();
	}

	return result;
}


uint32_t acc_os_get_time(void)
{
	if (init_done && acc_device_os_get_time_func != NULL)
	{
		return acc_device_os_get_time_func();
	}

	return 0;
}


acc_app_integration_mutex_t acc_os_mutex_create(void)
{
	acc_app_integration_mutex_t result = NULL;

	if (init_done && acc_device_os_mutex_create_func != NULL)
	{
		result = acc_device_os_mutex_create_func();
	}

	return result;
}


void acc_os_mutex_lock(acc_app_integration_mutex_t mutex)
{
	if (init_done && acc_device_os_mutex_lock_func != NULL)
	{
		acc_device_os_mutex_lock_func(mutex);
	}
}


void acc_os_mutex_unlock(acc_app_integration_mutex_t mutex)
{
	if (init_done && acc_device_os_mutex_unlock_func != NULL)
	{
		acc_device_os_mutex_unlock_func(mutex);
	}
}


void acc_os_mutex_destroy(acc_app_integration_mutex_t mutex)
{
	if (init_done && acc_device_os_mutex_destroy_func != NULL)
	{
		acc_device_os_mutex_destroy_func(mutex);
	}
}


acc_app_integration_thread_handle_t acc_os_thread_create(void (*func)(void *param), void *param, const char *name)
{
	void *result = NULL;

	if (init_done && acc_device_os_thread_create_func != NULL)
	{
		result = acc_device_os_thread_create_func(func, param, name);
	}

	return result;
}


void acc_os_thread_exit(void)
{
	if (init_done && acc_device_os_thread_exit_func != NULL)
	{
		acc_device_os_thread_exit_func();
	}
}


void acc_os_thread_cleanup(acc_app_integration_thread_handle_t handle)
{
	if (init_done && acc_device_os_thread_cleanup_func != NULL)
	{
		acc_device_os_thread_cleanup_func(handle);
	}
}


acc_app_integration_semaphore_t acc_os_semaphore_create()
{
	acc_app_integration_semaphore_t result = NULL;

	if (init_done && acc_device_os_semaphore_create_func != NULL)
	{
		result = acc_device_os_semaphore_create_func();
	}

	return result;
}


bool acc_os_semaphore_wait(acc_app_integration_semaphore_t sem, uint16_t timeout_ms)
{
	bool result = false;

	if (init_done && acc_device_os_semaphore_wait_func != NULL)
	{
		result = acc_device_os_semaphore_wait_func(sem, timeout_ms);
	}

	return result;
}


void acc_os_semaphore_signal(acc_app_integration_semaphore_t sem)
{
	if (init_done && acc_device_os_semaphore_signal_func != NULL)
	{
		acc_device_os_semaphore_signal_func(sem);
	}
}


void acc_os_semaphore_signal_from_interrupt(acc_app_integration_semaphore_t sem)
{
	if (init_done && acc_device_os_semaphore_signal_from_interrupt_func != NULL)
	{
		acc_device_os_semaphore_signal_from_interrupt_func(sem);
	}
}


void acc_os_semaphore_destroy(acc_app_integration_semaphore_t sem)
{
	if (init_done && acc_device_os_semaphore_destroy_func != NULL)
	{
		acc_device_os_semaphore_destroy_func(sem);
	}
}


bool acc_os_multithread_support(void)
{
	bool result = false;

	// We assume that we are multithreaded if the client has implemented thread_create
	if (init_done && acc_device_os_thread_create_func != NULL)
	{
		result = true;
	}

	return result;
}
