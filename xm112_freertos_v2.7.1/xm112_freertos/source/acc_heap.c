// Copyright (c) Acconeer AB, 2016-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#include <stdint.h>
#include <sys/types.h>

#include "FreeRTOS.h"


/**
 * @brief Allocate heap to be used by FreeRTOS
 */
static uint8_t primary_heap[configTOTAL_HEAP_SIZE];
HeapRegion_t xHeapRegions[] = {
	{ primary_heap, sizeof(primary_heap) },
	{ NULL, 0 }
};
