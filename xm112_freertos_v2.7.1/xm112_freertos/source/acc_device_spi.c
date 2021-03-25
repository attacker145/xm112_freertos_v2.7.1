// Copyright (c) Acconeer AB, 2016-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "acc_device.h"
#include "acc_device_os.h"
#include "acc_device_spi.h"

/**
 * @brief The module name
 */
#define MODULE		"device_spi"


size_t              (*acc_device_spi_get_max_transfer_size_func)(void) = NULL;
acc_device_handle_t (*acc_device_spi_create_func)(acc_device_spi_configuration_t *configuration) = NULL;
void                (*acc_device_spi_destroy_func)(acc_device_handle_t *handle) = NULL;
bool                (*acc_device_spi_transfer_func)(acc_device_handle_t handle, uint8_t *buffer, size_t buffer_size) = NULL;
bool		(*acc_device_spi_transfer_async_func)(acc_device_handle_t handle, uint8_t *buffer, bool rx, bool tx, size_t buffer_size, acc_device_spi_transfer_callback_t callback) = NULL;
uint8_t	            (*acc_device_spi_get_bus_func)(acc_device_handle_t) = NULL;


/**
 * @brief Mutex to protect SPI transfers
 */
static acc_app_integration_mutex_t spi_mutex[ACC_DEVICE_SPI_BUS_MAX] = {NULL};


acc_device_handle_t acc_device_spi_create(acc_device_spi_configuration_t *configuration)
{
	if (acc_device_spi_create_func != NULL) {
		if (spi_mutex[0] == NULL) {
			for (uint_fast8_t index = 0; index < ACC_DEVICE_SPI_BUS_MAX; index++)
			{
				spi_mutex[index] = acc_os_mutex_create();
			}
		}

		return (acc_device_spi_create_func(configuration));
	}

	return NULL;
}


void acc_device_spi_destroy(acc_device_handle_t *handle)
{
	if (acc_device_spi_destroy_func != NULL) {
		acc_device_spi_destroy_func(handle);
	}
}


uint_fast8_t acc_device_spi_get_bus(acc_device_handle_t handle)
{
	if (acc_device_spi_get_bus_func != NULL) {
		return acc_device_spi_get_bus_func(handle);
	}

	printf("Default SPI bus returned\n");

	return 0;
}


bool acc_device_spi_lock(uint_fast8_t bus)
{
	if (bus >= ACC_DEVICE_SPI_BUS_MAX) {
		return false;
	}

	acc_os_mutex_lock(spi_mutex[bus]);

	return true;
}


bool acc_device_spi_unlock(uint_fast8_t bus)
{
	if (bus >= ACC_DEVICE_SPI_BUS_MAX) {
		return false;
	}

	acc_os_mutex_unlock(spi_mutex[bus]);

	return true;
}


size_t acc_device_spi_get_max_transfer_size(void)
{
	if (acc_device_spi_get_max_transfer_size_func) {
		return acc_device_spi_get_max_transfer_size_func();
	}

	return SIZE_MAX;
}


bool acc_device_spi_transfer(acc_device_handle_t handle, uint8_t *buffer, size_t buffer_size)
{
	bool status = false;
	if (acc_device_spi_transfer_func != NULL) {
		status = acc_device_spi_transfer_func(handle, buffer, buffer_size);
	}

	if (!status) {
		printf("%s failed\n", __func__);
	}

	return status;
}


bool acc_device_spi_transfer_async(acc_device_handle_t handle, uint8_t *buffer, bool rx, bool tx, size_t buffer_size, acc_device_spi_transfer_callback_t callback)
{
	bool status = false;
	if (acc_device_spi_transfer_async_func != NULL) {
		status = acc_device_spi_transfer_async_func(handle, buffer, rx, tx, buffer_size, callback);
	}

	if (!status) {
		printf("%s failed\n", __func__);
	}

	return status;
}
