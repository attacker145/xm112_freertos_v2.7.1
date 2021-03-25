// Copyright (c) Acconeer AB, 2016-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "acc_driver_gpio_same70.h"
#include "acc_device_gpio.h"
#include "acc_device_os.h"
#include "acc_log.h"

#include "board.h"
#include "pio.h"

/*
PA0-PA31 =>   0 -  31
PB0-PB31 =>  32 -  63
PC0-PC31 =>  64 -  95
PD0-PD31 =>  96 - 127
PE0-PE31 => 128 - 159
*/

/**
 * @brief The module name
 */
#define MODULE		"driver_gpio_same70"

/**
 * @brief Array with information on GPIOs, allocated by board
 */
static gpio_t *gpios;

/**
 * @brief Number of GPIO pins supported by the interface
 */
static uint_fast16_t gpio_count;


/**
 * @brief Translate pin number to port [PIOA..PIOIE] and local pin [0..31]
 *
 * @param[in] pin GPIO pin. Input to function
 * @param[out] pin_struct that will be populated. Pure output
 */
static void internal_gpio_get_port_and_pin(uint_fast8_t pin, struct _pin *pin_struct)
{
	pin_struct->group     = pin / 32;
	pin_struct->mask      = 1 << (pin % 32);
	pin_struct->type      = 0;
	pin_struct->attribute = PIO_DEFAULT;

	if (pin_struct->group >= PIO_GROUP_LENGTH) {
		ACC_LOG_ERROR("GPIO %" PRIu8" is outside supported range", pin);
	}

}

/**
 * @brief Returns a gpio struct, creates it if not already created
 * The gpio struct holds the status and pin number of the defined pin
 *
 * @param[in] pin pin number to return a gpio struct for
 * @return a gpio or NULL in case of failure
 */
static gpio_t* internal_gpio_get(uint_fast8_t pin)
{
	if (!gpios[pin].init)
	{
		internal_gpio_get_port_and_pin(pin, &gpios[pin].pin_struct);
		gpios[pin].init = true;
	}

	return &gpios[pin];
}


/**
 * @brief IOpens a GPIO
 *
 * @param[in] pin GPIO to open
 * @param[in] attribute Pin attribute
 * @return The open gpio or NULL in case of failure
 */
static gpio_t* internal_gpio_open(uint_fast8_t pin, uint32_t attribute)
{
	gpio_t *gpio = NULL;

	gpio = internal_gpio_get(pin);
	if (gpio == NULL) {
		return NULL;
	}

	if (!gpio->is_open) {
		gpio->pin_struct.type = PIO_INPUT;
		gpio->pin_struct.attribute = attribute;
		pio_configure(&gpio->pin_struct, 1); // 1 is the number of pins in first argument

		gpio->dir		= GPIO_DIR_IN;
		gpio->level		= 0;
		gpio->is_open		= 1;
	}

	return gpio;
}


/**
 * @brief Internal GPIO set edge
 *
 * Set GPIO edge to the specified edge on the specified pin.
 *
 * @param[in] gpio GPIO information
 * @param edge The edge to be set
 * @return true if ok
 */
static bool internal_gpio_set_edge(gpio_t *gpio, acc_gpio_edge_t edge)
{
	uint8_t pin_attr = PIO_DEFAULT;

	switch (edge) {
	case ACC_DEVICE_GPIO_EDGE_NONE:
		pin_attr = PIO_DEFAULT;
		break;

	case ACC_DEVICE_GPIO_EDGE_FALLING:
		pin_attr = PIO_IT_FALL_EDGE;
		break;

	case ACC_DEVICE_GPIO_EDGE_RISING:
		pin_attr = PIO_IT_RISE_EDGE;
		break;

	case ACC_DEVICE_GPIO_EDGE_BOTH:
		pin_attr = PIO_IT_BOTH_EDGE;
		break;
	}

	/* Only change the bits for controlling irq trigger */
	gpio->pin_struct.attribute = (gpio->pin_struct.attribute & ~PIO_IT_MASK) | pin_attr;

	return true;
}


/**
 * @brief Internal GPIO set direction and level
 *
 * If 'dir' is GPIO_DIR_IN, 'level' is not used.
 *
 * @param[in] gpio GPIO information
 * @param[in] dir The direction to be set
 * @param[in] level The level to be set
 */
