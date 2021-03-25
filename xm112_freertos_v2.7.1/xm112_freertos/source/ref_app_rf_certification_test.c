// Copyright (c) Acconeer AB, 2019-2020
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "acc_driver_hal.h"
#include "acc_hal_definitions.h"
#include "acc_rf_certification_test.h"
#include "acc_rss.h"
#include "acc_version.h"


static bool acc_ref_app_rf_certification_test(void);


int main(void)
{
	if (!acc_driver_hal_init())
	{
		return EXIT_FAILURE;
	}

	if (!acc_ref_app_rf_certification_test())
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}


bool acc_ref_app_rf_certification_test(void)
{
	printf("Acconeer software version %s\n", acc_version_get());

	const acc_hal_t *hal = acc_driver_hal_get_implementation();

	if (!acc_rss_activate(hal))
	{
		printf("Failed to activate RSS\n");
		return false;
	}

	bool     tx_disable = false;
	uint32_t iterations = 0; // 0 means 'run forever'

	bool success = acc_rf_certification_test(tx_disable, iterations);

	acc_rss_deactivate();

	return success;
}
