// Copyright (c) Acconeer AB, 2016-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "acc_device_os.h"
#include "acc_device_spi.h"
#include "acc_device_pm.h"
#include "acc_driver_spi_same70.h"
#include "acc_log.h"

#include "spid.h"
#include "bus.h"

/**
 * @brief The module name
 */
#define MODULE "driver_spi_same70"

#define SPI_BUS_MAX           2
#define SPI_DEVICE_MAX        2
#define MIN(a,b)              (a < b ? a : b)

typedef struct
{
	uint8_t          bus;
	uint_fast8_t     device;
	uint32_t         speed;
	bool             master;
	struct _spi_desc spi_desc;
	struct _buffer   async_buf;
	uint8_t          *async_user_buffer;
	bool             async_rx;
	acc_device_spi_transfer_callback_t async_transfer_cb;
	uint8_t          *buffer;
	uint8_t          *buffer_unaligned;
	size_t           buffer_size;
} acc_driver_spi_same70_handle_t;


static acc_driver_spi_same70_handle_t handles[SPI_BUS_MAX];


static wait_for_transfer_complete_t wait_for_transfer_complete_func;
static transfer_complete_callback_t transfer_complete_func;


/**
 * @brief Given the bus, give back some other useful spi parameters
 *
 * @param bus The SPI bus
 * @param[out] spi The SPI interface pointer
 */
static bool lookup_spi(uint_fast8_t bus, Spi **spi)
{
	if (bus == 0)
	{
		*spi    = SPI0;
	}
	else if (bus == 1)
	{
		*spi    = SPI1;
	}
	else
	{
		return false;
	}

	return true;
}


static acc_device_handle_t acc_driver_spi_same70_create(acc_device_spi_configuration_t *configuration)
{
	Spi          *spi;

	if (configuration->bus >= SPI_BUS_MAX)
	{
		ACC_LOG_ERROR("Invalid bus number");
		return NULL;
	}

	if (!lookup_spi(configuration->bus, &spi))
	{
		ACC_LOG_ERROR("lookup_spi failed");
		return NULL;
	}

	// The buffer size must be at least L1_CACHE_BYTES and a multiple of L1_CACHE_BYTES
	uint32_t buffer_size = configuration->buffer_size;
	if (buffer_size % L1_CACHE_BYTES != 0)
	{
		buffer_size += L1_CACHE_BYTES - buffer_size % L1_CACHE_BYTES;
	}

	uint8_t *buffer = acc_os_mem_alloc(buffer_size + L1_CACHE_BYTES - 1);
	if (buffer == NULL)
	{
		ACC_LOG_ERROR("Failed to allocate SPI transfer buffer");
		return NULL;
	}
	handles[configuration->bus].buffer_size = buffer_size;
	handles[configuration->bus].buffer_unaligned = buffer;
	handles[configuration->bus].buffer = (void *)(((uintptr_t)buffer + L1_CACHE_BYTES - 1) & ~(L1_CACHE_BYTES - 1));

	acc_driver_spi_same70_config_t spi_pins = *(acc_driver_spi_same70_config_t *)configuration->configuration;

	// Configure SPI GPIO pins
	pio_configure(&spi_pins.spi_miso, 1);
	pio_configure(&spi_pins.spi_mosi, 1);
	pio_configure(&spi_pins.spi_clk, 1);
	pio_configure(&spi_pins.spi_npcs, 1);

	handles[configuration->bus].spi_desc.addr = spi;
	handles[configuration->bus].spi_desc.chip_select = configuration->device;
	handles[configuration->bus].spi_desc.transfer_mode = BUS_TRANSFER_MODE_DMA;

	spid_configure(&handles[configuration->bus].spi_desc);
	spid_configure_master(&handles[configuration->bus].spi_desc, configuration->master);
	spid_configure_cs(&handles[configuration->bus].spi_desc,
	                  handles[configuration->bus].spi_desc.chip_select,
	                  configuration->speed / 1000, // bitrate in kbps
	                  0, // delay_dlybs
	                  0, // delay_dlybct
	                  SPID_MODE_0);

	ACC_LOG_VERBOSE("SAME70 SPI driver initialized");

	handles[configuration->bus].bus    = configuration->bus;
	handles[configuration->bus].device = configuration->device;
	handles[configuration->bus].speed  = configuration->speed;
	handles[configuration->bus].master = configuration->master;

	// Fill in actual speed
	configuration->speed = spid_get_cs_bitrate(&handles[configuration->bus].spi_desc,
	                                           handles[configuration->bus].spi_desc.chip_select);

	return (acc_device_handle_t)&handles[configuration->bus];
}


static void acc_driver_spi_same70_destroy(acc_device_handle_t *dev_handle)
{
	acc_driver_spi_same70_handle_t *handle = (acc_driver_spi_same70_handle_t *)*dev_handle;
	spid_destroy(&handle->spi_desc);
	acc_os_mem_free(handle->buffer_unaligned);
	handle->buffer_unaligned = NULL;
	handle->buffer = NULL;
	*dev_handle = NULL;
}


static int spi_transfer_complete_callback(void *arg1, void *arg2)
{
	(void)arg2;
	acc_device_handle_t dev_handle = (acc_device_handle_t)arg1;
	if (transfer_complete_func)
	{
		transfer_complete_func(dev_handle);
	}
	return 0;
}


