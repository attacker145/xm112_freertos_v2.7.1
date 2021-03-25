// Copyright (c) Acconeer AB, 2019-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "acc_log_integration.h"

#include "acc_app_integration.h"
#include "acc_device_os.h"
#include "acc_hal_definitions.h"


#define LOG_FORMAT "%02u:%02u:%02u.%03u [%5u] (%c) (%s) %s\n"

#define LOG_BUFFER_MAX_SIZE 150


void acc_log(acc_log_level_t level, const char *module, const char *format, ...)
{
	char    log_buffer[LOG_BUFFER_MAX_SIZE];
	va_list ap;

	va_start(ap, format);

	int ret = vsnprintf(log_buffer, LOG_BUFFER_MAX_SIZE, format, ap);
	if (ret >= LOG_BUFFER_MAX_SIZE)
	{
		log_buffer[LOG_BUFFER_MAX_SIZE - 4] = '.';
		log_buffer[LOG_BUFFER_MAX_SIZE - 3] = '.';
		log_buffer[LOG_BUFFER_MAX_SIZE - 2] = '.';
		log_buffer[LOG_BUFFER_MAX_SIZE - 1] = 0;
	}

	acc_app_integration_thread_id_t thread_id = acc_os_get_thread_id();
	uint32_t                        time_ms = acc_os_get_time();
	char                            level_ch;

	unsigned int timestamp    = time_ms;
	unsigned int hours        = timestamp / 1000 / 60 / 60;
	unsigned int minutes      = timestamp / 1000 / 60 % 60;
	unsigned int seconds      = timestamp / 1000 % 60;
	unsigned int milliseconds = timestamp % 1000;

	level_ch = (level <= ACC_LOG_LEVEL_DEBUG) ? "EWIVD"[level] : '?';

	printf(LOG_FORMAT, hours, minutes, seconds, milliseconds, (unsigned int)thread_id, level_ch, module, log_buffer);

	fflush(stdout);

	va_end(ap);
}
