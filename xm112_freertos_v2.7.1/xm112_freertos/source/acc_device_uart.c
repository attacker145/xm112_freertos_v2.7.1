// Copyright (c) Acconeer AB, 2016-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#include <stdint.h>
#include <string.h>

#include "acc_device_uart.h"
#include "acc_log.h"

/**
 * @brief The module name
 */
#define MODULE "device_uart"


// These functions are to be used by drivers only, do not use them directly
bool    (*acc_device_uart_init_func)(uint_fast8_t port, uint32_t baudrate, acc_device_uart_options_t options) = NULL;
bool    (*acc_device_uart_write_func)(uint_fast8_t port, const uint8_t *data, size_t length) = NULL;
void    (*acc_device_uart_register_read_func)(uint_fast8_t port, acc_device_uart_read_func_t *callback) = NULL;
int32_t (*acc_device_uart_get_error_count_func)(uint_fast8_t port) = NULL;
void    (*acc_device_uart_deinit_func)(uint_fast8_t port) = NULL;


/**
 * @brief Initialize UART
 *
 * Init UART, set baudrate and prepare transmit/receive buffers
 *
 * @param port     The UART to configure
 * @param baudrate Baudrate to use
 * @param options  Driver specific parameter
 * @return True if successful, false otherwise
 */
bool acc_device_uart_init(uint_fast8_t port, uint32_t baudrate, acc_device_uart_options_t options)
{
	if (!acc_device_uart_init_func)
	{
		return false;
	}

	return acc_device_uart_init_func(port, baudrate, options);
}


/**
 * @brief Send data on UART
 *
 * @param port The UART to use
 * @param data The data to be transmitted
 * @return True if successful, false otherwise
 */
bool acc_device_uart_write(
	uint_fast8_t port,
	uint8_t      data)
{
	if (acc_device_uart_write_func == NULL)
	{
		return false;
	}

	return acc_device_uart_write_func(port, &data, 1);
}


/**
 * @brief Send array of data on UART
 *
 * @param port        The UART to use
 * @param buffer      The data buffer to be transmitted
 * @param buffer_size The size of the buffer
 * @return True if successful, false otherwise
 */
bool acc_device_uart_write_buffer(
	uint_fast8_t port,
	const void   *buffer,
	size_t       buffer_size)
{
	if (acc_device_uart_write_func == NULL)
	{
		return false;
	}

	return acc_device_uart_write_func(port, (const uint8_t *)buffer, buffer_size);
}


void acc_device_uart_register_read_callback(uint_fast8_t port, acc_device_uart_read_func_t *callback)
{
	if (acc_device_uart_register_read_func != NULL)
	{
		acc_device_uart_register_read_func(port, callback);
	}
}


int32_t acc_device_uart_get_error_count(uint_fast8_t port)
{
	if (acc_device_uart_get_error_count_func != NULL)
	{
		return acc_device_uart_get_error_count_func(port);
	}
	return -1;
}


void acc_device_uart_deinit(uint_fast8_t port)
{
	if (acc_device_uart_deinit_func != NULL)
	{
		acc_device_uart_deinit_func(port);
	}
}