static bool acc_driver_spi_same70_transfer(
	acc_device_handle_t dev_handle,
	uint8_t             *buffer,
	size_t              buffer_size)
{
	Spi          *spi;
	acc_driver_spi_same70_handle_t *handle = (acc_driver_spi_same70_handle_t *)dev_handle;

	/* Prevent low power mode until DMA transfer is completed */
	acc_device_pm_wake_lock();

	if ((handle->device >= SPI_DEVICE_MAX))
	{
		acc_device_pm_wake_unlock();
		return false;
	}

	if (!lookup_spi(handle->bus, &spi))
	{
		acc_device_pm_wake_unlock();
		return false;
	}

	size_t transferred = 0;
	while (transferred < buffer_size)
	{
		uint32_t chunk_size = MIN(handle->buffer_size, buffer_size - transferred);
		// We need to copy the data to cache line aligned memory so that we
		// don't invalidate data outside the buffer
		memcpy(handle->buffer, buffer + transferred, chunk_size);
		struct _buffer buf = {
			.data = handle->buffer,
			.size = chunk_size,
			.attr = BUS_BUF_ATTR_RX | BUS_BUF_ATTR_TX,
		};

		if (transferred + chunk_size == buffer_size)
		{
			// Release CS after last transfer
			buf.attr |= BUS_SPI_BUF_ATTR_RELEASE_CS;
		}

		struct _callback callback = {
			.method = spi_transfer_complete_callback,
			.arg = dev_handle
		};

		int err = spid_transfer(&handle->spi_desc, &buf, 1, &callback);
		if (err)
		{
			acc_device_pm_wake_unlock();
			return false;
		}

		if (wait_for_transfer_complete_func)
		{
			wait_for_transfer_complete_func(dev_handle);
		}
		spid_wait_transfer(&handle->spi_desc);

		// Copy back the data to the buffer
		memcpy(buffer + transferred, handle->buffer, chunk_size);

		transferred += chunk_size;
	}

	acc_device_pm_wake_unlock();

	return true;
}


static int spi_transfer_async_callback(void *arg1, void *arg2)
{
	acc_device_handle_t dev_handle = (acc_device_handle_t)arg1;
	acc_driver_spi_same70_handle_t *handle = (acc_driver_spi_same70_handle_t *)dev_handle;

	if (handle->async_rx)
	{
		// Copy back data from the cache aligned buffer
		memcpy(handle->async_user_buffer, handle->buffer, handle->async_buf.size);
	}

	if (handle->async_transfer_cb)
	{
		handle->async_transfer_cb(dev_handle, (acc_device_spi_transfer_status_t)arg2);
	}
	return 0;
}


static bool acc_driver_spi_same70_transfer_async(
	acc_device_handle_t dev_handle,
	uint8_t             *buffer,
	bool                rx,
	bool                tx,
	size_t              buffer_size,
	acc_device_spi_transfer_callback_t callback)
{
	Spi          *spi;
	struct _callback transfer_callback = {NULL,};
	acc_driver_spi_same70_handle_t *handle = (acc_driver_spi_same70_handle_t *)dev_handle;

	if ((handle->device >= SPI_DEVICE_MAX))
	{
		return false;
	}

	if (!lookup_spi(handle->bus, &spi))
	{
		return false;
	}

	handle->async_rx = rx;
	if (rx)
	{
		// If we receive data, caches will be invalidated by the driver
		// so we must copy to a cache line aligned buffer in order
		// to prevent possible data loss of surrounding data.
		assert(handle->buffer_size >= buffer_size);
		memcpy(handle->buffer, buffer, buffer_size);
		handle->async_buf.data = handle->buffer;
		handle->async_user_buffer = buffer;
	}
	else
	{
		handle->async_buf.data = buffer;
		handle->async_user_buffer = NULL;
	}

	handle->async_buf.size = buffer_size;
	handle->async_buf.attr = 0;
	if (rx)
	{
		handle->async_buf.attr |= BUS_BUF_ATTR_RX;
	}

	if (tx)
	{
		handle->async_buf.attr |= BUS_BUF_ATTR_TX;
	}
	handle->async_transfer_cb = callback;

	if (handle->master)
	{
		handle->async_buf.attr |= BUS_SPI_BUF_ATTR_RELEASE_CS;
	}

	callback_set(&transfer_callback, (callback_method_t)spi_transfer_async_callback, dev_handle);

	int err = spid_transfer(&handle->spi_desc, &handle->async_buf, 1, &transfer_callback);
	if (err)
	{
		return false;
	}

	return true;
}


static uint8_t acc_driver_spi_same70_get_bus(acc_device_handle_t dev_handle)
{
	acc_driver_spi_same70_handle_t *handle = dev_handle;

	return handle->bus;
}


extern void acc_driver_spi_same70_register(wait_for_transfer_complete_t wait_function,
                                           transfer_complete_callback_t transfer_complete)
{
	acc_device_spi_create_func                  = acc_driver_spi_same70_create;
	acc_device_spi_destroy_func                 = acc_driver_spi_same70_destroy;
	acc_device_spi_transfer_func                = acc_driver_spi_same70_transfer;
	acc_device_spi_transfer_async_func          = acc_driver_spi_same70_transfer_async;
	acc_device_spi_get_bus_func                 = acc_driver_spi_same70_get_bus;

	wait_for_transfer_complete_func             = wait_function;
	transfer_complete_func                      = transfer_complete;
}
