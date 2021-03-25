// Copyright (c) Acconeer AB, 2019
// All rights reserved

#include <stdbool.h>
#include <stdio.h>

#include "acc_app_integration.h"
#include "acc_definitions.h"
#include "acc_detector_presence.h"
#include "acc_driver_hal.h"
#include "acc_hal_definitions.h"
#include "acc_rss.h"
#include "acc_version.h"


static acc_hal_t hal;

#define DEFAULT_START_M             (0.2f)
#define DEFAULT_LENGTH_M            (1.4f)
#define DEFAULT_UPDATE_RATE         (10)
#define DEFAULT_POWER_SAVE_MODE     ACC_POWER_SAVE_MODE_SLEEP
#define DEFAULT_DETECTION_THRESHOLD (2.0f)


static bool acc_example_detector_presence(void);


static void set_default_configuration(acc_detector_presence_configuration_t presence_configuration)
{
	acc_detector_presence_configuration_update_rate_set(presence_configuration, DEFAULT_UPDATE_RATE);
	acc_detector_presence_configuration_detection_threshold_set(presence_configuration, DEFAULT_DETECTION_THRESHOLD);
	acc_detector_presence_configuration_start_set(presence_configuration, DEFAULT_START_M);
	acc_detector_presence_configuration_length_set(presence_configuration, DEFAULT_LENGTH_M);
	acc_detector_presence_configuration_power_save_mode_set(presence_configuration, DEFAULT_POWER_SAVE_MODE);
}


int main(void)
{
	if (!acc_driver_hal_init())
	{
		return EXIT_FAILURE;
	}

	if (!acc_example_detector_presence())
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


bool acc_example_detector_presence(void)
{
	int dist;
	unsigned int i;
	unsigned int threshld = 0;
	bool RDR_OK;

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

	set_default_configuration(presence_configuration);

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

	for (i = 0; i < 300; i++){//30sec
		acc_detector_presence_get_next(handle, &result);//bool - True if successful, otherwise false
		dist = (int)(result.presence_distance * 1000.0f);
		printf("I-Score: %5d, I-Distance: %4d\n", (int)(result.presence_score * 1000.0f), (int)(result.presence_distance * 1000.0f));
		acc_app_integration_sleep_ms(1000 / DEFAULT_UPDATE_RATE);
	}

	threshld = 0;
	for (i = 0; i < 10; i++){
		acc_detector_presence_get_next(handle, &result);//bool - True if successful, otherwise false
		dist = (int)(result.presence_distance * 1000.0f);
		threshld = (unsigned int)dist + threshld;
		acc_app_integration_sleep_ms(1000 / DEFAULT_UPDATE_RATE);
	}
	threshld = threshld / 10;// Set distance threshold. Mean value of the 10 reads
	threshld = threshld - 50;
	while(1)
	{
		RDR_OK = acc_detector_presence_get_next(handle, &result);
		if (RDR_OK == 1){
			printf("Sensor OK    ");
		}else{
			printf("Fault    ");
		}
		if (result.presence_detected)
		{
			printf("Motion    ");//Person detected
		}
		else
		{
			printf("Static    ");//Empty stall
		}
		dist = (int)(result.presence_distance * 1000.0f);

		if (dist < threshld){
			printf("Object    ");
		}
		else{
			printf("Empty     ");
		}
		printf("Presence score: %d, Distance: %d\n", (int)(result.presence_score * 1000.0f), (int)(result.presence_distance * 1000.0f));

		acc_app_integration_sleep_ms(1000 / DEFAULT_UPDATE_RATE);
	}

	acc_detector_presence_deactivate(handle);

	acc_detector_presence_destroy(&handle);

	acc_detector_presence_configuration_destroy(&presence_configuration);

	return true;
}
