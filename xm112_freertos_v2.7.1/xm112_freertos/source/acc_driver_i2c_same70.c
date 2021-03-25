// Copyright (c) Acconeer AB, 2018-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "acc_device_i2c.h"
#include "acc_driver_i2c.h"
#include "acc_driver_i2c_same70.h"

#include "bus.h"
#include "pio.h"
#include "twid.h"

/**
 * @brief I2C driver for same70q21
 */

/**
 * @brief The module name
 */
#define MODULE "driver_i2c_same70"


#define I2C_PERIPHERAL_COUNT   (3)
#define I2C_TIMEOUT            (15000u)
#define I2C_FAST_MODE_SPEED    (400000u)

typedef struct
{
	struct _pin pins[2];
} i2c_peripheral_t;


static const i2c_peripheral_t i2c_peripheral[I2C_PERIPHERAL_COUNT] =
{
	{
		.pins = PINS_TWI0,
	},
	{
		.pins = PINS_TWI1,
	},
	{
		.pins = PINS_TWI2,
	}
};

typedef struct
{
	const struct _pin                   *pins;
	bool                                peripheral_enabled;
	acc_device_i2c_slave_isr_callback_t slave_access_isr;
	struct _twi_slave_desc              slave_desc;
	struct _twi_desc                    master_desc;
	struct _twi_slave_ops               slave_ops;
} i2c_context_t;


//-----------------------------
// Private declarations
//-----------------------------
static acc_device_handle_t create(acc_device_i2c_configuration_t configuration);
static void destroy(acc_device_handle_t *handle);

static bool i2c_read_from_address_8(acc_device_handle_t device_handle, uint8_t slave_address, uint8_t address, uint8_t *buffer, size_t buffer_size);
static bool i2c_read_from_address_16(acc_device_handle_t device_handle, uint8_t slave_address, uint16_t address, uint8_t *buffer, size_t buffer_size);
static bool i2c_generic_read(acc_device_handle_t device_handle, uint8_t slave_address, uint32_t address, uint_fast8_t address_size, uint8_t *buffer , size_t buffer_size);

static bool i2c_write_to_address_8(acc_device_handle_t device_handle, uint8_t slave_address, uint8_t address, const uint8_t *buffer, size_t buffer_size);
static bool i2c_write_to_address_16(acc_device_handle_t device_handle, uint8_t slave_address, uint16_t address, const uint8_t *buffer, size_t buffer_size);
static bool i2c_generic_write(acc_device_handle_t device_handle, uint8_t slave_address, uint32_t address, uint_fast8_t address_size, const uint8_t *buffer, size_t buffer_size);

static void i2c_slave_access_isr_register(acc_device_handle_t device_handle, acc_device_i2c_slave_isr_callback_t *isr);

static void i2c_on_start(i2c_context_t *context);
static void i2c_on_stop(i2c_context_t *context);
static void i2c_on_write(i2c_context_t *context, uint8_t data);
static uint8_t i2c_on_read(i2c_context_t *context);

static void i2c_0_on_start(void);
static void i2c_0_on_stop(void);
static void i2c_0_on_write(uint8_t data);
static uint8_t i2c_0_on_read(void);
static void i2c_1_on_start(void);
static void i2c_1_on_stop(void);
static void i2c_1_on_write(uint8_t data);
static uint8_t i2c_1_on_read(void);
static void i2c_2_on_start(void);
static void i2c_2_on_stop(void);
static void i2c_2_on_write(uint8_t data);
static uint8_t i2c_2_on_read(void);


