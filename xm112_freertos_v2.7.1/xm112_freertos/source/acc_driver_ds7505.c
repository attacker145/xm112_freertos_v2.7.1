// Copyright (c) Acconeer AB, 2016-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#include <stdbool.h>
#include <stdint.h>

#include "acc_driver_ds7505.h"

#include "acc_device.h"
#include "acc_device_i2c.h"
#include "acc_device_temperature.h"


#define MODULE "driver_ds7505"  /**< module name */

#define DS7505_REG_TEMPERATURE    0x00  /**< temperature reading */
#define DS7505_REG_CONFIGURATION  0x01  /**< device configuration register */
#define DS7505_REG_T_HYST         0x02  /**< set temperature hysteresis */
#define DS7505_REG_T_OS           0x03  /**< set O.S. output temperature threshold */
#define DS7505_CMD_RECALL_DATA    0xb8  /**< load internal EEPROM data into SRAM shadow registers */
#define DS7505_CMD_COPY_DATA      0x48  /**< copy from SRAM shadow registers into internal EEPROM */
#define DS7505_CMD_SOFTWARE_RESET 0x54  /**< perform a software power on reset */


typedef struct
{
	acc_device_handle_t  i2c_device_handle;
	uint8_t              i2c_device_id;
} driver_context_t;


static driver_context_t driver_context;


//-----------------------------
// Private declarations
//-----------------------------

/**
 * @brief Initialize the DS7505 driver
 *
 * @return True if successful, false otherwise
 */
static bool acc_driver_ds7505_init(void);


/**
 * @brief Read a temperature
 *
 * @param[in] id The temperature sensor id to read
 * @param[out] value Value to receive temperature reading
 * @return True if successful, false otherwise
 */
static bool acc_driver_ds7505_read(acc_device_temperature_id_t id, float *value);


//-----------------------------
// Public definitions
//-----------------------------

void acc_driver_ds7505_register(acc_device_handle_t i2c_device_handle, uint8_t i2c_device_id)
{
	driver_context.i2c_device_handle  = i2c_device_handle;
	driver_context.i2c_device_id      = i2c_device_id;

	acc_device_temperature_init_func  = acc_driver_ds7505_init;
	acc_device_temperature_read_func  = acc_driver_ds7505_read;
}


//-----------------------------
// Private definitions
//-----------------------------

bool acc_driver_ds7505_init(void)
{
	static uint8_t		i2c_buffer;

	// errors are ignored since the DS7505 will not ACK the reset command
	acc_device_i2c_write_to_address_8(driver_context.i2c_device_handle, driver_context.i2c_device_id, DS7505_CMD_SOFTWARE_RESET, NULL, 0);

	i2c_buffer = 0x60;  /* R1..R0 = 12 bit resolution, TM = comparator mode, SD = active conversion and thermostat operation */

	if (!acc_device_i2c_write_to_address_8(driver_context.i2c_device_handle, driver_context.i2c_device_id, DS7505_REG_CONFIGURATION, &i2c_buffer, 1))
	{
		return false;
	}

	return true;
}


bool acc_driver_ds7505_read(acc_device_temperature_id_t id, float *value)
{
	bool		status = false;
	uint8_t		i2c_buffer[2];

	switch (id)
	{
		case ACC_DEVICE_TEMPERATURE_ID_BOARD :
			if (!acc_device_i2c_read_from_address_8(driver_context.i2c_device_handle, driver_context.i2c_device_id, DS7505_REG_TEMPERATURE, i2c_buffer, 2))
			{
				break;
			}

			uint16_t temp = ((uint16_t)i2c_buffer[0] << 8) + i2c_buffer[1];
			*value = (float)*(int16_t*)&temp / 256.0f;
			status = true;
			break;

		default :
			status = false;
	}

	return status;
}
