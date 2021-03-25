// Copyright (c) Acconeer AB, 2018
// All rights reserved
// This file is subject to the terms and conditions defined in the file
// 'LICENSES/license_acconeer.txt', (BSD 3-Clause License) which is part
// of this source code package.

#include "board_a111.h"

static struct _a111 a111;

void board_cfg_a111(void)
{
	const struct _a111_config a111_cfg = {
			.spi = {
					.bus = SPI_A111_BUS,
					.spi_dev = {
						.chip_select = SPI_A111_CS,
						.bitrate = SPI_A111_BITRATE,
						.delay = {       // See SPI Chip Select Register in SAM E70 Datasheet
						                 // page 1125
						                 // NOTE!! Don't change below without verifying with an
						                 //        Oscilloscope!
						                 // NOTE2!! The driver calculates the actual register
						                 //         values based in the current clock
						                 //         which means that 1 != 1 in the register!!
							.bs = 0,     // DLYBS, Delay Before SPCK (from NPCS falling edge)
							             // = SS setup time in A111 Datasheet rev 1.3, page 12,13.
							             // Min 1 ns. 0 -> 1/2 SPCK clock period, measured to
							             // ~14ns with oscilloscope
							.bct = 0,    // Maps to DLYBCT, Delay Between Consecutive Transfers
							             // The delay is always inserted after each transfer
							             // and before removing the chip select if needed.
							             // = SS hold time in A111 Datasheet.
							             // Min 2 ns.
							             // Measured to ~500 ns with a value of 0
						},
						.spi_mode = SPID_MODE_0,
					},
			},
			.sens_en = PIN_A111_SENS_EN,
			.ps_en = PIN_A111_PS_EN,
			.sens_int = PIN_A111_SENS_INT,
	};
	a111_configure(&a111, &a111_cfg);
}

struct _a111 *board_get_a111(void)
{
	return &a111;
}