static i2c_context_t i2c_context[I2C_PERIPHERAL_COUNT] =
{
	{
		.peripheral_enabled = false,
		.slave_access_isr = {0,},
		.slave_desc.twi = TWI0,
		.slave_ops.on_start = i2c_0_on_start,
		.slave_ops.on_stop = i2c_0_on_stop,
		.slave_ops.on_read = i2c_0_on_read,
		.slave_ops.on_write = i2c_0_on_write,
		.master_desc.addr = TWI0,
		.master_desc.transfer_mode = BUS_TRANSFER_MODE_POLLING,
		.pins = i2c_peripheral[0].pins,
	},
	{
		.peripheral_enabled = false,
		.slave_access_isr = {0,},
		.slave_desc.twi = TWI1,
		.slave_ops.on_start = i2c_1_on_start,
		.slave_ops.on_stop = i2c_1_on_stop,
		.slave_ops.on_read = i2c_1_on_read,
		.slave_ops.on_write = i2c_1_on_write,
		.master_desc.addr = TWI1,
		.master_desc.transfer_mode = BUS_TRANSFER_MODE_POLLING,
		.pins = i2c_peripheral[1].pins,
	},
	{
		.peripheral_enabled = false,
		.slave_access_isr = {0,},
		.slave_desc.twi = TWI2,
		.slave_ops.on_start = i2c_2_on_start,
		.slave_ops.on_stop = i2c_2_on_stop,
		.slave_ops.on_read = i2c_2_on_read,
		.slave_ops.on_write = i2c_2_on_write,
		.master_desc.addr = TWI2,
		.master_desc.transfer_mode = BUS_TRANSFER_MODE_POLLING,
		.pins = i2c_peripheral[2].pins,
	}
};


//-----------------------------
// Public definitions
//-----------------------------

void acc_driver_i2c_same70_register(void)
{
	acc_device_i2c_create_func = create;
	acc_device_i2c_destroy_func = destroy;
	acc_device_i2c_write_to_address_8_func = i2c_write_to_address_8;
	acc_device_i2c_write_to_address_16_func = i2c_write_to_address_16;
	acc_device_i2c_read_from_address_8_func = i2c_read_from_address_8;
	acc_device_i2c_read_from_address_16_func = i2c_read_from_address_16;
	acc_device_i2c_read_func = NULL;
	acc_device_i2c_slave_access_isr_register_func = i2c_slave_access_isr_register;
}


acc_device_handle_t create(acc_device_i2c_configuration_t configuration)
{
	acc_device_handle_t handle = NULL;

	if (configuration.bus < I2C_PERIPHERAL_COUNT)
	{
		i2c_context_t *i2c = &i2c_context[configuration.bus];
		pio_configure(i2c->pins, 2);

		if (configuration.master)
		{
			i2c->master_desc.freq = configuration.mode.master.frequency;
			int err = twid_configure(&i2c->master_desc);

			if (err != 0)
			{
				return NULL;
			}
		}
		else
		{
			i2c->slave_desc.addr = configuration.mode.slave.address;
			int err = twid_slave_configure(&i2c->slave_desc, &i2c->slave_ops);

			if (err != 0)
			{
				printf("Failed to create I2C slave %d\n", err);

				return NULL;
			}
		}

		handle = (acc_device_handle_t)i2c;
	}

	return handle;
}


void destroy(acc_device_handle_t *handle)
{
	*handle = NULL;
}


bool i2c_read_from_address_8(acc_device_handle_t device_handle, uint8_t slave_address, uint8_t address, uint8_t *buffer, size_t buffer_size)
{
	return i2c_generic_read(device_handle, slave_address, address, 1, buffer, buffer_size);
}


bool i2c_read_from_address_16(acc_device_handle_t device_handle, uint8_t slave_address, uint16_t address, uint8_t *buffer, size_t buffer_size)
{
	return i2c_generic_read(device_handle, slave_address, address, 2, buffer, buffer_size);
}


bool i2c_generic_read(acc_device_handle_t device_handle,
                      uint8_t slave_address,
                      uint32_t address,
                      uint_fast8_t address_size,
                      uint8_t *buffer,
                      size_t buffer_size)
{
	i2c_context_t *i2c = device_handle;
	i2c->master_desc.slave_addr = slave_address;

	uint8_t addr_buf[2];
	struct _buffer buf[2] =
	{
		{
			.data = addr_buf,
			.size = address_size,
			.attr = BUS_I2C_BUF_ATTR_START | BUS_BUF_ATTR_TX,
		},
		{
			.data = buffer,
			.size = buffer_size,
			.attr = BUS_I2C_BUF_ATTR_START | BUS_BUF_ATTR_RX | BUS_I2C_BUF_ATTR_STOP,
		},
	};

	if (address_size == 1)
	{
		addr_buf[0] = address & 0xff;
	}
	else
	{
		addr_buf[0] = address >> 8;
		addr_buf[1] = address & 0xff;
	}

	int err = twid_transfer(&i2c->master_desc, (struct _buffer *)&buf, 2, NULL);

	if (err != 0)
	{
		printf("I2C read error %d\n", err);
	}

	return (err == 0);
}


