// Copyright (c) Acconeer AB, 2016-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "acc_driver_uart_same70.h"

#include "acc_device_os.h"
#include "acc_device_pm.h"
#include "acc_device_uart.h"
#include "acc_log.h"

#include "board.h"
#include "pio.h"
#include "pmc.h"
#include "uart.h"
#include "irq/irq.h"
#include "serial/uartd.h"

/**
 * @brief The module name
 */
#define MODULE	"driver_uart_same70"
#define READ_BUFFER_SIZE 64

/**
 * Pin definitions per UART
 */

typedef struct {
	Uart *uart;
	const struct _pin *uart_pins;
	acc_device_uart_read_func_t *isr_read_callback;
	struct _uart_desc uart_config;
	int32_t error_count;
	uint8_t *read_buffer;
} acc_uart_description;

static const struct _pin uart0_pins[] = PINS_UART0;
static const struct _pin uart1_pins[] = PINS_UART1;
static const struct _pin uart2_pins[] = PINS_UART2;
static const struct _pin uart3_pins[] = PINS_UART3;
static const struct _pin uart4_pins[] = PINS_UART4_ALT;

static acc_uart_description uarts[] = {
		{
				.uart = UART0,
				.uart_pins = uart0_pins,
				.isr_read_callback = NULL,
				.uart_config = {0},
				.error_count = 0,
				.read_buffer = NULL,
		},
		{
				.uart = UART1,
				.uart_pins = uart1_pins,
				.isr_read_callback = NULL,
				.uart_config = {0},
				.error_count = 0,
				.read_buffer = NULL,
		},
		{
				.uart = UART2,
				.uart_pins = uart2_pins,
				.isr_read_callback = NULL,
				.uart_config = {0},
				.error_count = 0,
				.read_buffer = NULL,
		},
		{
				.uart = UART3,
				.uart_pins = uart3_pins,
				.isr_read_callback = NULL,
				.uart_config = {0},
				.error_count = 0,
				.read_buffer = NULL,
		},
		{
				.uart = UART4,
				.uart_pins = uart4_pins,
				.isr_read_callback = NULL,
				.uart_config = {0},
				.error_count = 0,
				.read_buffer = NULL,
		},
};

static uart_wait_for_transfer_complete_t wait_for_transfer_complete_func;
static uart_transfer_complete_callback_t transfer_complete_func;


static void acc_driver_uart_same70_register_read_callback(uint_fast8_t port, acc_device_uart_read_func_t *callback);


static int uart_error_callback(void *arg1, void *arg2)
{
	uint_fast8_t port = (uint_fast8_t)arg1;
	uint32_t status = (uint32_t)arg2;
	(void)status;
	uarts[port].error_count++;
	return 0;
}


static int uart_rx_callback(void *arg1, void *arg2)
{
	uint_fast8_t port = (uint_fast8_t)arg1;
	struct _buffer *rx_data = arg2;

	for (uint32_t ch = 0; ch < rx_data->size; ch++)
	{
		if(uarts[port].isr_read_callback != NULL)
		{
			uarts[port].isr_read_callback(port, rx_data->data[ch], 0);
		}
	}
	return 0;
}


/**
 * @brief Initialize UART
 *
 * Init UART, set baudrate and prepare transmit/receive buffers
 *
 * @param port     The UART to configure
 * @param baudrate Baudrate to use
 * @param options  Driver specific parameter
 * @return True if successful
 */
static bool acc_driver_uart_same70_init(uint_fast8_t port, uint32_t baudrate, acc_device_uart_options_t options)
{
	(void)options;

	if (!uarts[port].uart_config.addr)
	{
		pio_configure(uarts[port].uart_pins, 2);
		uarts[port].uart_config.addr = uarts[port].uart;
		uarts[port].uart_config.mode = UART_MR_CHMODE_NORMAL | UART_MR_PAR_NO;
		uarts[port].uart_config.baudrate = baudrate;
		uarts[port].uart_config.transfer_mode = UARTD_MODE_DMA;
		uarts[port].uart_config.error_callback.method = uart_error_callback;
		uarts[port].uart_config.error_callback.arg = (void*)(uintptr_t)port;
		uartd_configure(port, &uarts[port].uart_config);
	} else {
		/* Port already initiated, only update baudrate, but make sure that that we
		 * have transmitted all characters from the THR by checking the TXEMPTY.
		 */
		while (!uart_is_tx_empty(uarts[port].uart_config.addr))
		{
		}

		uarts[port].uart_config.baudrate = baudrate;
		uart_configure(uarts[port].uart_config.addr, uarts[port].uart_config.mode, uarts[port].uart_config.baudrate);
		// uart_configure disables all interrupts, enable the RX interrupt again since we know that it is needed
		uart_enable_it(uarts[port].uart_config.addr, US_IER_RXRDY);
	}

	ACC_LOG_VERBOSE("SAME70 UART driver initialized");

	return true;
}


