/* ----------------------------------------------------------------------------
 *         SAM Software Package License
 * ----------------------------------------------------------------------------
 * Copyright (c) 2015, Atmel Corporation
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

/**
 * \file
 *
 * \par Purpose
 *
 * This module provides several definitions and methods for using an UART
 * peripheral.
 *
 */

#ifndef UART_H
#define UART_H

#ifdef CONFIG_HAVE_UART
/*------------------------------------------------------------------------------
 *         Headers
 *------------------------------------------------------------------------------*/
#include "chip.h"

#include <stdbool.h>
#include <stdint.h>
/*----------------------------------------------------------------------------
 *        Macros
 *----------------------------------------------------------------------------*/

/* Returns 1 if the TXRDY bit (ready to transmit data) is set in the given status register value.*/
#define UART_STATUS_TXRDY(status) ((status & UART_SR_TXRDY) == UART_SR_TXRDY)

/* Returns 1 if the RXRDY bit (ready to receive data) is set in the given status register value.*/
#define UART_STATUS_RXRDY(status) ((status & UART_SR_RXRDY) == UART_SR_RXRDY)

/* Returns 1 if the TXEMPTY bit (end of transmit) is set in the given status register value.*/
#define UART_STATUS_TXEMPTY(status) ((status & UART_SR_TXEMPTY) == UART_SR_TXEMPTY)

/*------------------------------------------------------------------------------*/
/*         Exported functions                                                   */
/*------------------------------------------------------------------------------*/

extern void uart_configure(Uart* uart, uint32_t mode, uint32_t baudrate);
extern void uart_set_transmitter_enabled(Uart* uart, bool enabled);
extern void uart_set_receiver_enabled (Uart* uart, bool enabled);
extern void uart_enable_it(Uart* uart, uint32_t mask);
extern void uart_disable_it(Uart* uart, uint32_t mask);
extern bool uart_is_tx_ready(Uart* uart);
extern bool uart_is_tx_empty(Uart* uart);
extern void uart_put_char(Uart* uart, uint8_t c);
extern bool uart_is_rx_ready(Uart* uart);
extern uint8_t uart_get_char(Uart* uart);
extern uint32_t uart_get_status(Uart *uart);
extern uint32_t uart_get_masked_status(Uart *uart);

#endif /* CONFIG_HAVE_UART */

#endif /* UART_H */

