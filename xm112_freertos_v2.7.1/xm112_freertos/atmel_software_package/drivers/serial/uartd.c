/* ----------------------------------------------------------------------------
 *         SAM Software Package License
 * ----------------------------------------------------------------------------
 * Copyright (c) 2016, Atmel Corporation
 * Copyright (c) 2018, Acconeer AB
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "callback.h"
#include "chip.h"
#include "dma/dma.h"
#include "io.h"
#include "irq/irq.h"
#include "mm/cache.h"
#include "mutex.h"
#include "peripherals/pmc.h"
#include "serial/uart.h"
#include "serial/uartd.h"
#include "trace.h"

/*----------------------------------------------------------------------------
 *        Definition
 *----------------------------------------------------------------------------*/

#define UARTD_ATTRIBUTE_MASK     (0)
#define UARTD_POLLING_THRESHOLD  16
#define UART_RX_INTERRUPTS (UART_IER_RXRDY | UART_IER_OVRE | UART_IER_FRAME | UART_IER_PARE)

static struct _uart_desc *_serial[UART_IFACE_COUNT];

/*----------------------------------------------------------------------------
 *        Internal functions
 *----------------------------------------------------------------------------*/

static int _uartd_dma_write_callback(void* arg, void* arg2)
{
	uint8_t iface = (uint32_t)arg;
	assert(iface < UART_IFACE_COUNT);

	dma_reset_channel(_serial[iface]->dma.tx.channel);

	mutex_unlock(&_serial[iface]->tx.mutex);

	callback_call(&_serial[iface]->tx.callback, NULL);

	return 0;
}

static int _uartd_dma_read_callback(void* arg, void* arg2)
{
	uint8_t iface = (uint32_t)arg;
	assert(iface < UART_IFACE_COUNT);
	struct _uart_desc *desc = _serial[iface];
	struct _dma_channel* channel = desc->dma.rx.channel;

	if (!dma_is_transfer_done(channel))
		dma_stop_transfer(channel);
	dma_fifo_flush(channel);

	desc->rx.transferred = dma_get_transferred_data_len(channel, desc->dma.rx.cfg_dma.chunk_size, desc->dma.rx.cfg.len);
	dma_reset_channel(desc->dma.rx.channel);

	if (desc->rx.transferred > 0)
		cache_invalidate_region(desc->dma.rx.cfg.daddr, desc->rx.transferred);

	desc->rx.buffer.size = 0;

	mutex_unlock(&desc->rx.mutex);

	callback_call(&desc->rx.callback, NULL);

	return 0;
}

static void _uartd_dma_read(uint8_t iface)
{
	struct _callback _cb;
	assert(iface < UART_IFACE_COUNT);
	struct _uart_desc* desc = _serial[iface];

	memset(&desc->dma.rx.cfg, 0x0, sizeof(desc->dma.rx.cfg));

	desc->dma.rx.cfg.saddr = (void *)&desc->addr->UART_RHR;
	desc->dma.rx.cfg.daddr = desc->rx.buffer.data;
	desc->dma.rx.cfg.len = desc->rx.buffer.size;
	dma_configure_transfer(desc->dma.rx.channel, &desc->dma.rx.cfg_dma, &desc->dma.rx.cfg, 1);

	callback_set(&_cb, _uartd_dma_read_callback, (void*)(uint32_t)iface);
	cache_clean_region(desc->rx.buffer.data, desc->rx.buffer.size);
	dma_set_callback(desc->dma.rx.channel, &_cb);
	dma_start_transfer(desc->dma.rx.channel);
	uart_enable_it(desc->addr, UART_RX_INTERRUPTS);
}

static void _uartd_dma_write(uint8_t iface)
{
	struct _callback _cb;
	assert(iface < UART_IFACE_COUNT);
	struct _uart_desc* desc = _serial[iface];
	struct _dma_transfer_cfg cfg;

	cfg.saddr = desc->tx.buffer.data;
	cfg.daddr = (void *)&desc->addr->UART_THR;
	cfg.len = desc->tx.buffer.size;
	dma_configure_transfer(desc->dma.tx.channel, &desc->dma.tx.cfg_dma, &cfg, 1);

	callback_set(&_cb, _uartd_dma_write_callback, (void*)(uint32_t)iface);
	dma_set_callback(desc->dma.tx.channel, &_cb);
	cache_clean_region(cfg.saddr, cfg.len);
	dma_start_transfer(desc->dma.tx.channel);
}

static void _report_rx_data(struct _uart_desc* desc, uint8_t* data, uint32_t len)
{
	struct _buffer rx_data = {
		.data = data,
		.size = len,
		.attr = 0
	};
	callback_call(&desc->rx.callback, &rx_data);
}

