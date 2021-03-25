// Copyright (c) Acconeer AB, 2018
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#ifndef _BOARD_A111_H_
#define _BOARD_A111_H_

#include <board.h>
#include "a111/a111.h"

/**
 * Configure the A111 driver
 */
void board_cfg_a111(void);

/**
 * Get the A111 instance
 */
struct _a111 * board_get_a111(void);

#endif //_BOARD_A111_H_
