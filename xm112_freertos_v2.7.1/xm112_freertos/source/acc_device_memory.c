// Copyright (c) Acconeer AB, 2017-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "acc_device_memory.h"

#include "acc_device_os.h"


#define MODULE "device_memory"	/**< Module name */

bool (*acc_device_memory_init_func)(void) = NULL;
bool (*acc_device_memory_get_size_func)(size_t *memory_size) = NULL;
bool (*acc_device_memory_read_func)(uint32_t address, void *buffer, size_t size) = NULL;
bool (*acc_device_memory_write_func)(uint32_t address, const void *buffer, size_t size) = NULL;

static acc_app_integration_mutex_t	memory_mutex = NULL;	/**< Mutex to protect memory transfers */
static bool				init_done = false;	/**< Flag to indicate if device has been initialized */
static size_t				memory_size;		/**< Size of the currently registered memory in bytes, or 0 if unknown */


bool acc_device_memory_init(void)
{
	static acc_app_integration_mutex_t	init_mutex = NULL;

	if (init_done) {
		return true;
	}

	acc_os_init();
	init_mutex = acc_os_mutex_create();
	memory_mutex = acc_os_mutex_create();

	acc_os_mutex_lock(init_mutex);
	if (init_done) {
		acc_os_mutex_unlock(init_mutex);
		return true;
	}

	if (acc_device_memory_init_func != NULL) {
		if (!acc_device_memory_init_func()) {
			acc_os_mutex_unlock(init_mutex);
			return false;
		}
	}

	if (!acc_device_memory_get_size(&memory_size)) {
		memory_size = 0;
	}

	init_done = true;

	acc_os_mutex_unlock(init_mutex);

	return true;
}


bool acc_device_memory_get_size(size_t *memory_size)
{
	if (!init_done) {
		return false;
	}

	if (memory_size == NULL) {
		return false;
	}

	if (acc_device_memory_get_size_func == NULL) {
		return false;
	}

	return acc_device_memory_get_size_func(memory_size);
}


bool acc_device_memory_read(uint32_t address, void *buffer, size_t size)
{
	bool status;

	if (!init_done) {
		return false;
	}

	if (size == 0) {
		return false;
	}

	if ((memory_size != 0) && (address + size > memory_size)) {
		return false;
	}

	acc_os_mutex_lock(memory_mutex);

	if (acc_device_memory_read_func != NULL) {
		status = acc_device_memory_read_func(address, buffer, size);
	}
	else {
		status = false;
	}

	acc_os_mutex_unlock(memory_mutex);

	return status;
}


bool acc_device_memory_write(uint32_t address, const void *buffer, size_t size)
{
	bool status;

	if (!init_done) {
		return false;
	}

	if (size == 0) {
		return false;
	}

	if ((memory_size != 0) && (address + size > memory_size)) {
		return false;
	}

	acc_os_mutex_lock(memory_mutex);

	if (acc_device_memory_write_func != NULL) {
		status = acc_device_memory_write_func(address, buffer, size);
	}
	else {
		status = false;
	}

	acc_os_mutex_unlock(memory_mutex);

	return status;
}