static void _uartd_handler(uint32_t source, void* user_arg)
{
	int iface;
	uint32_t status = 0;
	Uart* addr = get_uart_addr_from_id(source);
	bool _rx_stop = true;
	bool _tx_stop = true;

	for (iface = 0; iface < UART_IFACE_COUNT; iface++) {
		if (_serial[iface]->addr == addr) {
			status = 1;
			break;
		}
	}

	if (!status) {
		/* async descriptor not found, disable interrupt */
		uart_disable_it(addr, UART_IDR_RXRDY | UART_IDR_TXRDY | UART_IDR_TXEMPTY);
		return;
	}

	struct _uart_desc* desc = _serial[iface];

	/* Make sure that we are in async mode, otherwise this is handled outside
	 * this interrupt
	 */
	if (desc->transfer_mode == UARTD_MODE_ASYNC) {
		status = uart_get_masked_status(addr);

		if (UART_STATUS_RXRDY(status)) {
			if (desc->rx.buffer.size) {
				desc->rx.buffer.data[desc->rx.transferred] = uart_get_char(addr);
				desc->rx.transferred++;

				if (desc->rx.transferred >= desc->rx.buffer.size)
					uart_disable_it(addr, UART_IDR_RXRDY);
				else
					_rx_stop = false;
			}
		}

		if (UART_STATUS_TXRDY(status)) {
			if (desc->tx.buffer.size) {
				uart_put_char(addr, desc->tx.buffer.data[desc->tx.transferred]);
				desc->tx.transferred++;

				if (desc->tx.transferred > desc->tx.buffer.size) {
					uart_disable_it(addr, UART_IDR_TXRDY);
					uart_enable_it(addr, UART_IDR_TXEMPTY);
				}
				_tx_stop = false;
			}
		}

		if (UART_STATUS_TXEMPTY(status)) {
			uart_disable_it(addr, UART_IDR_TXEMPTY);
		}

		if (_rx_stop) {
			desc->rx.buffer.size = 0;
			mutex_unlock(&desc->rx.mutex);
		}
		if (_tx_stop) {
			desc->tx.buffer.size = 0;
			mutex_unlock(&desc->tx.mutex);
		}
	} else if (desc->transfer_mode == UARTD_MODE_DMA) {
		/* In DMA the RXRDY bit in the status register might already been cleared by the DMA
		 * so assume that a byte has been received.
		 */
		if (desc->rx.buffer.size) {
			// At least one more byte received, flush DMA and check how much data we have.
			dma_fifo_flush(desc->dma.rx.channel);
			uint32_t transferred = dma_get_transferred_data_len(desc->dma.rx.channel,
			                                                    desc->dma.rx.cfg_dma.chunk_size, desc->dma.rx.cfg.len);

			//Invalidate entire rx buffer
			cache_invalidate_region(desc->dma.rx.cfg.daddr,
			                        desc->dma.rx.cfg.len);

			if (transferred < desc->rx.transferred) {
				// DMA have looped since last time, first send the last part of the buffer
				_report_rx_data(desc,
				                (uint8_t *)desc->dma.rx.cfg.daddr + desc->rx.transferred,
				                desc->dma.rx.cfg.len - desc->rx.transferred);
				desc->rx.transferred = 0;
			}

			int32_t received = transferred - desc->rx.transferred;
			if (received) {
				_report_rx_data(desc,
				                (uint8_t *)desc->dma.rx.cfg.daddr + desc->rx.transferred,
				                received);
			}

			desc->rx.transferred = transferred;
		}
	}
}

void uartd_configure(uint8_t iface, struct _uart_desc* config)
{
	uint32_t id = get_uart_id_from_addr(config->addr);
	assert(id < ID_PERIPH_COUNT);
	assert(iface < UART_IFACE_COUNT);

	_serial[iface] = config;

	pmc_configure_peripheral(id, NULL, true);
	uart_configure(config->addr, config->mode, config->baudrate);
	irq_add_handler(get_uart_id_from_addr(config->addr), _uartd_handler, NULL);
	/* Enable UART interrupt */
	irq_enable(id);

	config->dma.rx.cfg_dma.incr_saddr = false;
	config->dma.rx.cfg_dma.incr_daddr = true;
	config->dma.rx.cfg_dma.loop = true;
	config->dma.rx.cfg_dma.data_width = DMA_DATA_WIDTH_BYTE;
	config->dma.rx.cfg_dma.chunk_size = DMA_CHUNK_SIZE_1;

	config->dma.tx.cfg_dma.incr_saddr = true;
	config->dma.tx.cfg_dma.incr_daddr = false;
	config->dma.tx.cfg_dma.loop = false;
	config->dma.tx.cfg_dma.data_width = DMA_DATA_WIDTH_BYTE;
	config->dma.tx.cfg_dma.chunk_size = DMA_CHUNK_SIZE_1;

	config->dma.rx.channel = dma_allocate_channel(id, DMA_PERIPH_MEMORY);
	assert(config->dma.rx.channel);

	config->dma.tx.channel = dma_allocate_channel(DMA_PERIPH_MEMORY, id);
	assert(config->dma.tx.channel);
}