static bool internal_gpio_set_dir(gpio_t *gpio, gpio_dir_t dir, uint_fast8_t level)
{
	if (dir == GPIO_DIR_IN) {
		gpio->pin_struct.type = PIO_INPUT;
		pio_configure(&gpio->pin_struct, 1); // 1 is the number of pins in first argument
	} else {
		gpio->level = level ? 1 : 0;
		if (level) {
			gpio->pin_struct.type = PIO_OUTPUT_1;
		} else {
			gpio->pin_struct.type = PIO_OUTPUT_0;
		}
		pio_configure(&gpio->pin_struct, 1); // 1 is the number of pins in first argument
	}

	gpio->dir = dir;

	return true;
}


/**
 * @brief Check if an interrupt service routine has been registered with a GPIO
 *
 * @param[in] gpio The GPIO information
 * @return True if an interrupt service routine has been registered
 */
static bool is_isr_registered(const gpio_t *gpio)
{
	bool registered = false;

	if (gpio->mutex != NULL) {
		acc_os_mutex_lock(gpio->mutex);
		registered = gpio->isr != NULL;
		acc_os_mutex_unlock(gpio->mutex);
	}
	return registered;
}


/**
 * @brief GPIO interrupt ISR
 *
 * @param[in] group The GPIO group that caused the interrupt
 * @param[in] status The PIO_ISR value
 * @param[in] arg The user argument which is gpio_t * in this case
 */
static void gpio_isr(uint32_t group, uint32_t status, void *arg)
{
	(void)group; // Ignore parameter
	(void)status; // Ignore parameter
	gpio_t *gpio = (gpio_t *)arg;
	if (gpio->isr != NULL) {
		gpio->isr();
	}
}


/**
 * @brief Unregister an interrupt service routine
 *
 * @param pin The GPIO pin where the callback is registered.
 */
static void unregister_isr(uint_fast8_t pin)
{
	gpio_t *gpio = &gpios[pin];

	if (is_isr_registered(gpio)) {
		acc_os_mutex_lock(gpio->mutex);
		gpio->isr = NULL;
		pio_disable_it(&gpio->pin_struct);
		acc_os_mutex_unlock(gpio->mutex);

		acc_os_mutex_destroy(gpio->mutex);
		gpio->mutex = NULL;
	}
}


/**
 * @brief Register an interrupt service routine
 *
 * @param pin The GPIO pin to listen to
 * @param edge The edge that will trigger the isr
 * @param[in] isr The interrupt service routine which will be triggered on an interrupt
 * @return True if the interrupt service routine was successfully registered.
 */
static bool register_isr(uint_fast8_t pin, acc_gpio_edge_t edge, acc_device_gpio_isr_t isr)
{
	gpio_t *gpio;

	gpio = internal_gpio_open(pin, PIO_DEFAULT);

	if (gpio == NULL)
	{
		ACC_LOG_ERROR("GPIO not found");
	}

	if (is_isr_registered(gpio)) {
		// A callback is already registered so just swap it
		acc_os_mutex_lock(gpio->mutex);
		gpio->isr = isr;
		acc_os_mutex_unlock(gpio->mutex);

		return true;
	}

	gpio->mutex = acc_os_mutex_create();
	if (gpio->mutex == NULL) {
		ACC_LOG_ERROR("Failed to create mutex.");
		gpio->isr = NULL;
		return false;
	}

	acc_os_mutex_lock(gpio->mutex);
	if (!internal_gpio_set_edge(gpio, edge)) {
		acc_os_mutex_unlock(gpio->mutex);
		return false;
	}

	gpio->isr = isr;
	pio_configure(&gpio->pin_struct, 1);
	pio_add_handler_to_group(gpio->pin_struct.group, gpio->pin_struct.mask, gpio_isr, gpio);
	pio_enable_it(&gpio->pin_struct);

	acc_os_mutex_unlock(gpio->mutex);
	return true;
}


/**
 * @brief Initiate memory for gpios and set initial state of all gpios
 */
static void initiate_gpio_mem(void)
{
	memset(gpios, 0, (sizeof(gpio_t*) * (gpio_count + 1)));

	for (uint_fast16_t pin = 0; pin < gpio_count; pin++)
	{
		memset(&gpios[pin], 0, sizeof(gpio_t));
		gpios[pin].dir = GPIO_DIR_UNKNOWN;
		gpios[pin].level = -1;
		gpios[pin].is_open = 0;
		gpios[pin].init = false;
	}
}


/**
 * @brief Initialize GPIO driver
 */
static bool acc_driver_gpio_same70_init(void)
{
	// Switch TDI function to PIO PB4 function
	MATRIX->CCFG_SYSIO |= CCFG_SYSIO_SYSIO4;

	// Disable ERASE pin functionality in order to avoid unintentional erases
	// when pressing the ERASE button. The pin is still active during reset.
	MATRIX->CCFG_SYSIO |= CCFG_SYSIO_SYSIO12;

	initiate_gpio_mem();

	ACC_LOG_VERBOSE("SAME70 GPIO driver initialized");

	return true;
}


