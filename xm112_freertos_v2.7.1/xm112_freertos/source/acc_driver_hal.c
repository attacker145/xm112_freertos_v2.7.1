// Copyright (c) Acconeer AB, 2018-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#include <stdbool.h>
#include <stdint.h>

#include "acc_driver_hal.h"

#include "acc_board.h"
#include "acc_definitions.h"
#include "acc_device_spi.h"
#include "acc_driver_os.h"
#include "acc_hal_definitions.h"
#include "acc_log_integration.h"


void (*acc_board_hibernate_enter_func)(acc_sensor_id_t sensor) = NULL;
void (*acc_board_hibernate_exit_func)(acc_sensor_id_t sensor) = NULL;


//-----------------------------
// Public definitions
//-----------------------------


bool acc_driver_hal_init(void)
{
	if (!acc_board_init())
	{
		return false;
	}

	if (!acc_board_gpio_init())
	{
		return false;
	}

	return true;
}


const acc_hal_t *acc_driver_hal_get_implementation(void)
{
	static acc_hal_t hal =
	{
		.properties.sensor_count          = 0,
		.properties.max_spi_transfer_size = 0,

		.sensor_device.power_on                = acc_board_start_sensor,
		.sensor_device.power_off               = acc_board_stop_sensor,
		.sensor_device.wait_for_interrupt      = acc_board_wait_for_sensor_interrupt,
		.sensor_device.transfer                = acc_board_sensor_transfer,
		.sensor_device.get_reference_frequency = acc_board_get_ref_freq,

		.sensor_device.hibernate_enter = NULL,
		.sensor_device.hibernate_exit  = NULL,

		.os.mem_alloc = NULL,
		.os.mem_free  = NULL,
		.os.gettime   = NULL,

		.log.log_level = ACC_LOG_LEVEL_INFO,
		.log.log       = acc_log
	};

	hal.properties.sensor_count          = acc_board_get_sensor_count();
	hal.properties.max_spi_transfer_size = acc_device_spi_get_max_transfer_size();
	hal.sensor_device.hibernate_enter    = acc_board_hibernate_enter_func;
	hal.sensor_device.hibernate_exit     = acc_board_hibernate_exit_func;

	hal.os.mem_alloc = acc_device_os_mem_alloc_func;
	hal.os.mem_free  = acc_device_os_mem_free_func;
	hal.os.gettime   = acc_device_os_get_time_func;

	return &hal;
}
