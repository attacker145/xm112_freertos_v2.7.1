// Copyright (c) Acconeer AB, 2018
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#ifndef _A111_H
#define _A111_H

#include <stdint.h>
#include <stdbool.h>

#include "gpio/pio.h"
#include "peripherals/bus.h"

struct _a111_config {
	const struct _bus_dev_cfg spi;
	const struct _pin sens_en;
	const struct _pin ps_en;
	const struct _pin sens_int;
};

struct _a111 {
	struct _bus_dev_cfg spi;
	struct _pin sens_en;
	struct _pin ps_en;
	struct _pin sens_int;
};

/**
 * Configure the a111 sensor driver
 */
int a111_configure(struct _a111* a111, const struct _a111_config* cfg);

/**
 * Set PS_ENABLE pin which controls the 1V8IO to the A111.
 */
void a111_set_ps_enable(struct _a111* a111);

/**
 * Clear PS_ENABLE pin which controls the 1V8IO to the A111.
 */
void a111_clear_ps_enable(struct _a111* a111);

/**
 * Set SENSE_EN pin on the A111
 */
void a111_set_sense_en(struct _a111* a111);

/**
 * Clear SENSE_EN pin on the A111
 */
void a111_clear_sense_en(struct _a111* a111);

/**
 * Returns true if the INTERRUPT pin at the A111 is active
 */
bool a111_is_interrupt_active(struct _a111* a111);

/**
 * Adds an interrupt handler to the INTERRUPT pin of A111
 */
void a111_add_interrupt_handler(struct _a111* a111, pio_handler_t handler, void* user_arg);

/**
 * Transfer SPI data to and from the A111
 */
int a111_spi_transfer(struct _a111* a111, uint8_t *data, uint32_t size, struct _callback* cb);

#endif // _A111_H_
