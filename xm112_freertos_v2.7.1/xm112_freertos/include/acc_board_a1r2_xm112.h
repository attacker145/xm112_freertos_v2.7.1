// Copyright (c) Acconeer AB, 2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#ifndef ACC_BOARD_A1R2_XM112_H_
#define ACC_BOARD_A1R2_XM112_H_

#include "chip.h"
#include <stdbool.h>
#include <stdint.h>

#include "acc_device.h"

typedef struct
{
	bool     open;
	uint32_t baudrate;
	bool     use_as_debug;
} acc_board_xm112_uart_config_t;

typedef struct
{
	acc_board_xm112_uart_config_t uart_config[UART_IFACE_COUNT];
} acc_board_xm112_config_t;

typedef void (*acc_board_get_config_t)(acc_board_xm112_config_t *config);

extern acc_board_get_config_t acc_board_get_config;


uint32_t acc_board_set_sensor_transfer_speed(uint_fast32_t speed);


void acc_board_set_sensor_transfer_default_speed(void);


acc_device_handle_t acc_board_get_spi_slave_handle(void);


acc_device_handle_t acc_board_get_i2c_slave_handle(void);


#endif
