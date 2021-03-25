// Copyright (c) Acconeer AB, 2019-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#ifndef ACC_APP_INTEGRATION_H_
#define ACC_APP_INTEGRATION_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


typedef uint32_t acc_app_integration_thread_id_t;

struct acc_app_integration_thread_handle;

typedef struct acc_app_integration_thread_handle *acc_app_integration_thread_handle_t;

struct acc_app_integration_mutex;

typedef struct acc_app_integration_mutex *acc_app_integration_mutex_t;

struct acc_app_integration_semaphore;

typedef struct acc_app_integration_semaphore *acc_app_integration_semaphore_t;


/**
 * @brief Create thread function
 *
 * @param func Thread func
 * @param param Thread func parameters
 * @param name Name of thread
 *
 * @return A thread handle
 */
acc_app_integration_thread_handle_t acc_app_integration_thread_create(void (*func)(void *param), void *param, const char *name);


/**
 * @brief Clean up thread
 *
 * @param handle A thread handle
 */
void acc_app_integration_thread_cleanup(acc_app_integration_thread_handle_t handle);


/**
 * @brief Create a mutex
 *
 * @return A mutex
 */
acc_app_integration_mutex_t acc_app_integration_mutex_create(void);


/**
 * @brief Destroy a mutex
 *
 * @param mutex A mutex
 */
void acc_app_integration_mutex_destroy(acc_app_integration_mutex_t mutex);


/**
 * @brief Lock a mutex
 *
 * @param mutex A mutex
 */
void acc_app_integration_mutex_lock(acc_app_integration_mutex_t mutex);


/**
 * @brief Unlock a mutex
 *
 * @param mutex A mutex
 */
void acc_app_integration_mutex_unlock(acc_app_integration_mutex_t mutex);


/**
 * @brief Sleep for a specified number of microseconds
 *
 * @param time_usec Time in microseconds to sleep
 */
void acc_app_integration_sleep_us(uint32_t time_usec);


/**
 * @brief Sleep for a specified number of milliseconds
 *
 * @param time_msec Time in milliseconds to sleep
 */
void acc_app_integration_sleep_ms(uint32_t time_msec);


/**
 * @brief Set up a periodic timer used to wake up the system from sleep
 *
 * This function will start a periodic timer with the specified time.
 * This is useful when the drift of the wakeup interval should be kept
 * at a minimum.
 *
 * @param time_msec Time in milliseconds
 */
void acc_app_integration_set_periodic_wakeup(uint32_t time_msec);


/**
 * @brief Put the system in sleep until the periodic timer triggers
 *
 * The periodic timer must be started using
 * @ref acc_app_integration_set_periodic_wakeup prior to
 * invoking this function.
 * The target specific implementation of this function will determine
 * the sleep depth based on the set sleep interval and it will be a
 * trade-off between wake-up latency and power consumption.
 */
void acc_app_integration_sleep_until_periodic_wakeup(void);


/**
 * @brief Gets the current time from the low power timer used for power management
 *
 * @return Returns the current time in ms
 */
uint32_t acc_app_integration_get_current_time(void);


/**
 * @brief Creates a semaphore and returns a pointer to the newly created semaphore
 *
 * @return A pointer to the semaphore on success otherwise NULL
 */
acc_app_integration_semaphore_t acc_app_integration_semaphore_create(void);


/**
 * @brief Waits for the semaphore to be available. The task calling this function will be
 * blocked until the semaphore is signaled from another task.
 *
 * @param[in]  sem A pointer to the semaphore to use
 * @param[in]  timeout_ms The amount of time to wait before a timeout occurs
 * @return Returns true on success and false on timeout
 */
bool acc_app_integration_semaphore_wait(acc_app_integration_semaphore_t sem, uint16_t timeout_ms);


/**
 * @brief Signal the semaphore.
 *
 * @param[in]  sem A pointer to the semaphore to signal
 */
void acc_app_integration_semaphore_signal(acc_app_integration_semaphore_t sem);


/**
 * @brief Deallocates the semaphore
 *
 * @param[in]  sem A pointer to the semaphore to deallocate
 */
void acc_app_integration_semaphore_destroy(acc_app_integration_semaphore_t sem);


/**
 * @brief Allocate dynamic memory
 *
 * @param[in]  size The bytesize of the reuested memory block
 * @return Returns either NULL or a unique pointer to a memory block
 */
void *acc_app_integration_mem_alloc(size_t size);


/**
 * @brief Free dynamic memory
 *
 * @param[in]  ptr A pointer to the memory space to be freed
 */
void acc_app_integration_mem_free(void *ptr);


#endif
