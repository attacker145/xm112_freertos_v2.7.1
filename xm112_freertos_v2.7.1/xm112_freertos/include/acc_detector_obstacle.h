// Copyright (c) Acconeer AB, 2018-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#ifndef ACC_DETECTOR_OBSTACLE_H_
#define ACC_DETECTOR_OBSTACLE_H_

#include <complex.h>
#include <stdbool.h>
#include <stdint.h>

#include "acc_definitions.h"
#include "acc_detector_obstacle_processing.h"

/**
 * @defgroup Obstacle Obstacle Detector
 * @ingroup Detectors
 *
 * @brief Obstacle detector API description
 *
 * @{
 */


/**
 * @brief Metadata for each update provided by the obstacle detector
 */
typedef struct
{
	bool missed_data;                // Indication of missed data from the sensor
	bool sensor_communication_error; // Indication of a sensor communication error, service probably needs to be restarted
	bool data_saturated;             // Indication of sensor data being saturated, can cause result instability
	bool data_quality_warning;       // Indication of bad data quality that may be addressed by restarting the service to recalibrate the sensor
	bool data_available;             // Indicates if obstacle data is available or if more updates are needed
	bool proximity_detected;         // Indicates that object is detected close to the sensor, requires proximity_detection to be enabled, experimental
} acc_detector_obstacle_result_info_t;


/**
 * @brief Obstacle detector detector handler
 */
struct acc_detector_obstacle_handle;

typedef struct acc_detector_obstacle_handle *acc_detector_obstacle_handle_t;


/**
 * @brief Create an obstacle detector.
 *
 * @param[in] obstacle_configuration The configuration to create the detector with
 * @return A handle for the created detector or a nullpointer if creation failed
 */
acc_detector_obstacle_handle_t acc_detector_obstacle_create(acc_detector_obstacle_configuration_t obstacle_configuration);


/**
 * @brief Activate an obstacle detector.
 *
 * Activate an obstacle detector associated by handle. Use acc_detector_obstacle_get_next
 * to retrieve obstacles.
 *
 * Activating an already activated detector has no effect and true will be returned.
 *
 * If handle is a nullpointer the activation will fail and return false.
 *
 * @param[in] handle A handle for a detector to be activated
 * @return True if activation succeeded otherwise false.
 */
bool acc_detector_obstacle_activate(acc_detector_obstacle_handle_t handle);


/**
 * @brief Deactivate an obstacle detector.
 *
 * Deactivate an obstacle detector associated by the handle.
 *
 * Deactivating an already deactivated detector has no effect and true will be returned.
 * If handle is a nullpointer the activation will fail and return false.
 *
 * If the deactivation fails other than if the handle is a nullpointer, the system is in
 * an undefined state and further activation of detectors and services should be avoided
 * in the same session.
 *
 * @param[in] handle A handle for a detector to be deactivated
 * @return True if deactivation succeed otherwise false
 *
 */
bool acc_detector_obstacle_deactivate(acc_detector_obstacle_handle_t handle);


/**
 * @brief Estimate background
 *
 * This fetches one update from the sensor and use it to estimate the background. This assumes
 * that there are no obstacles in front of the sensor.
 *
 * @param[in] handle The obstacle detector handle
 * @param[out] completed true when background estimation is finished
 * @param[out] result_info The result info for the update
 *
 * @return True if successful, otherwise false
 */
bool acc_detector_obstacle_estimate_background(acc_detector_obstacle_handle_t      handle,
                                               bool                                *completed,
                                               acc_detector_obstacle_result_info_t *result_info);


/**
 * @brief Get background estimation data
 *
 * @param[in] handle The handle
 * @param[out] background_estimation Buffer used for background estimation data. Must be at least
 *                                   acc_detector_obstacle_background_estimation_get_size() bytes long.
 *
 * @return True if successful, otherwise false
 */
bool acc_detector_obstacle_background_estimation_get(acc_detector_obstacle_handle_t handle,
                                                     uint8_t                        *background_estimation);


/**
 * @brief Set background estimation data
 *
 * Background estimation data will be copied to detector.
 * NOTE! Use same configuration as when the background estimation data was saved.
 *
 * @param[in] handle The handle
 * @param[in] background_estimation The background estimation data to be used.
 *
 * @return True if successful, otherwise false
 */
bool acc_detector_obstacle_background_estimation_set(acc_detector_obstacle_handle_t handle,
                                                     const uint8_t                  *background_estimation);


/**
 * @brief Get the size of the background estimation buffer.
 *
 * @param[in] handle Obstacle handle
 * @return Size of the buffer needed
 */
size_t acc_detector_obstacle_background_estimation_get_size(acc_detector_obstacle_handle_t handle);


/**
 * @brief Get next update from the sensor and process it
 *
 * May only be called after an obstacle detector has been activated to retrieve the next result, blocks
 * the application until sensor data is ready. Not every call to this function will generate obstacle
 * information, the result_info->data_available indicates this.
 * It is important to call this function as often as required in order to not miss any data frames as the
 * algorithm requires a continuous supply of data updates in order to work properly.
 *
 * @param[in] handle The obstacle detector handle for the obstacle detector to get the next result for
 * @param[out] obstacle_data Information about detected obstacles that is going to be sent to the user.
 *                           note that this is only valid if result_info->data_available is set to true.
 *                           This parameter can be NULL if no result is wanted.
 * @param[out] result_info The result info for the last update
 *
 * @return True if successful, otherwise false
 */
bool acc_detector_obstacle_get_next(acc_detector_obstacle_handle_t      handle,
                                    acc_detector_obstacle_t             *obstacle_data,
                                    acc_detector_obstacle_result_info_t *result_info);


/**
 * @brief Destroy an obstacle detector
 *
 * Destroy an obstacle detector associated with handle.
 * The handle becomes invalid and is set to NULL.
 *
 * @param[in] handle A handle for a detector that will be destroyed
 */
void acc_detector_obstacle_destroy(acc_detector_obstacle_handle_t *handle);


/**
 * @brief Convert angle index to angle in degrees
 *
 * The angle index will be converted to the actual angle in degrees.
 * This depends on the assigned max speed and the current speed.
 *
 * @param[in] speed The current speed
 * @param[in] radial_velocity Radial velocity of obstacle
 * @return Angle in degrees
 */
float acc_detector_obstacle_radial_velocity_to_degrees(float speed, float radial_velocity);


/**
 * @}
 */

#endif
