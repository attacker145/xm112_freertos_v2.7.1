// Copyright (c) Acconeer AB, 2018-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#ifndef ACC_DRIVER_HAL_H_
#define ACC_DRIVER_HAL_H_

#include "acc_definitions.h"
#include "acc_hal_definitions.h"


/**
 * @brief Initialize hal driver
 *
 * @return True if initialization is successful
 */
bool acc_driver_hal_init(void);


/**
 * @brief Get hal implementation reference
 */
const acc_hal_t *acc_driver_hal_get_implementation(void);


#endif