bool i2c_write_to_address_8(acc_device_handle_t device_handle, uint8_t slave_address, uint8_t address, const uint8_t *buffer, size_t buffer_size)
{
	return i2c_generic_write(device_handle, slave_address, address, 1, buffer, buffer_size);
}


bool i2c_write_to_address_16(acc_device_handle_t device_handle, uint8_t slave_address, uint16_t address, const uint8_t *buffer, size_t buffer_size)
{
	return i2c_generic_write(device_handle, slave_address, address, 2, buffer, buffer_size);
}


bool i2c_generic_write(acc_device_handle_t device_handle,
                       uint8_t slave_address,
                       uint32_t address,
                       uint_fast8_t address_size,
                       const uint8_t *buffer,
                       size_t buffer_size)
{
	i2c_context_t *i2c = device_handle;
	i2c->master_desc.slave_addr = slave_address;

	uint8_t addr_buf[2];
	struct _buffer buf[2] =
	{
		{
			.data = addr_buf,
			.size = address_size,
			.attr = BUS_I2C_BUF_ATTR_START | BUS_BUF_ATTR_TX,
		},
		{
			.data = (uint8_t *)buffer,
			.size = buffer_size,
			.attr = BUS_BUF_ATTR_TX | BUS_I2C_BUF_ATTR_STOP,
		},
	};

	if (address_size == 1)
	{
		addr_buf[0] = address & 0xff;
	}
	else
	{
		addr_buf[0] = address >> 8;
		addr_buf[1] = address & 0xff;
	}

	int err = twid_transfer(&i2c->master_desc, (struct _buffer *)&buf, 2, NULL);

	if (err != 0)
	{
		printf("I2C write error %d\n", err);
	}

	return (err == 0);
}


void i2c_slave_access_isr_register(acc_device_handle_t device_handle, acc_device_i2c_slave_isr_callback_t *isr)
{
	i2c_context_t *i2c = device_handle;

	i2c->slave_access_isr = *isr;
}


void i2c_on_start(i2c_context_t *i2c)
{
	if (i2c->slave_access_isr.on_start != NULL)
	{
		i2c->slave_access_isr.on_start(i2c);
	}
}


void i2c_on_stop(i2c_context_t *i2c)
{
	if (i2c->slave_access_isr.on_stop != NULL)
	{
		i2c->slave_access_isr.on_stop(i2c);
	}
}


void i2c_on_write(i2c_context_t *i2c, uint8_t data)
{
	if (i2c->slave_access_isr.on_write != NULL)
	{
		i2c->slave_access_isr.on_write(i2c, data);
	}
}


uint8_t i2c_on_read(i2c_context_t *i2c)
{
	uint8_t data = 0xff;
	if (i2c->slave_access_isr.on_read != NULL)
	{
		data = i2c->slave_access_isr.on_read(i2c);
	}
	return data;
}


void i2c_0_on_start(void)
{
	i2c_on_start(&i2c_context[0]);
}


void i2c_0_on_stop(void)
{
	i2c_on_stop(&i2c_context[0]);
}


void i2c_0_on_write(uint8_t data)
{
	i2c_on_write(&i2c_context[0], data);
}


uint8_t i2c_0_on_read(void)
{
	return i2c_on_read(&i2c_context[0]);
}


void i2c_1_on_start(void)
{
	i2c_on_start(&i2c_context[1]);
}


void i2c_1_on_stop(void)
{
	i2c_on_stop(&i2c_context[1]);
}


void i2c_1_on_write(uint8_t data)
{
	i2c_on_write(&i2c_context[1], data);
}


uint8_t i2c_1_on_read(void)
{
	return i2c_on_read(&i2c_context[1]);
}


void i2c_2_on_start(void)
{
	i2c_on_start(&i2c_context[2]);
}


void i2c_2_on_stop(void)
{
	i2c_on_stop(&i2c_context[2]);
}


void i2c_2_on_write(uint8_t data)
{
	i2c_on_write(&i2c_context[2], data);
}


uint8_t i2c_2_on_read(void)
{
	return i2c_on_read(&i2c_context[2]);
}
