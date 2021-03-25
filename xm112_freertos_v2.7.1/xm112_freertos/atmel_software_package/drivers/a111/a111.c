// Copyright (c) Acconeer AB, 2018
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#include "a111/a111.h"

/**
 * Configure the a111 sensor driver
 */
int a111_configure(struct _a111* a111, const struct _a111_config* cfg)
{
	a111->ps_en = cfg->ps_en;
	a111->sens_en = cfg->sens_en;
	a111->sens_int = cfg->sens_int;
	a111->spi = cfg->spi;

	pio_configure(&a111->ps_en, 1);
	pio_configure(&a111->sens_en, 1);
	pio_configure(&a111->sens_int, 1);

	int res = bus_configure_slave(a111->spi.bus, &a111->spi);
	return res;
}

void a111_set_ps_enable(struct _a111* a111)
{
	pio_set(&a111->ps_en);
}

void a111_clear_ps_enable(struct _a111* a111)
{
	pio_clear(&a111->ps_en);
}

void a111_set_sense_en(struct _a111* a111)
{
	pio_set(&a111->sens_en);
}

void a111_clear_sense_en(struct _a111* a111)
{
	pio_clear(&a111->sens_en);
}

bool a111_is_interrupt_active(struct _a111* a111)
{
	return pio_get(&a111->sens_int) ? true : false;
}

void a111_add_interrupt_handler(struct _a111* a111, pio_handler_t handler, void* user_arg)
{
	pio_enable_it(&a111->sens_int);
	pio_add_handler_to_group(a111->sens_int.group, a111->sens_int.mask, handler, user_arg);
}

int a111_spi_transfer(struct _a111* a111, uint8_t *data, uint32_t size, struct _callback* cb)
{
	struct _buffer a111_buf = {
		.data = data,
		.size = size,
		.attr = BUS_BUF_ATTR_RX | BUS_BUF_ATTR_TX | BUS_SPI_BUF_ATTR_RELEASE_CS,
	};
	int res = bus_start_transaction(a111->spi.bus);
	if (!res) {
		res = bus_transfer(a111->spi.bus, a111->spi.spi_dev.chip_select, &a111_buf, 1, cb);
		if (!res) {
			res = bus_wait_transfer(a111->spi.bus);
			bus_stop_transaction(a111->spi.bus);
		}
	}
	return res;
}