uint32_t uartd_transfer(uint8_t iface, struct _buffer* buf, struct _callback* cb)
{
	assert(iface < UART_IFACE_COUNT);
	struct _uart_desc *desc = _serial[iface];
	uint8_t tmode;
	uint32_t i;

	if ((buf == NULL) || (buf->size == 0))
		return UARTD_SUCCESS;

	if (buf->attr & UARTD_BUF_ATTR_READ) {
		if (!mutex_try_lock(&desc->rx.mutex))
			return UARTD_ERROR_LOCK;

		desc->rx.transferred = 0;
		desc->rx.buffer.data = buf->data;
		desc->rx.buffer.size = buf->size;
		desc->rx.buffer.attr = buf->attr;
		callback_copy(&desc->rx.callback, cb);
	}

	if (buf->attr & UARTD_BUF_ATTR_WRITE) {
		if (!mutex_try_lock(&desc->tx.mutex))
			return UARTD_ERROR_LOCK;

		desc->tx.transferred = 0;
		desc->tx.buffer.data = buf->data;
		desc->tx.buffer.size = buf->size;
		desc->tx.buffer.attr = buf->attr;
		callback_copy(&desc->tx.callback, cb);
	}

	tmode = desc->transfer_mode;

	/* If short tx transfer detected, use POLLING mode */
	if (tmode != UARTD_MODE_POLLING)
		if ((buf->size < UARTD_POLLING_THRESHOLD) && (buf->attr & UARTD_BUF_ATTR_WRITE))
			tmode = UARTD_MODE_POLLING;

	switch (tmode) {
	case UARTD_MODE_POLLING:
		i = 0;

		while (i < buf->size) {
			if (buf->attr & UARTD_BUF_ATTR_WRITE) {
				if (i < desc->tx.buffer.size) {
					{
						uart_put_char(desc->addr, desc->tx.buffer.data[i]);
						i++;
					}
					desc->tx.transferred = i;

					if (desc->tx.transferred >= desc->tx.buffer.size) {
						desc->tx.buffer.size = 0;
						mutex_unlock(&desc->tx.mutex);
						callback_call(&desc->tx.callback, NULL);
					}
				}
			}
			if (buf->attr & UARTD_BUF_ATTR_READ) {
				if (i < desc->rx.buffer.size) {
					{
						desc->rx.buffer.data[i] = uart_get_char(desc->addr);
						i++;
					}
					desc->rx.transferred = i;

					if (desc->rx.transferred >= desc->rx.buffer.size) {
						desc->rx.buffer.size = 0;
						mutex_unlock(&desc->rx.mutex);
						callback_call(&desc->rx.callback, NULL);
					}
				}
			}
		}
		break;

	case UARTD_MODE_ASYNC:
		if (buf->attr & UARTD_BUF_ATTR_WRITE)
			uart_enable_it(desc->addr, US_IER_TXRDY);

		if (buf->attr & UARTD_BUF_ATTR_READ) {
			uart_enable_it(desc->addr, US_IER_RXRDY);
		}
		break;

	case UARTD_MODE_DMA:
		if (buf->attr & UARTD_BUF_ATTR_WRITE)
			_uartd_dma_write(iface);
		if (buf->attr & UARTD_BUF_ATTR_READ)
			_uartd_dma_read(iface);
		break;

	default:
		trace_fatal("Unknown Uart mode!\r\n");
	}

	return UARTD_SUCCESS;
}

void uartd_finish_rx_transfer(uint8_t iface)
{
	assert(iface < UART_IFACE_COUNT);
	mutex_unlock(&_serial[iface]->rx.mutex);
}

void uartd_finish_tx_transfer(uint8_t iface)
{
	assert(iface < UART_IFACE_COUNT);
	mutex_unlock(&_serial[iface]->tx.mutex);
}

uint32_t uartd_rx_is_busy(const uint8_t iface)
{
	assert(iface < UART_IFACE_COUNT);
	return mutex_is_locked(&_serial[iface]->rx.mutex);
}

uint32_t uartd_tx_is_busy(const uint8_t iface)
{
	assert(iface < UART_IFACE_COUNT);
	return mutex_is_locked(&_serial[iface]->tx.mutex);
}

void uartd_wait_rx_transfer(const uint8_t iface)
{
	assert(iface < UART_IFACE_COUNT);
	while (mutex_is_locked(&_serial[iface]->rx.mutex));
}

void uartd_wait_tx_transfer(const uint8_t iface)
{
	assert(iface < UART_IFACE_COUNT);
	while (mutex_is_locked(&_serial[iface]->tx.mutex));
}
