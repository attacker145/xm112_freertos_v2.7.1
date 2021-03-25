// Copyright (c) Acconeer AB, 2019-2020
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

#include "acc_device_uart.h"
#include "acc_driver_gpio_same70.h"

/** \example example_detector_presence.c
 * @brief This is an example on how the presence detector can be used
 * @n
 * The example executes as follows:
 *   - Activate Radar System Software (RSS)
 *   - Create a presence detector configuration
 *   - Create a presence detector using the previously created configuration
 *   - Destroy the presence detector configuration
 *   - Activate the presence detector
 *   - Get the result and print it 200 times
 *   - Deactivate and destroy the presence detector
 *   - Deactivate Radar System Software (RSS)
 */


#define DEFAULT_START_M             (0.2f)
#define DEFAULT_LENGTH_M            (1.6f)
#define DEFAULT_UPDATE_RATE         (10)
#define DEFAULT_POWER_SAVE_MODE     ACC_POWER_SAVE_MODE_SLEEP
#define DEFAULT_DETECTION_THRESHOLD (3.0f)


static bool acc_example_detector_presence(void);


static void update_configuration(acc_detector_presence_configuration_t presence_configuration);


static void print_result(acc_detector_presence_result_t result);

static void uart_read_callback(uint_fast8_t port, uint8_t data, uint32_t status);




int main(void)
{
	if (!acc_driver_hal_init())
	{
		return EXIT_FAILURE;
	}

	acc_device_uart_register_read_callback(0, uart_read_callback);


	if (!acc_example_detector_presence())
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


static volatile int profile = 3;
static volatile int threshold = DEFAULT_DETECTION_THRESHOLD;
static volatile bool restart = false;

volatile int buffer_pos = 0;


#define MAX_INPUT_LENGTH  32

char input_string[MAX_INPUT_LENGTH];


void uart_read_callback(uint_fast8_t port, uint8_t data, uint32_t status)
{


	
	if (data == ';' && buffer_pos > 0) {

	
		switch(input_string[0])
		{
			case 'P' : profile = atoi(&input_string[1]); break;
			case 'T' : threshold = atoi(&input_string[1]); break;
			case 'R' : restart = true; break;
		}
	
		buffer_pos=0;

	} else if (data >= '0' && data <= 'Z') 
	{
		input_string[buffer_pos] = data;
		if (buffer_pos < MAX_INPUT_LENGTH-1)
			buffer_pos++;
		
	}

	input_string[buffer_pos] = '\0';

}



bool acc_example_detector_presence(void)
{
	printf("Acconeer software version %s\n", acc_version_get());

	const acc_hal_t *hal = acc_driver_hal_get_implementation();

	if (!acc_rss_activate(hal))
	{
		fprintf(stderr, "Failed to activate RSS\n");
		return false;
	}

	bool success ;
	bool deactivated;

	while(true){
		success    = true;
		acc_detector_presence_configuration_t presence_configuration = acc_detector_presence_configuration_create();
		if (presence_configuration == NULL)
		{
			fprintf(stderr, "Failed to create configuration\n");
			acc_rss_deactivate();
			return false;
		}

		update_configuration(presence_configuration);

		acc_detector_presence_handle_t handle = acc_detector_presence_create(presence_configuration);
		if (handle == NULL)
		{
			fprintf(stderr, "Failed to create detector\n");
			acc_detector_presence_configuration_destroy(&presence_configuration);
			acc_rss_deactivate();
			return false;
		}

		acc_detector_presence_configuration_destroy(&presence_configuration);

		if (!acc_detector_presence_activate(handle))
		{
			fprintf(stderr, "Failed to activate detector\n");
			acc_detector_presence_destroy(&handle);
			acc_rss_deactivate();
			return false;
		}


		acc_detector_presence_result_t result;

		while (!restart)
		{
			success = acc_detector_presence_get_next(handle, &result);
			if (!success)
			{
				fprintf(stderr, "acc_detector_presence_get_next() failed\n");
				break;
			}

			print_result(result);

			// printf("buffer_pos=%d  input_string=%s\n", buffer_pos, input_string);

			acc_app_integration_sleep_ms(1000 / DEFAULT_UPDATE_RATE);
		}

		restart = false;

		deactivated = acc_detector_presence_deactivate(handle);

		acc_detector_presence_destroy(&handle);
	}

	acc_rss_deactivate();

	return deactivated && success;
}


void update_configuration(acc_detector_presence_configuration_t presence_configuration)
{
	printf("Updating configuration\n");
	printf("profile = %d\n", profile);
	printf("threshold = %d\n", threshold);

	acc_detector_presence_configuration_update_rate_set(presence_configuration, DEFAULT_UPDATE_RATE);
	acc_detector_presence_configuration_detection_threshold_set(presence_configuration, threshold);
	acc_detector_presence_configuration_start_set(presence_configuration, DEFAULT_START_M);
	acc_detector_presence_configuration_length_set(presence_configuration, DEFAULT_LENGTH_M);
	acc_detector_presence_configuration_power_save_mode_set(presence_configuration, DEFAULT_POWER_SAVE_MODE);
	acc_detector_presence_configuration_service_profile_set(presence_configuration, profile);
}

#define XM11x_LED_PIN        67  // PC3


void print_result(acc_detector_presence_result_t result)
{
	if (result.presence_detected)
	{
		printf("Motion\n");
		acc_device_gpio_write(XM11x_LED_PIN, 0);
	}
	else
	{
		printf("No motion\n");
		acc_device_gpio_write(XM11x_LED_PIN, 1);
	}

	printf("Presence score: %d, Distance: %d\n", (int)(result.presence_score * 1000.0f), (int)(result.presence_distance * 1000.0f));
}
