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

/**
 *  \file
 *
 *  \section Purpose
 *
 *  Interface for configuration the Analog-to-Digital Converter (ADC) peripheral.
 *
 *  \section Usage
 *
 *  -# Configurate the pins for ADC.
 *  -# Initialize the ADC with adc_initialize().
 *  -# Set ADC clock and timing with adc_set_clock() and adc_set_timing().
 *  -# Select the active channel using adc_enable_channel().
 *  -# Start the conversion with adc_start_conversion().
 *  -# Wait the end of the conversion by polling status with adc_get_status().
 *  -# Finally, get the converted data using adc_get_converted_data() or adc_get_last_converted_data().
 *
*/
#ifndef _ADC_
#define _ADC_

#ifdef CONFIG_HAVE_ADC
/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include <assert.h>
#include <stdint.h>

/*------------------------------------------------------------------------------
 *         Definitions
 *------------------------------------------------------------------------------*/

/* Max. ADC Clock Frequency (Hz) */
#define ADC_CLOCK_MAX   20000000

/* Max. normal ADC startup time (us) */
#define ADC_STARTUP_NORMAL_MAX     40
/* Max. fast ADC startup time (us) */
#define ADC_STARTUP_FAST_MAX       12

#define ADC_CHANNEL_NUM_IN_LCDR(d) (((d) & ADC_LCDR_CHNB_Msk) >> ADC_LCDR_CHNB_Pos)
#define ADC_LAST_DATA_IN_LCDR(d)  (((d) & ADC_LCDR_LDATA_Msk) >> ADC_LCDR_LDATA_Pos)

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------------------
 *         Exported functions
 *------------------------------------------------------------------------------*/

/**
 * \brief Returns the number of ADC channels
 */
extern uint32_t adc_get_num_channels(void);

/**
 * \brief Returns the resolution of ADC channels (bits)
 */
extern uint32_t adc_get_resolution(void);
	
/**
 * \brief Initialize the ADC controller
 */
extern void adc_initialize(void);

/**
 * \brief Set ADC clock.
 *
 * \param clk Desired ADC clock frequency.
 *
 * \return ADC clock
 */
extern uint32_t adc_set_clock(uint32_t clk);

/**
 * \brief Enable ADC interrupt sources
 *
 * \param mask bitmask of the sources to enable
 */
extern void adc_enable_it(uint32_t mask);

/**
 * \brief Disable ADC interrupt sources
 *
 * \param mask bitmask of the sources to disable
 */
extern void adc_disable_it(uint32_t mask);

/**
 * \brief Get ADC Interrupt Status
 *
 * \return the content of the ADC interrupt status register
 */
extern uint32_t adc_get_status(void);

/**
 * \brief Trigger ADC conversion (i.e. software trigger)
 */
extern void adc_start_conversion(void);

/**
 * \brief Enable ADC channel
 *
 * \param channel index of ADC channel to enable
 */
extern void adc_enable_channel(uint32_t channel);

/**
 * \brief Disable ADC channel
 *
 * \param channel index of ADC channel to disable
 */
extern void adc_disable_channel(uint32_t channel);

/**
 * \brief Set ADC timing.
 *
 * \param startup startup value
 * \param tracking tracking value
 * \param settling settling value
 */
extern void adc_set_timing(uint32_t startup, uint32_t tracking, uint32_t settling);

/**
 * Sets the trigger mode to following:
 * - \ref ADC_TRGR_TRGMOD_NO_TRIGGER
 * - \ref ADC_TRGR_TRGMOD_EXT_TRIG_RISE
 * - \ref ADC_TRGR_TRGMOD_EXT_TRIG_FALL
 * - \ref ADC_TRGR_TRGMOD_EXT_TRIG_ANY
 * - \ref ADC_TRGR_TRGMOD_PEN_TRIG
 * - \ref ADC_TRGR_TRGMOD_PERIOD_TRIG
 * - \ref ADC_TRGR_TRGMOD_CONTINUOUS
 * \param mode Trigger mode.
 */
extern void adc_set_trigger_mode(uint32_t mode);

/**
 * \brief Enable/Disable sleep mode.
 *
 * \param enable Enable/Disable sleep mode.
 */
extern void adc_set_sleep_mode(uint8_t enable);

extern void adc_set_fast_wakeup(uint8_t enable);

/**
 * \brief Enable/Disable seqnence mode.
 *
 * \param enable Enable/Disable seqnence mode.
 */
extern void adc_set_sequence_mode(uint8_t enable);

/**
 * \brief Set channel sequence.
 *
 * \param seq1 Sequence 1 ~ 8  channel number.
 * \param seq2 Sequence 9 ~ 16 channel number.
 */
extern void adc_set_sequence(uint32_t seq1, uint32_t seq2);

/**
 * \brief Set channel sequence by given channel list.
 *
 * \param channel_list Channel list.
 * \param len  Number of channels in list.
 */
extern void adc_set_sequence_by_list(uint8_t channel_list[],
				     uint8_t len);

/**
 * \brief Set "TAG" mode, show channel number in last data or not.
 *
 * \param enable Enable/Disable TAG value.
 */
extern void adc_set_tag_enable(uint8_t enable);

