/* ----------------------------------------------------------------------------
 *         SAM Software Package License
 * ----------------------------------------------------------------------------
 * Copyright (c) 2015, Atmel Corporation
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

#ifndef UARTD_H_
#define UARTD_H_

#ifdef CONFIG_HAVE_UART

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "callback.h"
#include "dma/dma.h"
#include "io.h"
#include "mutex.h"
#include "serial/uart.h"

/*----------------------------------------------------------------------------
 *        Constants
 *----------------------------------------------------------------------------*/

#define UARTD_SUCCESS         (0)
#define UARTD_INVALID_ID      (1)
#define UARTD_INVALID_BITRATE (2)
#define UARTD_ERROR_LOCK      (3)
#define UARTD_ERROR_DUPLEX    (4)
#define UARTD_ERROR_TIMEOUT   (5)

/*----------------------------------------------------------------------------
 *        Type definitions
 *----------------------------------------------------------------------------*/

enum _uartd_buf_attr {
	UARTD_BUF_ATTR_WRITE = 0x01,
	UARTD_BUF_ATTR_READ  = 0x02,
};

struct _uart_desc
{
	Uart* addr;
	uint32_t mode;
	uint32_t baudrate;
	uint8_t transfer_mode;
	struct _callback error_callback;

	/* implicit internal padding is mandatory here */
	struct {
		mutex_t mutex;

		struct _buffer buffer;
		uint16_t transferred;

		struct _callback callback;
	} rx, tx;

	struct {
		struct {
			struct _dma_channel *channel;
			struct _dma_cfg cfg_dma;
			struct _dma_transfer_cfg cfg;
		} rx;
		struct {
			struct _dma_channel *channel;
			struct _dma_cfg cfg_dma;
		} tx;
	} dma;
};

enum _uartd_trans_mode
{
	UARTD_MODE_POLLING,
	UARTD_MODE_ASYNC,
	UARTD_MODE_DMA,
};

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/

extern void uartd_configure(uint8_t iface, struct _uart_desc* desc);
extern uint32_t uartd_transfer(uint8_t iface, struct _buffer* buf, struct _callback* cb);
extern void uartd_finish_rx_transfer(uint8_t iface);
extern uint32_t uartd_rx_is_busy(const uint8_t iface);
extern void uartd_wait_rx_transfer(const uint8_t iface);
extern void uartd_finish_tx_transfer(uint8_t iface);
extern uint32_t uartd_tx_is_busy(const uint8_t iface);
extern void uartd_wait_tx_transfer(const uint8_t iface);

#endif /* CONFIG_HAVE_UART */

#endif /* UARTD_H_ */
