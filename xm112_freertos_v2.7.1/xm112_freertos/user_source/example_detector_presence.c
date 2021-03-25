/*
 * Author: Roman Chak
 * To set the profile to 2 and the threshold to 10000 send the string  “P2;T10000;R;” to the application.
 * I have used RSS  SW v 2.6.0.
 * Copy this file to Ubuntu source folder and compile
 */

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

#include "acc_device_uart.h"
#include "acc_driver_gpio_same70.h"
#include "acc_app_integration.h"

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
//static acc_hal_t hal;
//extern void set_led(bool enable);
extern bool acc_device_gpio_write(uint_fast8_t pin, uint_fast8_t level);
static bool detect_presence(void);
static void uart_read_callback(uint_fast8_t port, uint8_t data, uint32_t status);// UART readback
//static bool acc_example_detector_presence(void);
static void update_configuration(acc_detector_presence_configuration_t presence_configuration);
//static void print_result(acc_detector_presence_result_t result);
static void uart_read_callback(uint_fast8_t port, uint8_t data, uint32_t status);

static void configure_presence(acc_detector_presence_configuration_t presence_configuration)
{
	/*
	 * Setting a higher profile in the call to the function
	 * acc_detector_presence_configuration_service_profile_set.
	 * This will make the sensor send out a longer pulse with more energy.
	 */
	acc_detector_presence_configuration_service_profile_set(presence_configuration, ACC_SERVICE_PROFILE_5);//ok ACC_SERVICE_PROFILE_3
	acc_detector_presence_configuration_update_rate_set(presence_configuration, DEFAULT_UPDATE_RATE);//ok
	acc_detector_presence_configuration_detection_threshold_set(presence_configuration, 1.8f);//ok The value 2.0 corresponds to the detection score  2000. We multiply by 1000 to avoid decimals when we print the score values.
	acc_detector_presence_configuration_start_set(presence_configuration, 0.7);//ok was 0.4, 0.7
	acc_detector_presence_configuration_length_set(presence_configuration, 2.0);//ok
	acc_detector_presence_configuration_power_save_mode_set(presence_configuration, ACC_BASE_CONFIGURATION_POWER_SAVE_MODE_SLEEP);//ok
}

int main(void)
{
	if (!acc_driver_hal_init())
	{
		return EXIT_FAILURE;
	}

	acc_device_uart_register_read_callback(0, uart_read_callback);

	if (!detect_presence())
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

//To set the profile to 2 and the threshold to 10000 send the string  "P2;T10000;R;" to the application.
void uart_read_callback(uint_fast8_t port, uint8_t data, uint32_t status)
{
	if (data == ';' && buffer_pos > 0) {
		switch(input_string[0])
		{
			case 'P' : profile = atoi(&input_string[1]); break;//Parses the C-string str interpreting its content as an integral number, which is returned as a value of type int.
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

bool detect_presence(void)
{

	int dist;
	unsigned int i;
	unsigned int threshld;
	bool RDR_OK;

	printf("Acconeer software version %s\n", acc_version_get());
	const acc_hal_t *hal = acc_driver_hal_get_implementation();

	if (!acc_rss_activate(hal))	//The Radar System Services (RSS) must be activated before any other calls are done  to the radar sensor service API
	{
		fprintf(stderr, "Failed to activate RSS\n");
		return false;
	}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	bool success ;
	bool deactivated;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//Presence Service Configuration
	acc_detector_presence_configuration_t presence_configuration = acc_detector_presence_configuration_create();	//First a configuration is created
	if (presence_configuration == NULL)
	{
		fprintf(stderr, "Failed to create configuration\n");
		return false;
	}
	/*
	 * The newly created service configuration contains default settings for all configuration parameters and
	 * can be passed directly to the acc_service_create function. However, in most scenarios there is a
	 * need to change at least some of the configuration parameters.
	 */
	configure_presence(presence_configuration);		//First a configuration is created
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
		//acc_app_integration_sleep_us(1000000 / DEFAULT_UPDATE_RATE);
		acc_app_integration_sleep_ms(1000 / DEFAULT_UPDATE_RATE);
	}

	threshld = 0;
	for (i = 0; i < 10; i++){
		acc_detector_presence_get_next(handle, &result);//bool - True if successful, otherwise false
		dist = (int)(result.presence_distance * 1000.0f);
		threshld = (unsigned int)dist + threshld;
		//acc_app_integration_sleep_us(1000000 / DEFAULT_UPDATE_RATE);
		acc_app_integration_sleep_ms(1000 / DEFAULT_UPDATE_RATE);
	}
	threshld = threshld / 10;// Set distance threshold. Mean value of the 10 reads
	threshld = threshld - 50;
	//set_led(1);
	//acc_device_gpio_write(XM11x_LED_PIN, 1);
	deactivated = acc_detector_presence_deactivate(handle);
	acc_detector_presence_destroy(&handle);
	acc_detector_presence_configuration_destroy(&presence_configuration);
	while(1)
	{
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		while (!restart)
		{
			printf("Threshold = %4d   ", threshld);
			RDR_OK = acc_detector_presence_get_next(handle, &result);//bool - True if successful, otherwise false
			if (RDR_OK == 1){
				printf("Sensor OK    ");
			}else{
				printf("Fault    ");//Faulty sensor
			}

			if (result.presence_detected)
			{
				printf("Motion    ");//Person detected
			}else
			{
				printf("Static    ");//Empty stall
			}

			dist = (int)(result.presence_distance * 1000.0f);

			if (dist < threshld){
				printf("Object    ");
			}else{
				printf("Empty     ");
			}
			printf("Score: %5d, Distance: %4d\n", (int)(result.presence_score * 1000.0f), (int)(result.presence_distance * 1000.0f));

			//acc_app_integration_sleep_us(1000000 / DEFAULT_UPDATE_RATE);
			acc_app_integration_sleep_ms(1000 / DEFAULT_UPDATE_RATE);
		}
		restart = false;
		deactivated = acc_detector_presence_deactivate(handle);
		acc_detector_presence_destroy(&handle);
	}
	acc_detector_presence_deactivate(handle);
	acc_detector_presence_destroy(&handle);
	acc_detector_presence_configuration_destroy(&presence_configuration);
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


/*void print_result(acc_detector_presence_result_t result)
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
*/