static int uart_transfer_complete_callback(void *arg1, void *arg2)
{
	(void)arg2;
	uint_fast8_t port = (uint_fast8_t)arg1;
	if (NULL != transfer_complete_func)
	{
		transfer_complete_func(port);
	}
	return 0;
}


/**
 * @brief Send data on UART
 *
 * @param port The UART to use
 * @param data Data to be transmitted
 * @param length Length of data
 * @return True if successful, false otherwise
 */
static bool acc_driver_uart_same70_write(uint_fast8_t port, const uint8_t *data, size_t length)
{
	if (port >= UART_IFACE_COUNT)
	{
		return false;
	}

	struct _buffer buf = {
		.data = (uint8_t *)data,
		.size = length,
		.attr = UARTD_BUF_ATTR_WRITE,
	};

	struct _callback callback = {
		.method = uart_transfer_complete_callback,
		.arg = (void*)(uintptr_t)port
	};

	/* Prevent low power mode until uart transfer is completed */
	acc_device_pm_wake_lock();

	uint32_t result = uartd_transfer(port, &buf, &callback);
	if (result == UARTD_SUCCESS)
	{
		if (NULL != wait_for_transfer_complete_func)
		{
			wait_for_transfer_complete_func(port);
		}

		uartd_wait_tx_transfer(port);
	}

	acc_device_pm_wake_unlock();

	return result == UARTD_SUCCESS;
}


static void acc_driver_uart_same70_register_read_callback(uint_fast8_t port, acc_device_uart_read_func_t *callback)
{
	if (callback != NULL)
	{
		struct _buffer buf = {
			.data = NULL,
			.size = READ_BUFFER_SIZE,
			.attr = UARTD_BUF_ATTR_READ,
		};

		if (uarts[port].read_buffer == NULL)
		{
			uarts[port].read_buffer = acc_os_mem_alloc(READ_BUFFER_SIZE + L1_CACHE_BYTES);
			assert(uarts[port].read_buffer != NULL);
			uarts[port].read_buffer = (void *)(((uintptr_t)uarts[port].read_buffer + L1_CACHE_BYTES - 1) & ~(L1_CACHE_BYTES - 1));
		}
		buf.data = uarts[port].read_buffer;

		struct _callback callback = {
			.method = uart_rx_callback,
			.arg = (void*)(uintptr_t)port
		};

		uint32_t result = uartd_transfer(port, &buf, &callback);
		assert(result == UARTD_SUCCESS);
	}
	uarts[port].isr_read_callback = callback;
}


static int32_t acc_driver_uart_same70_get_error_count(uint_fast8_t port)
{
	return uarts[port].error_count;
}


static void acc_driver_uart_same70_deinit(uint_fast8_t port)
{
	uint32_t id = get_uart_id_from_addr(uarts[port].uart_config.addr);
	irq_disable(id);

	dma_stop_transfer(uarts[port].uart_config.dma.tx.channel);
	dma_stop_transfer(uarts[port].uart_config.dma.rx.channel);
	dma_free_channel(uarts[port].uart_config.dma.tx.channel);
	dma_free_channel(uarts[port].uart_config.dma.rx.channel);
}


/**
 * @brief Request driver to register with appropriate device(s)
 */
extern void acc_driver_uart_same70_register(uart_wait_for_transfer_complete_t wait_function,
                                            uart_transfer_complete_callback_t transfer_complete)
{
	acc_device_uart_init_func		= acc_driver_uart_same70_init;
	acc_device_uart_write_func		= acc_driver_uart_same70_write;
	acc_device_uart_register_read_func	= acc_driver_uart_same70_register_read_callback;
	acc_device_uart_get_error_count_func = acc_driver_uart_same70_get_error_count;
	acc_device_uart_deinit_func          = acc_driver_uart_same70_deinit;

	wait_for_transfer_complete_func = wait_function;
	transfer_complete_func = transfer_complete;
}
