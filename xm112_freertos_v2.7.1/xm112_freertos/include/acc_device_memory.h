// Copyright (c) Acconeer AB, 2017-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#ifndef ACC_DEVICE_MEMORY_H_
#define ACC_DEVICE_MEMORY_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


// These functions are to be used by drivers only, do not use them directly
extern bool (*acc_device_memory_init_func)(void);
extern bool (*acc_device_memory_get_size_func)(size_t *memory_size);
extern bool (*acc_device_memory_write_func)(uint32_t address, const void *buffer, size_t size);
extern bool (*acc_device_memory_read_func)(uint32_t address, void *buffer, size_t size);


/**
 * @brief Initializes a non-volatile memory device.
 *
 * @return True if successful, false otherwise
 */
extern bool acc_device_memory_init(void);


/**
 * @brief Returns the size of the unerlying memory
 *
 * @param[out] memory_size Argument to receive maximum size of underlying memory
 * @return True if successful, false otherwise
 */
extern bool acc_device_memory_get_size(size_t *memory_size);


/**
 * @brief Writes data to a non-volatile memory.
 *
 * The function acc_device_memory_write writes size bytes of data, from buffer,
 * to a non-volatile memory starting at address. The device has to be
 * initialized before this function is called.
 *
 * If size equals 0, false is returned.
 *
 * @param[in] address The memory address to start write at
 * @param[out] buffer The buffer containing the data to be written
 * @param[in] size The size in bytes of the data to be written
 * @return True if successful, false otherwise
 */
extern bool acc_device_memory_write(uint32_t address, const void *buffer, size_t size);


/**
 * @brief Reads data from a non-volatile memory.
 *
 * The function acc_device_memory_read reads size bytes of data, from a
 * non-volatile memory starting at address, to buffer. The device has
 * to be initialized before this function is called.
 *
 * If size equals 0, false is returned
 *
 * @param[in] address The memory address to start read at
 * @param[out] buffer The buffer where the data is stored
 * @param[in] size The size in bytes of the data to be read
 * @return True if successful, false otherwise
 */
extern bool acc_device_memory_read(uint32_t address, void *buffer, size_t size);

#endif
