// Copyright (c) Acconeer AB, 2018-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "acc_detector_obstacle.h"
#include "acc_driver_hal.h"
#include "acc_hal_definitions.h"
#include "acc_rss.h"
#include "acc_version.h"


/** \example example_detector_obstacle.c
 * @brief This is an example on how the obstacle detector can be used
 * @n
 * The example executes as follows:
 *   - Activate Radar System Software (RSS)
 *   - Create an obstacle detector configuration
 *   - Create an obstacle detector using the previously created configuration
 *   - Activate the obstacle detector
 *   - Optionally do background estimation
 *   - Get the result and print it 5 times
 *   - Deactivate and destroy the obstacle detector
 *   - Destroy the obstacle detector configuration
 *   - Deactivate Radar System Software (RSS)
 */


static const acc_hal_t *hal;

static uint8_t *background_estimation_data = NULL;

static bool execute_obstacle_detection(acc_detector_obstacle_configuration_t configuration);


static bool acc_example_detector_obstacle(void);


int main(void)
{
	if (!acc_driver_hal_init())
	{
		return EXIT_FAILURE;
	}

	if (!acc_example_detector_obstacle())
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


bool acc_example_detector_obstacle(void)
{
	printf("Acconeer software version %s\n", acc_version_get());

	hal = acc_driver_hal_get_implementation();

	if (!acc_rss_activate(hal))
	{
		printf("acc_rss_activate() failed\n");
		return false;
	}

	acc_detector_obstacle_configuration_t configuration = acc_detector_obstacle_configuration_create();

	if (configuration == NULL)
	{
		printf("acc_detector_obstacle_configuration_create failed\n");
		acc_rss_deactivate();
		return false;
	}

	if (!execute_obstacle_detection(configuration))
	{
		acc_detector_obstacle_configuration_destroy(&configuration);
		acc_rss_deactivate();
		return false;
	}

	acc_detector_obstacle_configuration_destroy(&configuration);

	acc_rss_deactivate();

	return true;
}


bool execute_obstacle_detection(acc_detector_obstacle_configuration_t configuration)
{
	acc_detector_obstacle_handle_t handle = acc_detector_obstacle_create(configuration);

	if (handle == NULL)
	{
		printf("acc_detector_obstacle_create() failed\n");
		return false;
	}

	if (!acc_detector_obstacle_activate(handle))
	{
		printf("acc_detector_obstacle_activate() failed\n");
		acc_detector_obstacle_destroy(&handle);
		return false;
	}

	// Handle background estimation differently depending on if it has been done before or ont
	if (background_estimation_data == NULL)
	{
		acc_detector_obstacle_result_info_t result_info;
		bool                                completed;

		do
		{
			if (!acc_detector_obstacle_estimate_background(handle, &completed, &result_info))
			{
				printf("acc_detector_obstacle_estimate_background() failed\n");
				acc_detector_obstacle_deactivate(handle);
				acc_detector_obstacle_destroy(&handle);
				return false;
			}
		} while (!completed);

		size_t background_estimation_size = acc_detector_obstacle_background_estimation_get_size(handle);

		background_estimation_data = hal->os.mem_alloc(background_estimation_size);
		if (background_estimation_data != NULL)
		{
			if (!acc_detector_obstacle_background_estimation_get(handle, background_estimation_data))
			{
				printf("acc_detector_obstacle_background_estimation_get() failed\n");
				acc_detector_obstacle_deactivate(handle);
				acc_detector_obstacle_destroy(&handle);
				hal->os.mem_free(background_estimation_data);
				background_estimation_data = NULL;
				return false;
			}
		}
	}
	else
	{
		if (!acc_detector_obstacle_background_estimation_set(handle, background_estimation_data))
		{
			printf("acc_detector_obstacle_background_estimation_get() failed\n");
			acc_detector_obstacle_deactivate(handle);
			acc_detector_obstacle_destroy(&handle);
			hal->os.mem_free(background_estimation_data);
			background_estimation_data = NULL;
			return false;
		}
	}

	acc_obstacle_t          obstacles[16];
	acc_detector_obstacle_t obstacle_data;
	obstacle_data.obstacles = obstacles;

	bool      success    = true;
	const int iterations = 20;

	for (uint16_t i = 0; i < iterations; i++)
	{
		acc_detector_obstacle_result_info_t result_info;

		do
		{
			success = acc_detector_obstacle_get_next(handle, &obstacle_data, &result_info);
		} while (success && !result_info.data_available);

		if (!success)
		{
			printf("acc_detector_obstacle_get_next() failed\n");
			break;
		}

		uint8_t nbr_of_obstacles = obstacle_data.nbr_of_obstacles;
		printf("Detected %u obstacles!\n", (unsigned int)nbr_of_obstacles);

		if (nbr_of_obstacles > 0)
		{
			for (uint8_t j = 0; j < nbr_of_obstacles; j++)
			{
				printf("Obstacle at a distance %d mm with amplitude %d\n",
				       (int)(obstacle_data.obstacles[j].distance * 1000.0f),
				       (int)(obstacle_data.obstacles[j].amplitude * 1000.0f));
			}
		}

		printf("\n");
	}

	bool deactivated = acc_detector_obstacle_deactivate(handle);

	acc_detector_obstacle_destroy(&handle);

	hal->os.mem_free(background_estimation_data);
	background_estimation_data = NULL;

	return deactivated && success;
}
