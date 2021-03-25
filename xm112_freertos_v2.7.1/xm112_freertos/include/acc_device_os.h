// Copyright (c) Acconeer AB, 2016-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#ifndef ACC_DEVICE_OS_H_
#define ACC_DEVICE_OS_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h> // For strrchr

#include "acc_app_integration.h"


/**
 * @brief Perform any os specific initialization
 */
void acc_os_init(void);


/**
 * @brief Prepare stack for measuring stack usage - to be called as early as possible
 *
 * @param stack_size Amount of stack in bytes that is allocated
 */
void acc_os_stack_setup(size_t stack_size);


/**
 * @brief Measure amount of used stack in bytes
 *
 * @param stack_size Amount of stack in bytes that is allocated
 * @return Number of bytes of used stack space
 */
size_t acc_os_stack_get_usage(size_t stack_size);


/**
 * @brief Sleep for a specified number of microseconds
 *
 * @param time_usec Time in microseconds to sleep
 */
void acc_os_sleep_us(uint32_t time_usec);


/**
 * @brief Sleep for a specified number of milliseconds
 *
 * @param time_msec Time in milliseconds to sleep
 */
void acc_os_sleep_ms(uint32_t time_msec);


/**
 * @brief Allocate dynamic memory
 *
 * Use platform specific mechanism to allocate dynamic memory. The memory is guaranteed
 * to be naturally aligned. Requesting zero bytes will return NULL.
 *
 * On error, NULL is returned.
 *
 * @param size The number of bytes to allocate
 * @param file The file which makes the allocation
 * @param line The line where this allocation takes place
 * @return Pointer to the allocated memory, or NULL if allocation failed
 */
#define REL_FILE_PATH       (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define acc_os_mem_alloc(X) acc_os_mem_alloc_debug(X, REL_FILE_PATH, __LINE__)
void *acc_os_mem_alloc_debug(size_t size, const char *file, uint16_t line);


/**
 * @brief Allocate dynamic memory and initialize it with zeros.
 *
 * Use platform specific mechanism to allocate dynamic memory. The memory is guaranteed
 * to be naturally aligned. If either num or size is zero, NULL is returned.
 *
 * On error, NULL is returned.
 *
 * @param num The number of elements to allocate.
 * @param size The size of each element.
 * @param file The file which makes the allocation.
 * @param line The line where this allocation takes place
 * @return Pointer to the allocated memory, or NULL if allocation failed
 */
void *acc_os_mem_calloc_debug(size_t num, size_t size, const char *file, uint16_t line);


#define acc_os_mem_calloc(X, Y) acc_os_mem_calloc_debug(X, Y, REL_FILE_PATH, __LINE__)


/**
 * @brief Free dynamic memory allocated with acc_os_mem_alloc
 *
 * Use platform specific mechanism to free dynamic memory. Passing NULL is allowed
 * but will do nothing.
 *
 * Freeing memory not allocated with acc_os_mem_alloc, or freeing memory already
 * freed, will result in undefined behaviour.
 *
 * @param ptr Pointer to the dynamic memory to free
 */
void acc_os_mem_free(void *ptr);


/**
 * @brief Return the unique thread ID for the current thread
 */
acc_app_integration_thread_id_t acc_os_get_thread_id(void);


/**
 * @brief Get current time and return as milliseconds
 *
 * @return Time in milliseconds is returned here
 */
uint32_t acc_os_get_time(void);


/**
 * @brief Create a mutex
 *
 * @return Handle to an initialized mutex, NULL if creation fails
 */
acc_app_integration_mutex_t acc_os_mutex_create(void);


/**
 * @brief Mutex lock
 *
 * @param mutex Mutex to be locked
 */
void acc_os_mutex_lock(acc_app_integration_mutex_t mutex);


/**
 * @brief Mutex unlock
 *
 * @param mutex Mutex to be unlocked
 */
void acc_os_mutex_unlock(acc_app_integration_mutex_t mutex);


/**
 * @brief Destroys the mutex
 *
 * @param mutex Mutex to be destroyed
 */
void acc_os_mutex_destroy(acc_app_integration_mutex_t mutex);


/**
 * @brief Create new thread
 * If you want to run in single thread mode, make sure to NOT
 * implement this function.
 *
 * @param func	Function implementing the thread code
 * @param param	Parameter to be passed to the thread function
 * @param name	Name of the thread used for debugging
 * @return A handle to a newly created thread
 */
acc_app_integration_thread_handle_t acc_os_thread_create(void (*func)(void *param), void *param, const char *name);


/**
 * @brief Exit current thread
 *
 * The last function called by a thread before exiting, may or may not return.
 */
void acc_os_thread_exit(void);


/**
 * @brief Cleanup after thread termination
 *
 * For operating systems that require it, perform any post-thread cleanup operation.
 *
 * @param thread Handle of thread
 */
void acc_os_thread_cleanup(acc_app_integration_thread_handle_t thread);


/**
 * @brief Creates a semaphore and returns a pointer to the newly created semaphore
 *
 * @return A pointer to the semaphore on success otherwise NULL
 */
acc_app_integration_semaphore_t acc_os_semaphore_create(void);


/**
 * @brief Waits for the semaphore to be available. The task calling this function will be
 * blocked until the semaphore is signaled from another task.
 *
 * @param[in]  sem A pointer to the semaphore to use
 * @param[in]  timeout_ms The amount of time to wait before a timeout occurs
 * @return Returns true on success and false on timeout
 */
bool acc_os_semaphore_wait(acc_app_integration_semaphore_t sem, uint16_t timeout_ms);


/**
 * @brief Signal the semaphore. Not ISR safe. If needed from an ISR
 * use acc_os_semaphore_signal_from_interrupt instead. Releases the semaphore resulting
 * in a release of the task that is blocked waiting for the semaphore.
 *
 * @param[in]  sem A pointer to the semaphore to signal
 */
void acc_os_semaphore_signal(acc_app_integration_semaphore_t sem);


/**
 * @brief Signal the semaphore. This routine is safe to call from an
 * ISR routine
 *
 * @param[in]  sem A pointer to the semaphore to signal
 */
void acc_os_semaphore_signal_from_interrupt(acc_app_integration_semaphore_t sem);


/**
 * @brief Deallocates the semaphore
 *
 * @param[in]  sem A pointer to the semaphore to deallocate
 */
void acc_os_semaphore_destroy(acc_app_integration_semaphore_t sem);


/**
 * @brief Tell whether or not the system has multithread support
 *
 * @return True if multithread is supported, false for single thread
 */
bool acc_os_multithread_support(void);


#endif
