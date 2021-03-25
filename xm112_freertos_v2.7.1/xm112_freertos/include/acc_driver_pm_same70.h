// Copyright (c) Acconeer AB, 2018-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#ifndef ACC_DRIVER_PM_SAME70_H_
#define ACC_DRIVER_PM_SAME70_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Request driver to register with appropriate device(s)
 */
extern void acc_driver_pm_same70_register(uint8_t req_wkup_gpio);

#ifdef __cplusplus
}
#endif

#endif
