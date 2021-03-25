// Copyright (c) Acconeer AB, 2018-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#ifndef ACC_MS_SYSTEM_H_
#define ACC_MS_SYSTEM_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "acc_hal_definitions.h"

typedef void (*acc_ms_sensor_interrupt_callback_t)(void);

typedef void (*acc_ms_system_uart_read_func_t)(uint8_t data, uint32_t status);


/**
 * Get the reference to the hal implementation that should be used.
 *
 * @return a reference to a acc_hal_t implementation
 */
const acc_hal_t *acc_ms_system_get_hal_implementation(void);


/**
 * @brief Get the product id
 *
 * @return The product id
 */
uint32_t acc_ms_system_get_product_id(void);


/**
 * @brief Set the power state the system can go at the lowest
 *
 * @param[in] req_power_state power state
 */
void acc_ms_system_set_lowest_power_state(uint32_t req_power_state);


/**
 * @brief Used by the client to register a read callback
 *
 * @param callback Function pointer to the callback, NULL to disable
 */
void acc_ms_system_uart_register_read_callback(acc_ms_system_uart_read_func_t callback);


/*
 * Get max supported baudrate
 *
 * @return The maximum supported baudrate for this module
 */
uint32_t acc_ms_system_get_max_uart_baudrate(void);


/**
 * @brief Set baudrate of the UART
 *
 * @param baudrate Baudrate to use
 *
 */
void acc_ms_system_uart_set_baudrate(uint32_t baudrate);


/**
 * @brief Send array of data on UART
 *
 * @param buffer      The data buffer to be transmitted
 * @param buffer_size The size of the buffer
 *
 * @return True if successful, false otherwise
 */
bool acc_ms_system_uart_write_buffer(const void *buffer, size_t buffer_size);


/**
 * @brief Get the error count for all UART ports, typically overrun errors when receiving data
 *
 * @return 0 if no errors occurred or -1 if not implemented
 */
int32_t acc_ms_system_uart_get_error_count(void);


/**
 * Enter a critical section
 */
void acc_ms_system_critical_section_enter(void);


/**
 * Exit a critical section
 */
void acc_ms_system_critical_section_exit(void);


/**
 * @brief Get current time
 *
 * It is important that this value wraps correctly and uses all bits. I.e. it should count
 * upwards to 2^32 - 1 and then 0 again.
 *
 * @returns Current time as milliseconds
 */
uint32_t acc_ms_system_get_time(void);


/**
 * @brief Allocate dynamic memory
 *
 * Use platform specific mechanism to allocate dynamic memory. The memory is guaranteed
 * to be naturally aligned. Requesting zero bytes will return NULL.
 *
 * On error, NULL is returned.
 *
 * @param size The number of bytes to allocate
 *
 * @return Pointer to the allocated memory, or NULL if allocation failed
 */
void *acc_ms_system_mem_alloc(size_t size);


/**
 * @brief Free dynamic memory allocated with acc_os_mem_alloc
 *
 * Use platform specific mechanism to free dynamic memory. Passing NULL is allowed
 * but will do nothing.
 *
 * Freeing memory not allocated with acc_os_mem_alloc, or freeing memory already
 * freed, will result in undefined behavior.
 *
 * @param ptr Pointer to the dynamic memory to free
 */
void acc_ms_system_mem_free(void *ptr);


/**
 * @brief Enable and disable IRQ
 *
 * @param enable True for enable and false for disable
 */
void acc_ms_system_enable_irq(bool enable);


/**
 * @brief Function to register an ISR callback.
 *
 * @param callback Function that shall be called when the sensor interrupt is activated.
 */
void acc_ms_system_register_sensor_interrupt_callback(acc_ms_sensor_interrupt_callback_t callback);


/**
 * @brief Function to get the current state of the sensor interrupt.
 *
 * @returns true if the sensor interrupt is active
 */
bool acc_ms_system_is_sensor_interrupt_active(void);


/**
 * Signal that a new message have been posted to any of the queues
 */
void acc_ms_system_signal_message(void);


/*
 * Wait until a message have been received
 *
 * @param[in] timeout Maximum amount of time to wait in ms
 */
void acc_ms_system_wait_for_message(uint32_t timeout);


/**
 * Initialization function called during startup
 */
void acc_ms_system_init(void);


#endif
