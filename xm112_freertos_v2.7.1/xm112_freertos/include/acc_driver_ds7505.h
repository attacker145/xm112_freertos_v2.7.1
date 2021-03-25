// Copyright (c) Acconeer AB, 2016-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#ifndef ACC_DRIVER_DS7505_H_
#define ACC_DRIVER_DS7505_H_

#include <stdint.h>

#include "acc_device.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Request driver to register with device(s)
 *
 * @param[in] i2c_device_handle I2C device handle
 * @param[in] i2c_device_id I2C device ID of the temperature sensor
 */
extern void acc_driver_ds7505_register(acc_device_handle_t i2c_device_handle, uint8_t i2c_device_id);

#ifdef __cplusplus
}
#endif

#endif
