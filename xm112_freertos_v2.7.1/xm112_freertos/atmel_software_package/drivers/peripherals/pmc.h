/* ----------------------------------------------------------------------------
 *         SAM Software Package License
 * ----------------------------------------------------------------------------
 * Copyright (c) 2015, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

#ifndef _PMC_H_
#define _PMC_H_

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "chip.h"

#include <stdint.h>

/*----------------------------------------------------------------------------
 *        Types
 *----------------------------------------------------------------------------*/

struct _pmc_plla_cfg {
	/** PLLA MUL value */
	uint32_t mul;

	/** PLLA DIV value */
	uint32_t div;

	/** PLLA COUNT value (number of slow clock cycles before the PLLA is locked) */
	uint32_t count;

#ifdef CONFIG_HAVE_PMC_PLLA_CHARGE_PUMP
	/** PLLA ICP value */
	uint32_t icp;
#endif
};

struct pck_mck_cfg {
	/** PLL A, SLCK, MAIN, UPLL */
	uint32_t pck_input;

	/** RC12M (false) or EXT12M (true) */
	bool ext12m;

	/** Use BYPASS */
	bool ext_bypass;

	/** RC32K (false) or EXT32K (true) */
	bool ext32k;

	/** PLLA configuration */
	struct _pmc_plla_cfg plla;

	/** Master/Processor Clock Prescaler */
	uint32_t pck_pres;

	/** Master Clock Division after Prescaler divider */
	uint32_t mck_div;

#ifdef CONFIG_HAVE_PMC_PLLADIV2
	/** PLLA DIV value by 2 */
	bool plla_div2;
#endif

#ifdef CONFIG_HAVE_PMC_UPLLDIV2
	/** UPLL DIV value by 2 */
	bool upll_div2;
#endif

#ifdef CONFIG_HAVE_PMC_H32MXDIV
	/** true if the AHB 32-bit Matrix frequency is equal to the AHB 64-bit Matrix frequency divided by 2 */
	bool h32mx_div2;
#endif
};

struct	_pmc_periph_cfg{
#ifdef CONFIG_HAVE_PMC_PERIPH_DIV
	/** Peripheral clock divisor (0 means use lowest divider possible) */
	uint32_t div;
#endif

#ifdef CONFIG_HAVE_PMC_GENERATED_CLOCKS
	struct {
		/** gck source selection: SLOW, MAIN, PLLA, UPLL, MCK or AUDIO */
		uint32_t css;
		/** gck division ratio (0 means disable, n >= 1 divide by n) */
		uint32_t div;
	} gck;
#endif

#if defined(__ICCARM__) && \
    !defined(CONFIG_HAVE_PMC_PERIPH_DIV) && \
    !defined(CONFIG_HAVE_PMC_GENERATED_CLOCKS)
	uint8_t dummy; /* IAR compiler doesn't like empty structs */
#endif
};

/**
 * \brief System clock identifiers, used for pmc_{enable,disable}_system_clock
 */
enum _pmc_system_clock {
	PMC_SYSTEM_CLOCK_DDR,
	PMC_SYSTEM_CLOCK_LCD,
	PMC_SYSTEM_CLOCK_SMD,
	PMC_SYSTEM_CLOCK_UHP,
	PMC_SYSTEM_CLOCK_UDP,
	PMC_SYSTEM_CLOCK_PCK0,
	PMC_SYSTEM_CLOCK_PCK1,
	PMC_SYSTEM_CLOCK_PCK2,
	PMC_SYSTEM_CLOCK_PCK3,
	PMC_SYSTEM_CLOCK_PCK4,
	PMC_SYSTEM_CLOCK_PCK5,
	PMC_SYSTEM_CLOCK_PCK6,
	PMC_SYSTEM_CLOCK_ISC,
};

#ifdef CONFIG_HAVE_PMC_AUDIO_CLOCK
/**
 * \brief Configuration data for Audio clock
 *
 * AUDIOPLLCK = BOARD_EXT_OSC * (nd + 1 + (fracr / 2^22)) / (qdpmc + 1)
 * AUDIOPINCLK = BOARD_EXT_OSC * (nc + 1 + (fracr / 2^22)) / (div * qdaudio)
 */