/**
 * @brief Inform the driver of the pull up/down level for a GPIO pin after reset
 *
 * This function in this driver only opens the pin.
 *
 * The GPIO pin numbering is decided by the GPIO driver.
 *
 * @param[in] pin Pin number
 * @param[in] level The pull level 0 or 1
 */
static bool acc_driver_gpio_same70_set_initial_pull(uint_fast8_t pin, uint_fast8_t level)
{
	gpio_t* gpio;

	gpio = internal_gpio_open(pin, level ? PIO_PULLUP : PIO_PULLDOWN);
	if (gpio == NULL) {
		return false;
	}

	return true;
}


/**
 * @brief Set GPIO to input
 *
 * This function sets the direction of a GPIO to input.
 * GPIO parameter is not a pin number but GPIO index (0-X).
 *
 * @param[in] pin GPIO pin to be set to input
 */
static bool acc_driver_gpio_same70_input(uint_fast8_t pin)
{
	gpio_t *gpio;

	gpio = internal_gpio_open(pin, PIO_DEFAULT);
	if (gpio == NULL) {
		return false;
	}

	if (gpio->dir == GPIO_DIR_IN) {
		return true;
	}

	return internal_gpio_set_dir(gpio, GPIO_DIR_IN, 0);
}


/**
 * @brief Read from GPIO
 *
 * @param[in] pin GPIO pin to read
 * @param[out] value The value which has been read
 */
static bool acc_driver_gpio_same70_read(uint_fast8_t pin, uint_fast8_t *value)
{
	gpio_t *gpio;

	gpio = internal_gpio_open(pin, PIO_DEFAULT);
	if (gpio == NULL) {
		return false;
	}

	if (gpio->dir != GPIO_DIR_IN) {
		ACC_LOG_ERROR("Cannot read GPIO %u as it is output/unknown", pin);
		return false;
	}

	*value = (bool)pio_get(&gpio->pin_struct);

	return true;
}


/**
 * @brief Set GPIO output level
 *
 * This function sets a GPIO to output and the level to low or high.
 *
 * @param[in] pin GPIO pin to be set
 * @param[in] level 0 to 1 to set pin low or high
 */
static bool acc_driver_gpio_same70_write(uint_fast8_t pin, uint_fast8_t level)
{
	gpio_t *gpio;

	gpio = internal_gpio_open(pin, PIO_DEFAULT);
	if (gpio == NULL) {
		return false;
	}

	return internal_gpio_set_dir(gpio, GPIO_DIR_OUT, level);
}


/**
 * @brief Register an interrupt service routine for a GPIO pin
 *
 * Registers an interrupt service routine which will be called when the specified edge is detected on the selected GPIO pin.
 * If true is returned the interrupt service routine will immediately be triggered if the specified edge is detected.
 * If a new interrupt service routine is registered it will replace the old one.
 *
 * The interrupt service routine can be unregistered by register NULL.
 * Unregister an already unregistered interrupt service routine has no effect.
 *
 * @param pin GPIO pin the interrupt service routine will be attached to.
 * @param edge The edge that will trigger the interrupt service routine. Can be set to "falling", "rising" or both.
 * @param isr The function to be called when the specified edge is detected.
 * @return true if the interrupt service routine was registered, false if the specified pin does not
 *      support interrupts or if the registration failed.
 */
static bool acc_driver_gpio_same70_register_isr(uint_fast8_t pin, acc_gpio_edge_t edge, acc_device_gpio_isr_t isr)
{
	if (isr == NULL) {
		unregister_isr(pin);
	} else {
		if (!register_isr(pin, edge, isr)) {
			return false;
		}
	}

	return true;
}


void acc_driver_gpio_same70_register(uint_fast16_t pin_count, gpio_t *gpio_mem)
{
	gpios = gpio_mem;
	gpio_count = pin_count;

	acc_device_gpio_init_func		= acc_driver_gpio_same70_init;
	acc_device_gpio_set_initial_pull_func	= acc_driver_gpio_same70_set_initial_pull;
	acc_device_gpio_input_func		= acc_driver_gpio_same70_input;
	acc_device_gpio_read_func		= acc_driver_gpio_same70_read;
	acc_device_gpio_write_func		= acc_driver_gpio_same70_write;
	acc_device_gpio_register_isr_func	= acc_driver_gpio_same70_register_isr;
}
