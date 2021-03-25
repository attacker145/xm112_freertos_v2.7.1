// Copyright (c) Acconeer AB, 2019-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#include <assert.h>
#include <stddef.h>

#include "acc_app_integration.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#define ACC_APP_STACK_SIZE 6000


typedef struct acc_app_integration_thread_handle
{
	TaskHandle_t handle;
	void (*func)(void *param);
	void              *param;
	SemaphoreHandle_t stopped;
} acc_app_integration_thread_handle;


void acc_app_integration_thread_cleanup(acc_app_integration_thread_handle_t thread)
{
	assert(thread != NULL);
	xSemaphoreTake(thread->stopped, portMAX_DELAY);
	vSemaphoreDelete(thread->stopped);
	vPortFree(thread);
}


/**
 * @brief Internal function to handle graceful termination of thread when thread function returns
 *
 * This is a wrapper function for the main thread function to gracefully
 * terminate a thread if the thread function returns.
 *
 * The data pointed to by thread_info must have been allocated with acc_integration_mem_alloc.
 *
 * @param thread_handle A pointer to a acc_app_integration_thread_handle_t struct
 */
static void acc_app_integration_thread_work(void *thread_handle)
{
	acc_app_integration_thread_handle_t thread = (acc_app_integration_thread_handle_t)thread_handle;

	thread->func(thread->param);

	xSemaphoreGive(thread->stopped);
	vTaskDelete(NULL);
}


acc_app_integration_thread_handle_t acc_app_integration_thread_create(void (*func)(void *param), void *param, const char *name)
{
	assert(func != NULL);
	BaseType_t                          result;
	acc_app_integration_thread_handle_t thread = NULL;

	thread = pvPortMalloc(sizeof(*thread));

	if (thread == NULL)
	{
		return NULL;
	}

	thread->func  = func;
	thread->param = param;

	thread->stopped = xSemaphoreCreateBinary();
	if (thread->stopped == NULL)
	{
		vPortFree(thread);
		return NULL;
	}

	result = xTaskCreate(acc_app_integration_thread_work, name, ACC_APP_STACK_SIZE / sizeof(int), thread, tskIDLE_PRIORITY + 1,
	                     (TaskHandle_t *)&thread->handle);
	if (result != pdPASS)
	{
		vSemaphoreDelete(thread->stopped);
		vPortFree(thread);
		return NULL;
	}

	return thread;
}


acc_app_integration_mutex_t acc_app_integration_mutex_create(void)
{
	return (acc_app_integration_mutex_t)xSemaphoreCreateMutex();
}


void acc_app_integration_mutex_destroy(acc_app_integration_mutex_t mutex)
{
	assert(mutex != NULL);
	vSemaphoreDelete(mutex);
	mutex = NULL;
}


void acc_app_integration_mutex_lock(acc_app_integration_mutex_t mutex)
{
	assert(mutex != NULL);
	xSemaphoreTake((SemaphoreHandle_t)mutex, portMAX_DELAY);
}


void acc_app_integration_mutex_unlock(acc_app_integration_mutex_t mutex)
{
	assert(mutex != NULL);
	xSemaphoreGive(mutex);
}


/**
 * @brief Convert milliseconds to ticks
 *
 * Delaying one tick means "wait until next tick" in FreeRTOS, this means that
 * the elapsed time for 1 tick can be between 0 and (1 / configTICK_RATE_HZ) seconds
 *
 * @param[in] ms Number of milliseconds
 *
 * @return The amount of ticks to wait that corresponds to at least ms time
 */
static TickType_t ms_to_ticks(uint32_t ms)
{
	TickType_t ticks = 0;

	if (ms != 0)
	{
		ticks = (uint64_t)ms * (configTICK_RATE_HZ) / 1000 + 1;
		if ((((uint64_t)ms * (configTICK_RATE_HZ)) % 1000) != 0)
		{
			ticks++;
		}
	}

	return ticks;
}


void acc_app_integration_sleep_ms(uint32_t time_msec)
{
	vTaskDelay(ms_to_ticks(time_msec));
}


acc_app_integration_semaphore_t acc_app_integration_semaphore_create(void)
{
	return (acc_app_integration_semaphore_t)xSemaphoreCreateBinary();;
}


void acc_app_integration_semaphore_destroy(acc_app_integration_semaphore_t sem)
{
	assert(sem != NULL);
	vSemaphoreDelete(sem);
}


bool acc_app_integration_semaphore_wait(acc_app_integration_semaphore_t sem, uint16_t timeout_ms)
{
	assert(sem != NULL);
	return xSemaphoreTake((SemaphoreHandle_t)sem, ms_to_ticks(timeout_ms)) == pdTRUE;
}


static bool is_interrupt_context(void)
{
	uint32_t ipsr;

	__asm("mrs %0, ipsr" : "=r" (ipsr));
	return ipsr != 0;
}


void acc_app_integration_semaphore_signal(acc_app_integration_semaphore_t sem)
{
	assert(sem != NULL);
	if (is_interrupt_context())
	{
		BaseType_t xHigherPriorityTaskWoken;
		xSemaphoreGiveFromISR((SemaphoreHandle_t)sem, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
	else
	{
		xSemaphoreGive((SemaphoreHandle_t)sem);
	}
}


void *acc_app_integration_mem_alloc(size_t size)
{
	return pvPortMalloc(size);
}


void acc_app_integration_mem_free(void *ptr)
{
	vPortFree(ptr);
}