struct _pmc_audio_cfg {
	uint32_t nd;
	uint32_t fracr;
	uint32_t qdpmc;
	uint32_t div;
	uint32_t qdaudio;
};
#endif /* CONFIG_HAVE_PMC_AUDIO_CLOCK */

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Set the external oscillator frequency (in Hz)
 * \return Main oscillator frequency (0 if not detected)
 */
extern uint32_t pmc_set_main_oscillator_freq(uint32_t freq);

/**
 * \brief Get the external oscillator frequency (in Hz)
 * \return Main oscillator frequency (0 if not detected)
 */
extern uint32_t pmc_get_main_oscillator_freq(void);

/**
 * \brief Configure PCK and MCK with custom setting
 */
extern void pmc_set_custom_pck_mck(const struct pck_mck_cfg *cfg);

/**
 * \brief Get the configured frequency of the master clock
 * \return master clock frequency in Hz
 */
extern uint32_t pmc_get_master_clock(void);

/**
 * \brief Get the configured frequency of the slow clock
 * \return slow clock frequency in Hz
 */
extern uint32_t pmc_get_slow_clock(void);

/**
 * \brief Get the configured frequency of the main clock
 * \return main clock frequency in Hz
 */
extern uint32_t pmc_get_main_clock(void);

/**
 * \brief Get the configured frequency of the PLLA clock
 * \return PLLA clock frequency in Hz
 */
extern uint32_t pmc_get_plla_clock(void);

/**
 * \brief Get the configured frequency of the processor clock
 * \return processor clock frequency in Hz
 */
extern uint32_t pmc_get_processor_clock(void);

/**
 * \brief Select external 32K crystal.
 */
extern void pmc_select_external_crystal(void);

/**
 * \brief Select internal 32K crystal.
 */
extern void pmc_select_internal_crystal(void);

/**
 * \brief Select external 12M OSC.
 * \return 0 on success, negative error otherwise
 */
extern int pmc_select_external_osc(bool bypass);

/**
 * \brief Enable external 12M OSC.
 * \return 0 on success, negative error otherwise
 */
extern int pmc_enable_external_osc(bool bypass);

/**
 * \brief Disable external 12M OSC.
 */
extern void pmc_disable_external_osc(void);

/**
 * \brief Select internal 12M OSC.
 */
extern void pmc_select_internal_osc(void);

/**
 * \brief Enable internal 12M OSC.
 */
extern void pmc_enable_internal_osc(void);

/**
 * \brief Disable internal 12M OSC.
 */
extern void pmc_disable_internal_osc(void);

/**
 * \brief Switch MCK to new source.
 */
extern void pmc_switch_mck_to_new_source(uint32_t mckr_css);

/**
 * \brief Switch PMC from MCK to PLL clock.
 */
extern void pmc_switch_mck_to_pll(void);

/**
 * \brief Switch PMC from MCK to UPLL clock.
 */
extern void pmc_switch_mck_to_upll(void);

/**
 * \brief Switch PMC from MCK to main clock.
 */
extern void pmc_switch_mck_to_main(void);

/**
 * \brief Switch PMC from MCK to slow clock.
 */
extern void pmc_switch_mck_to_slck(void);

/**
 * \brief Configure PLL Registers.
 * \param plla PLLA configuration.
 */
extern void pmc_configure_plla(const struct _pmc_plla_cfg* plla);

/**
 * \brief Configure MCK Prescaler.
 * \param prescaler prescaler value.
 */
extern void pmc_set_mck_prescaler(uint32_t prescaler);

/**
 * \brief Configure MCK Divider.
 * \param divider divider value.
 */
extern void pmc_set_mck_divider(uint32_t divider);

#ifdef CONFIG_HAVE_PMC_H32MXDIV
/**
 * \brief Configure MCK H32MXDIV.
 * \param div2 true if the AHB 32-bit Matrix frequency is equal to the AHB 64-bit Matrix frequency divided by 2
 */
