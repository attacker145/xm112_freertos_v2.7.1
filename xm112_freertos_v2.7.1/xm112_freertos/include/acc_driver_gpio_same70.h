// Copyright (c) Acconeer AB, 2017-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#ifndef ACC_DRIVER_GPIO_SAME70_H_
#define ACC_DRIVER_GPIO_SAME70_H_

#include <stdbool.h>
#include <stdint.h>

#include "acc_app_integration.h"
#include "acc_device_gpio.h"

#include "pio.h"

typedef enum
{
	GPIO_DIR_IN,
	GPIO_DIR_OUT,
	GPIO_DIR_UNKNOWN
} gpio_dir_enum_t;

typedef uint32_t gpio_dir_t;

/**
 * @brief GPIO pin information
 */
typedef struct
{
	uint8_t                     is_open;
	int8_t                      level;
	gpio_dir_t                  dir;
	struct _pin                 pin_struct;
	acc_app_integration_mutex_t mutex;
	acc_device_gpio_isr_t       isr;
	bool                        init;
} gpio_t;


/**
 * @brief Request driver to register with appropriate device(s)
 *
 * @param[in] pin_count The maximum number of pins supported
 * @param[in] gpio_mem Memory to be used be GPIO driver
 */
extern void acc_driver_gpio_same70_register(uint_fast16_t pin_count, gpio_t *gpio_mem);


#endif
