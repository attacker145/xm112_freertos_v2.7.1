// Copyright (c) Acconeer AB, 2019-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#ifndef ACC_RF_CERTIFICATION_TEST_H_
#define ACC_RF_CERTIFICATION_TEST_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Execute the RF certification test
 *
 * @param tx_disable Set to true to disable TX, set to false to enable TX
 * @param iterations The number of iterations, set to 0 to run infinite number of iterations.
 *
 * @return True if executed successfully, false if not
 */
bool acc_rf_certification_test(bool tx_disable, uint32_t iterations);


#endif
