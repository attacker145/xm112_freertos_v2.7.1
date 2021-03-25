// Copyright (c) Acconeer AB, 2017-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#ifndef ACC_DEVICE_TEMPERATURE_H_
#define ACC_DEVICE_TEMPERATURE_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum {
	ACC_DEVICE_TEMPERATURE_ID_BOARD,
	ACC_DEVICE_TEMPERATURE_ID_MAX
} acc_device_temperature_id_enum_t;
typedef uint32_t acc_device_temperature_id_t;

// These functions are to be used by drivers only, do not use them directly
extern bool (*acc_device_temperature_init_func)(void);
extern bool (*acc_device_temperature_read_func)(acc_device_temperature_id_t id, float *value);


/**
 * @brief Initializes the temperature device.
 *
 * @return True if successful, false otherwise
 */
extern bool acc_device_temperature_init(void);


/**
 * @brief Reads temperature value.
 *
 * Read the temperature from a specified temperature sensor given by ID.
 *
 * @param[in] id The temperature sensor ID to read
 * @param[out] value temperature value is returned
 * @return True if successful, false otherwise
 */
extern bool acc_device_temperature_read(acc_device_temperature_id_t id, float *value);

#endif