/**
 * Configure extended mode register
 * \param mode ADC extended mode.
 */
extern void adc_configure_ext_mode(uint32_t mode);

/**
 * \brief Set compare channel.
 *
 * \param channel channel number to be set,16 for all channels
 */
extern void adc_set_compare_channel(uint32_t channel);

/**
 * \brief Set compare mode.
 *
 * \param mode compare mode
 */
extern void adc_set_compare_mode(uint32_t mode);

/**
 * \brief Set comparsion window.
 *
 * \param window Comparison Window
 */extern void adc_set_comparison_window(uint32_t window);

/**
 * \brief Check if ADC configuration is right.
 *
 * \return 0 if check ok, others if not ok.
 */
extern uint8_t adc_check_configuration(void);

/**
 * \brief Return the Channel Converted Data
 *
 * \param channel channel to get converted value
 */
extern uint32_t adc_get_converted_data(uint32_t channel);

#ifdef CONFIG_HAVE_ADC_DIFF_INPUT
/**
 * \brief Enable differential input for the specified channel.
 *
 * \param channel ADC channel number.
 */
extern void adc_enable_channel_differential_input (uint32_t channel);

/**
 * \brief Disable differential input for the specified channel.
 *
 * \param channel ADC channel number.
 */
extern void adc_disable_channel_differential_input(uint32_t channel);
#endif /* CONFIG_HAVE_ADC_DIFF_INPUT */

#ifdef CONFIG_HAVE_ADC_INPUT_OFFSET
/**
 * \brief Enable analog signal offset for the specified channel.
 *
 * \param channel ADC channel number.
 */
extern void adc_enable_channel_input_offset (uint32_t channel);

/**
 * \brief Disable analog signal offset for the specified channel.
 *
 * \param channel ADC channel number.
 */
extern void adc_disable_channel_input_offset (uint32_t channel);
#endif /* CONFIG_HAVE_ADC_INPUT_OFFSET */

#ifdef CONFIG_HAVE_ADC_INPUT_GAIN
/**
 * \brief Configure input gain for the specified channel.
 *
 * \param channel ADC channel number.
 * \param gain Gain value for the input.
 */
extern void adc_set_channel_input_gain (uint32_t channel, uint32_t gain);
#endif /* CONFIG_HAVE_ADC_INPUT_GAIN */

/**
 * Sets the average of the touch screen ADC. The mode can be:
 * - \ref ADC_TSMR_TSAV_NO_FILTER (No filtering)
 * - \ref ADC_TSMR_TSAV_AVG2CONV (Average 2 conversions)
 * - \ref ADC_TSMR_TSAV_AVG4CONV (Average 4 conversions)
 * - \ref ADC_TSMR_TSAV_AVG8CONV (Average 8 conversions)
 * \param avg_2_conv Average mode for touch screen
 */
extern void adc_set_ts_average(uint32_t avg_2_conv);

/**
 * Return X measurement position value.
 */
extern uint32_t adc_get_ts_xposition(void);

/**
 * Return Y measurement position value.
 */
extern uint32_t adc_get_ts_yposition(void);

/**
 * Return Z measurement position value.
 */
extern uint32_t adc_get_ts_pressure(void);

/**
 * Sets the touchscreen pan debounce time.
 * \param time Debounce time in nS.
 */
extern void adc_set_ts_debounce(uint32_t time);

/**
 * Enable/Disable touch screen pen detection.
 * \param enable If true, pen detection is enabled;
 *               in normal mode otherwise.
 */
extern void adc_set_ts_pen_detect(uint8_t enable);

/**
 * Sets the ADC startup time.
 * \param startup  Startup time in uS.
 */
extern void adc_set_startup_time(uint32_t startup);

/**
 * Set ADC tracking time
 * \param dwNs  Tracking time in nS.
 */
extern void adc_set_tracking_time(uint32_t dwNs);

/**
 * Sets the trigger period.
 * \param period Trigger period in uS.
 */
extern void adc_set_trigger_period(uint32_t period);

/**
 * Sets the operation mode of the touch screen ADC. The mode can be:
 * - \ref ADC_TSMR_TSMODE_NONE (TSADC off)
 * - \ref ADC_TSMR_TSMODE_4_WIRE_NO_PM
 * - \ref ADC_TSMR_TSMODE_4_WIRE (CH 0~3 used)
 * - \ref ADC_TSMR_TSMODE_5_WIRE (CH 0~4 used)
 * \param mode Desired mode
 */
extern void adc_set_ts_mode(uint32_t mode);

/**
 * Start screen calibration (VDD/GND measurement)
 */
extern void adc_ts_calibration(void);

/**
 * \brief Set ADC trigger.
 *
 * \param trigger Trigger selection
 */
extern void adc_set_trigger(uint32_t trigger);

#ifdef CONFIG_HAVE_ADC_LOW_RES
/**
 * \brief Enable/Disable low resolution.
 *
 * \param enable Enable/Disable low resolution.
 */
extern void adc_set_low_resolution(uint8_t enable);
#endif /* CONFIG_HAVE_ADC_LOW_RES */

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_HAVE_ADC */
#endif /* _ADC_ */
