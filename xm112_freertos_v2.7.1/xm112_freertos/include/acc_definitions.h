// Copyright (c) Acconeer AB, 2018-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#ifndef ACC_DEFINITIONS_H_
#define ACC_DEFINITIONS_H_

#include <inttypes.h>
#include <stdint.h>


/**
 * @brief Type representing a sensor ID
 */
typedef uint32_t acc_sensor_id_t;

/**
 * @brief Macro for printing sensor id
 */
#define PRIsensor_id PRIu32


/**
 * @brief Power save mode
 *
 * Each power save mode corresponds to how much of the sensor hardware is shutdown
 * between data frame aquisition. Mode 'OFF' means that the whole sensor is shutdown.
 * Mode 'ACTIVE' means that the sensor is in its active state all the time.
 * Mode 'HIBERNATE' means that the sensor is still powered but the internal oscillator is
 * turned off and the application needs to clock the sensor by toggling a GPIO a pre-defined
 * number of times to enter and exit this mode. Mode 'HIBERNATE' is currently only supported
 * by the Sparse service
 *
 * For each power save mode there will be a limit in the achievable update rate. Mode 'OFF'
 * can achieve the lowest power consumption but also has the lowest update rate limit
 */
typedef enum
{
	ACC_POWER_SAVE_MODE_OFF,
	ACC_POWER_SAVE_MODE_SLEEP,
	ACC_POWER_SAVE_MODE_READY,
	ACC_POWER_SAVE_MODE_ACTIVE,
	ACC_POWER_SAVE_MODE_HIBERNATE,
} acc_power_save_mode_enum_t;
typedef uint32_t acc_power_save_mode_t;


/**
 * @brief Service profile
 *
 * Each profile consists of a number of settings for the sensor that configures
 * the RX and TX paths. Profile 1 maximizes on the depth resolution and
 * profile 5 maximizes on radar loop gain with a sliding scale in between.
 */
typedef enum
{
	ACC_SERVICE_PROFILE_1 = 1,
	ACC_SERVICE_PROFILE_2,
	ACC_SERVICE_PROFILE_3,
	ACC_SERVICE_PROFILE_4,
	ACC_SERVICE_PROFILE_5,
} acc_service_profile_t;


/**
 * @brief Data type for interger-based representation of complex numbers
 */
typedef struct
{
	int16_t real;
	int16_t imag;
} acc_int16_complex_t;


/**
 * Type used when retrieving and setting a sensor calibration context
 */
typedef struct
{
	uint8_t data[64];
} acc_calibration_context_t;


#endif
