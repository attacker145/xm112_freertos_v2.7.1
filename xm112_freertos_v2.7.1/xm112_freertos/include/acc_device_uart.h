// Copyright (c) Acconeer AB, 2016-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#ifndef ACC_DEVICE_UART_H_
#define ACC_DEVICE_UART_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


#define ACC_DEVICE_UART_MAX	4


/**
 * @brief Driver specific parameter
 */
typedef enum {
	ACC_DEVICE_UART_OPTIONS_ALT_PINS_1 = 1,
	ACC_DEVICE_UART_OPTIONS_ALT_PINS_2 = 2,
	ACC_DEVICE_UART_OPTIONS_ALT_PINS_3 = 4
} acc_device_uart_options_enum_t;
typedef uint32_t acc_device_uart_options_t;


typedef void (acc_device_uart_read_func_t)(uint_fast8_t port, uint8_t data, uint32_t status);

// These functions are to be used by drivers only, do not use them directly
extern bool	(*acc_device_uart_init_func)(uint_fast8_t port, uint32_t baudrate, acc_device_uart_options_t options);
extern bool	(*acc_device_uart_write_func)(uint_fast8_t port, const uint8_t *data, size_t length);
extern void		(*acc_device_uart_register_read_func)(uint_fast8_t port, acc_device_uart_read_func_t *callback);
extern int32_t (*acc_device_uart_get_error_count_func)(uint_fast8_t port);
extern void    (*acc_device_uart_deinit_func)(uint_fast8_t port);


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
extern bool acc_device_uart_init(uint_fast8_t port, uint32_t baudrate, acc_device_uart_options_t options);


/**
 * @brief Send data on UART
 *
 * @param port The UART to use
 * @param data The data to be transmitted
 * @return True if successful, false otherwise
 */
extern bool acc_device_uart_write(
		uint_fast8_t	port,
		uint8_t		data);


/**
 * @brief Send array of data on UART
 *
 * @param port        The UART to use
 * @param buffer      The data buffer to be transmitted
 * @param buffer_size The size of the buffer
 * @return True if successful, false otherwise
 */
extern bool acc_device_uart_write_buffer(
		uint_fast8_t	port,
		const void	*buffer,
		size_t		buffer_size);


/**
 * @brief Used by the client to register a read callback
 * @param port	the UART port
 * @param callback Function pointer to the callback
 *
 */
extern void acc_device_uart_register_read_callback(uint_fast8_t port, acc_device_uart_read_func_t *callback);

/**
 * @brief Get the error count, typically overrun errors when receiving data
 * @param port the UART port
 *
 * @return 0 if no errors were found or -1 if not implemented
 */
int32_t acc_device_uart_get_error_count(uint_fast8_t port);

/**
 * @brief Deinitialize UART
 *
 * @param port the UART port
 *
 */
void acc_device_uart_deinit(uint_fast8_t port);


#ifdef __cplusplus
}
#endif

#endif
