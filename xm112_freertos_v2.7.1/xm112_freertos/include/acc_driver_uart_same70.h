// Copyright (c) Acconeer AB, 2017-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#ifndef ACC_DRIVER_UART_SAME70_H_
#define ACC_DRIVER_UART_SAME70_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Function called by driver to wait for transfer complete.
 *
 * Typically this function waits for a semaphore that is signaled when the
 * transfer_complete_callback_t is called.
 *
 * @param[in] port The port
 */
typedef void (*uart_wait_for_transfer_complete_t)(uint_fast8_t port);

/**
 * @brief Function called by driver when transfer is complete
 *
 * Typically this function signals the semaphore that wait_for_transfer_complete_t
 * is waiting for.
 *
 * @param[in] port The port
 */
typedef void (*uart_transfer_complete_callback_t)(uint_fast8_t port);

/**
 * @brief Request driver to register with appropriate device(s)
 *
 * @param[in] wait_function     Called by driver to wait for transfer complete. See above
 *                              for more information.
 * @param[in] transfer_complete Called by driver when transfer is completed. See above
 *                              for more information.
 */
void acc_driver_uart_same70_register(uart_wait_for_transfer_complete_t wait_function,
                                     uart_transfer_complete_callback_t transfer_complete);


#ifdef __cplusplus
}
#endif

#endif