extern void pmc_set_mck_h32mxdiv(bool div2);
#endif /* CONFIG_HAVE_PMC_H32MXDIV */

#ifdef CONFIG_HAVE_PMC_PLLADIV2
/**
 * \brief Configure MCK PLLA divider.
 * \param div2 true if PLLA should be divided by 2, false otherwise
 */
extern void pmc_set_mck_plladiv2(bool div2);
#endif /* CONFIG_HAVE_PMC_PLLADIV2 */

#ifdef CONFIG_HAVE_PMC_UPLLDIV2
/**
 * \brief Configure MCK UPLL divider.
 * \param div2 true if UPLL should be divided by 2, false otherwise
 */
extern void pmc_set_mck_uplldiv2(bool div2);
#endif /* CONFIG_HAVE_PMC_UPLLDIV2 */

/**
 * \brief Disable PLLA Register.
 */
extern void pmc_disable_plla(void);

/**
 * \brief Check whether the device supports a given system clock
 * \param clock system clock to check
 */
extern bool pmc_has_system_clock(enum _pmc_system_clock clock);

/**
 * \brief Enables a system clock
 * \param clock system clock to enable
 */
extern void pmc_enable_system_clock(enum _pmc_system_clock clock);

/**
 * \brief Disables a system clock
 * \param clock system clock to disable
 */
extern void pmc_disable_system_clock(enum _pmc_system_clock clock);

/**
 * \brief Get the status of a system clock
 * \param clock system clock to check
 */
extern bool pmc_is_system_clock_enabled(enum _pmc_system_clock clock);

#ifdef CONFIG_HAVE_PMC_FAST_STARTUP
/**
 * \brief Set up fast startup mode
 * \param source and low power mode
 */
extern void pmc_set_fast_startup_mode(uint32_t startup_mode);

/**
 * \brief Set up fast startup polarity
 * \param level
 */
extern void pmc_set_fast_startup_polarity(uint32_t high_level,
	uint32_t low_level);
#endif /* CONFIG_HAVE_PMC_FAST_STARTUP */

/**
 * \brief Config the peripheral clock. The peripheral ID is used
 * to identify which peripheral is targeted.
 *
 * \param id  Peripheral ID (ID_xxx).
 * \param cfg pcr configuration
 * \param enable enable peripheral clk
 */
extern void pmc_configure_peripheral(uint32_t id, const struct _pmc_periph_cfg* cfg, bool enable);

/**
 * \brief Enables the clock of a peripheral. The peripheral ID is used
 * to identify which peripheral is targeted.
 *
 * \param id  Peripheral ID (ID_xxx).
 */
extern void pmc_enable_peripheral(uint32_t id);

/**
 * \brief Disables the clock of a peripheral. The peripheral ID is used
 * to identify which peripheral is targeted.
 *
 * \param id  Peripheral ID (ID_xxx).
 */
extern void pmc_disable_peripheral(uint32_t id);

/**
 * \brief Get Peripheral Status for the given peripheral ID.
 *
 * \param id  Peripheral ID (ID_xxx).
 */
extern bool pmc_is_peripheral_enabled(uint32_t id);

/**
 * \brief Get current frequency clock for the given peripheral ID.
 *
 * \param id  Peripheral ID (ID_xxx).
 */
extern uint32_t pmc_get_peripheral_clock(uint32_t id);

/**
 * \brief Disable clocks for all peripherals
 */
extern void pmc_disable_all_peripherals(void);

/**
 * \brief Configure programmable clock x (PCKx) with the given master clock
 * source and clock prescaler
 * \param index index of the PCK
 * \param clock_source clock source selection (one of the PMC_PCK_CSS_xxx_CLK
 * constants)
 * \param prescaler prescaler
 */
extern void pmc_configure_pck(uint32_t index, uint32_t clock_source, uint32_t prescaler);

/**
 * \brief Enable programmable clock x (PCKx)
 * \param index index of the PCK
 */
extern void pmc_enable_pck(uint32_t index);

