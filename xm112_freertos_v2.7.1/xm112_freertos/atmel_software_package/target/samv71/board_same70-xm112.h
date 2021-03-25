/* ----------------------------------------------------------------------------
 *         SAM Software Package License
 * ----------------------------------------------------------------------------
 * Copyright (c) 2017, Atmel Corporation
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

#ifndef _BOARD_SAME70_XM112_H
#define _BOARD_SAME70_XM112_H

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "chip.h"
#include "peripherals/bus.h"

#include "board_support.h"

/*----------------------------------------------------------------------------
 *        HW BOARD Definitions
 *----------------------------------------------------------------------------*/

/** Name of the board */
#define BOARD_NAME "same70-xm112"

/*----------------------------------------------------------------------------*/

/** Frequency of the board main clock oscillator (autodetect) */
#define BOARD_MAIN_CLOCK_EXT_OSC 0

/** \def Board System timer */
#define BOARD_TIMER_TC      TC1
#define BOARD_TIMER_CHANNEL 0

#define BOARD_CONSOLE_ADDR     UART0
#define BOARD_CONSOLE_BAUDRATE 115200
#define BOARD_CONSOLE_TX_PIN   PIN_UART0_TXD
#define BOARD_CONSOLE_RX_PIN   PIN_UART0_RXD

#define BOARD_UART0_ADDR       UART0
#define BOARD_UART0_PINS       PINS_UART0

#define BOARD_UART2_ADDR       UART2
#define BOARD_UART2_PINS       PINS_UART2

#define BOARD_UART4_ADDR       UART4
#define BOARD_UART4_PINS       PINS_UART4_ALT

#define LED_RED 0
#define PIN_LED_0 { PIO_GROUP_C, PIO_PC3, PIO_OUTPUT_1, PIO_OPENDRAIN }
#define PINS_LEDS { PIN_LED_0 }
#define NUM_LEDS 1

#define PIN_A111_SENS_EN { PIO_GROUP_D, PIO_PD10, PIO_OUTPUT_0, PIO_DEFAULT }
#define PIN_A111_PS_EN { PIO_GROUP_D, PIO_PD2, PIO_OUTPUT_0, PIO_DEFAULT }
#define PIN_A111_SENS_INT { PIO_GROUP_A, PIO_PA0, PIO_INPUT, PIO_IT_RISE_EDGE | PIO_PULLDOWN }

#define SPI_A111_ADDR SPI1
#define SPI_A111_PINS PINS_SPI1_NPCS0
#define SPI_A111_BUS BUS(BUS_TYPE_SPI, 1)
#define SPI_A111_CS 0
// Bit rate is in kHz
#define SPI_A111_BITRATE 50000

#define BOARD_SPI_BUS0       SPI0
#define BOARD_SPI_BUS0_PINS  PINS_SPI0_NPCS0
#define BOARD_SPI_BUS0_MODE  BUS_TRANSFER_MODE_DMA

#define BOARD_SPI_BUS1       SPI1
#define BOARD_SPI_BUS1_PINS  PINS_SPI1_NPCS0
#define BOARD_SPI_BUS1_MODE  BUS_TRANSFER_MODE_DMA

#define BOARD_TWI_BUS0      TWI2
#define BOARD_TWI_BUS0_FREQ 400000
#define BOARD_TWI_BUS0_MODE BUS_TRANSFER_MODE_DMA
#define BOARD_TWI_BUS0_PINS PINS_TWI2

#define BOARD_AT24_TWI_BUS BUS(BUS_TYPE_I2C, 0)
#define BOARD_AT24_ADDR    0x51
#define BOARD_AT24_MODEL   AT24C128 //AT24MAC402

#endif /* _BOARD_SAME70_XM112_H */
