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
#include "acc_device_gpio.h"


static acc_hal_t hal;

#define UPDATE_RATE                 (10)
#define XM11x_LED_PIN        67  // PC3 used in void set_led

//extern void set_led(bool enable);

static bool detect_presence(void);
void set_led(bool enable);


static void configure_presence(acc_detector_presence_configuration_t presence_configuration)
{
	acc_detector_presence_configuration_service_profile_set(presence_configuration, ACC_SERVICE_PROFILE_3);//You can also try set a higher profile in the call to the function
	acc_detector_presence_configuration_update_rate_set(presence_configuration, UPDATE_RATE);
	acc_detector_presence_configuration_detection_threshold_set(presence_configuration, 2.0f);//The value 2.0 corresponds to the detection score  2000. We multiply by 1000 to avoid decimals when we print the score values.
	acc_detector_presence_configuration_start_set(presence_configuration, 0.4);
	acc_detector_presence_configuration_length_set(presence_configuration, 2.0);
	acc_detector_presence_configuration_power_save_mode_set(presence_configuration, ACC_BASE_CONFIGURATION_POWER_SAVE_MODE_SLEEP);
}

void set_led(bool enable)
{
	if (enable)
	{
		// Driving the pin low enables LED
		acc_device_gpio_write(XM11x_LED_PIN, 0);
	}
	else
	{
		// Driving the pin high disables LED
		acc_device_gpio_write(XM11x_LED_PIN, 1);
	}
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

	int dist;
	unsigned int i;
	unsigned int threshld;

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

	for (i = 0; i < 300; i++){//30sec
		acc_detector_presence_get_next(handle, &result);
		dist = (int)(result.presence_distance * 1000.0f);
		printf("I-Score: %5d, I-Distance: %4d\n", (int)(result.presence_score * 1000.0f), (int)(result.presence_distance * 1000.0f));
		acc_app_integration_sleep_us(1000000 / UPDATE_RATE);
	}

	threshld = 0;
	for (i = 0; i < 10; i++){
		acc_detector_presence_get_next(handle, &result);
		dist = (int)(result.presence_distance * 1000.0f);
		threshld = (unsigned int)dist + threshld;
		acc_app_integration_sleep_us(1000000 / UPDATE_RATE);
	}
	threshld = threshld / 10;// Set distance threshold. Mean value of the 10 reads
	threshld = threshld - 50;
	//set_led(1);
	//acc_device_gpio_write(XM11x_LED_PIN, 1);
	while(1)
	{
		printf("Threshold = %4d   ", threshld);

		acc_detector_presence_get_next(handle, &result);

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
		printf("Score: %5d, Distance: %4d\n", (int)(result.presence_score * 1000.0f), (int)(result.presence_distance * 1000.0f));

		acc_app_integration_sleep_us(1000000 / UPDATE_RATE);
	}

	acc_detector_presence_deactivate(handle);

	acc_detector_presence_destroy(&handle);

	acc_detector_presence_configuration_destroy(&presence_configuration);

	return true;
}
