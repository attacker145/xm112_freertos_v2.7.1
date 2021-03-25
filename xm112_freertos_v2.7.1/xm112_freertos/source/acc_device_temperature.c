// Copyright (c) Acconeer AB, 2017-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "acc_device_os.h"
#include "acc_device_temperature.h"
#include "acc_log.h"

#define MODULE "device_temperature"  /**< Module name */

bool (*acc_device_temperature_init_func)(void) = NULL;
bool (*acc_device_temperature_read_func)(acc_device_temperature_id_t id, float *value) = NULL;


/**
 * @brief Flag to indicate if device has been initialized
 */
static bool init_done = false;


bool acc_device_temperature_init(void)
{
	static acc_app_integration_mutex_t	init_mutex = NULL;

	if (init_done) {
		return true;
	}

	acc_os_init();
	init_mutex = acc_os_mutex_create();

	acc_os_mutex_lock(init_mutex);
	if (init_done) {
		acc_os_mutex_unlock(init_mutex);
		return true;
	}

	if (acc_device_temperature_init_func != NULL) {
		if (!acc_device_temperature_init_func()) {
			acc_os_mutex_unlock(init_mutex);
			return false;
		}
	}

	init_done = true;
	acc_os_mutex_unlock(init_mutex);

	return true;
}


bool acc_device_temperature_read(acc_device_temperature_id_t id, float *value)
{
	bool status;

	if (!acc_device_temperature_init()) {
		ACC_LOG_ERROR("acc_device_temperature_init() failed.");
		return false;
	}

	if (!init_done) {
		return false;
	}

	if (acc_device_temperature_read_func != NULL) {
		status = acc_device_temperature_read_func(id, value);
	} else {
		status = false;
	}

	return status;
}
