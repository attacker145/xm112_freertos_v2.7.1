// Copyright (c) Acconeer AB, 2017-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#ifndef ACC_DRIVER_SPI_SAME70_H_
#define ACC_DRIVER_SPI_SAME70_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pio.h"

typedef struct
{
	struct _pin spi_miso;
	struct _pin spi_mosi;
	struct _pin spi_clk;
	struct _pin spi_npcs;
} acc_driver_spi_same70_config_t;

/**
 * @brief Function called by driver to wait for transfer complete.
 *
 * Typically this function waits for a semaphore that is signaled when the
 * transfer_complete_callback_t is called.
 *
 * @param[in] dev_handle The device handle
 */
typedef void (*wait_for_transfer_complete_t)(acc_device_handle_t dev_handle);

/**
 * @brief Function called by driver when transfer is complete
 *
 * Typically this function signals the semaphore that wait_for_transfer_complete_t
 * is waiting for.
 *
 * @param[in] dev_handle The device handle
 */
typedef void (*transfer_complete_callback_t)(acc_device_handle_t dev_handle);

/**
 * @brief Request driver to register with appropriate device(s)
 */
extern void acc_driver_spi_same70_register(wait_for_transfer_complete_t wait_function,
                                           transfer_complete_callback_t transfer_complete);


#ifdef __cplusplus
}
#endif

#endif