/**
 * \brief Disable programmable clock x (PCKx)
 * \param index index of the PCK
 */
extern void pmc_disable_pck(uint32_t index);

/**
 * \brief Get the frequency of the programmable clock x (PCKx)
 * \param index index of the PCK
 * \return PCK frequency in Hz
 */
extern uint32_t pmc_get_pck_clock(uint32_t index);

/**
 * \brief Enable the UPLL clock
 */
extern void pmc_enable_upll_clock(void);

/**
 * \brief Disable the UPLL clock
 */
extern void pmc_disable_upll_clock(void);

/**
 * \brief Get the UPLL clock status
 * \return true if the UPLL clock is enabled, false otherwise
 */
extern bool pmc_is_upll_clock_enabled(void);

/**
 * \brief Get the frequency of the UPLL clock
 * \return UPLL clock frequency in Hz
 */
extern uint32_t pmc_get_upll_clock(void);

#ifdef CONFIG_HAVE_PMC_UPLL_BIAS
/**
 * \brief Enable the UPLL clock bias
 */
extern void pmc_enable_upll_bias(void);

/**
 * \brief Disable the UPLL clock bias
 */
extern void pmc_disable_upll_bias(void);
#endif /* CONFIG_HAVE_PMC_UPLL_BIAS */

/**
 * \brief Get the configured frequency of the UTMI clock trim
 * \return UTMI clock trim frequency
 */
extern uint32_t pmc_get_utmi_clock_trim(void);

#ifdef CONFIG_HAVE_PMC_GENERATED_CLOCKS
/**
 * \brief Configure the generated clock (GCK) for the given peripheral with the
 * given master clock source and clock prescaler
 * \param id Peripheral ID (ID_xxx)
 * \param clock_source Clock source selection (one of the PMC_PCR_GCKCSS_xxx_CLK constants)
 * \param div Generated Clock Division Ratio (selected clock is divided by div)
 */
extern void pmc_configure_gck(uint32_t id, uint32_t clock_source, uint32_t div);

/**
 * \brief Enable generated clock for the given peripheral
 * \param id Peripheral ID (ID_xxx)
 */
extern void pmc_enable_gck(uint32_t id);

/**
 * \brief Disable generated clock for the given peripheral
 * \param id Peripheral ID (ID_xxx)
 */
extern void pmc_disable_gck(uint32_t id);

/**
 * \brief Get the frequency of the generated clock (GCK) for the given
 * peripheral
 * \param id Peripheral ID (ID_xxx)
 * \return GCK Frequency in Hz
 */
extern uint32_t pmc_get_gck_clock(uint32_t id);

/**
 * \brief Get Gck Status for the given peripheral ID.
 *
 * \param id  Peripheral ID (ID_xxx).
 */
extern bool pmc_is_gck_enabled(uint32_t id);
#endif /* CONFIG_HAVE_PMC_GENERATED_CLOCKS */

#ifdef CONFIG_HAVE_PMC_AUDIO_CLOCK
/**
 * \brief Configure the audio clock
 */
extern void pmc_configure_audio(const struct _pmc_audio_cfg *cfg);

/**
 * \brief Enable audio clocks
 * \param pmc_clock if true AUDIOPLLCK is sent to the PMC
 * \param pad_clock if true the external audio pin is driven by AUDIOPINCLK, if
 * false the audio pin is driven low
 */
extern void pmc_enable_audio(bool pmc_clock, bool pad_clock);

/**
 * \brief Disable audio clocks
 */
extern void pmc_disable_audio(void);

/**
 * \brief Get the frequency of the audio PMC clock
 * \return Audio PMC Frequency in Hz
 */
extern uint32_t pmc_get_audio_pmc_clock(void);

/**
 * \brief Get the frequency of the audio pad clock
 * \return Audio pad Frequency in Hz
 */
extern uint32_t pmc_get_audio_pad_clock(void);
#endif /* CONFIG_HAVE_PMC_AUDIO_CLOCK */

#ifdef __cplusplus
}
#endif
#endif /* #ifndef _PMC_H_ */
