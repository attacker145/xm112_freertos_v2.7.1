// Copyright (c) Acconeer AB, 2019-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#ifndef ACC_LOG_INTEGRATION_H_
#define ACC_LOG_INTEGRATION_H_

#include "acc_hal_definitions.h"


#if defined(__GNUC__)
#define PRINTF_ATTRIBUTE_CHECK(a, b) __attribute__((format(printf, a, b)))
#else
#define PRINTF_ATTRIBUTE_CHECK(a, b)
#endif


void acc_log(acc_log_level_t level, const char *module, const char *format, ...) PRINTF_ATTRIBUTE_CHECK(3, 4);


#endif
