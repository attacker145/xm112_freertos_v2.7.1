// Copyright (c) Acconeer AB, 2019
// All rights reserved

#include <stdbool.h>
#include <stdio.h>

#include "acc_app_integration.h"
#include "acc_base_configuration.h"
#include "acc_definitions.h"
#include "acc_detector_presence.h"
#include "acc_driver_hal.h"
#include "acc_hal_definitions.h"
#include "acc_rss.h"
#include "acc_version.h"


static acc_hal_t hal;

#define UPDATE_RATE                 (10)


static bool detect_presence(void);


static void configure_presence(acc_detector_presence_configuration_t presence_configuration)
{
	acc_detector_presence_configuration_service_profile_set(presence_configuration, ACC_SERVICE_PROFILE_3);
	acc_detector_presence_configuration_update_rate_set(presence_configuration, UPDATE_RATE);
	acc_detector_presence_configuration_detection_threshold_set(presence_configuration, 2.0f);
	acc_detector_presence_configuration_start_set(presence_configuration, 0.4);
	acc_detector_presence_configuration_length_set(presence_configuration, 2.0);
	acc_detector_presence_configuration_power_save_mode_set(presence_configuration, ACC_BASE_CONFIGURATION_POWER_SAVE_MODE_SLEEP);
}


int main(void)
{
	if (!acc_driver_hal_init())
	{
		return EXIT_FAILURE;
	}

	if (!detect_presence())
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


bool detect_presence(void)
{

	int dist, threshld;

	printf("Acconeer software version %s\n", acc_version_get());

	hal = acc_driver_hal_get_implementation();

	if (!acc_rss_activate(&hal))
	{
		fprintf(stderr, "Failed to activate RSS\n");
		return false;
	}

	acc_detector_presence_configuration_t presence_configuration = acc_detector_presence_configuration_create();
	if (presence_configuration == NULL)
	{
		fprintf(stderr, "Failed to create configuration\n");
		return false;
	}

	configure_presence(presence_configuration);

	acc_detector_presence_handle_t handle = acc_detector_presence_create(presence_configuration);
	if (handle == NULL)
	{
		fprintf(stderr, "Failed to create detector\n");
		return false;
	}

	if (!acc_detector_presence_activate(handle))
	{
		fprintf(stderr, "Failed to activate detector\n");
		return false;
	}

	acc_detector_presence_result_t result;

	//for (int i = 0; i < 2000; i++)

	acc_detector_presence_get_next(handle, &result);
	dist = (int)(result.presence_distance * 1000.0f);
	//threshld = dist - 50;
	threshld = 1000;

	while(1)
	{
		acc_detector_presence_get_next(handle, &result);

		if (result.presence_detected)
		{
			printf("Motion    ");
		}
		else
		{
			printf("Static    ");
		}

		dist = (int)(result.presence_distance * 1000.0f);

		if (dist < threshld){
			printf("Object    ");
		}
		else{
			printf("Empty     ");
		}
		printf("Score: %5d, Distance: %4d\n", (int)(result.presence_score * 1000.0f), (int)(result.presence_distance * 1000.0f));

		acc_app_integration_sleep_us(1000000 / UPDATE_RATE);
	}

	acc_detector_presence_deactivate(handle);

	acc_detector_presence_destroy(&handle);

	acc_detector_presence_configuration_destroy(&presence_configuration);

	return true;
}
