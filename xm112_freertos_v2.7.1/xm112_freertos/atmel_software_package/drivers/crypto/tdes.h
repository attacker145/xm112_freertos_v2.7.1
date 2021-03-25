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

#ifndef _TDES_
#define _TDES_

#ifdef CONFIG_HAVE_TDES

/*----------------------------------------------------------------------------
 *         Headers
 *----------------------------------------------------------------------------*/

#include "chip.h"

/*----------------------------------------------------------------------------
 *         Definitions
 *----------------------------------------------------------------------------*/

#define MODE_SINGLE_DES              0x00
#define MODE_TRIPLE_DES              0x01
#define MODE_XTEA                    0x02

/*----------------------------------------------------------------------------
 *         Exported functions
 *----------------------------------------------------------------------------*/

/**
 * \brief Starts Manual encryption/decryption process.
 */
extern void tdes_start(void);

/**
 * \brief Resets the TDES.
 * A software triggered hardware reset of the TDES interface is performed.
 */
extern void tdes_soft_reset(void);

/**
 * \brief Configures an TDES peripheral with the specified parameters.
 * \param mode  Desired value for the TDES_MR mode register (see the datasheet).
 */
extern void tdes_configure(uint32_t mode);

/**
 * \brief Set TDES start mode.
 *  \param mode  Start mode.
 */
extern void tdes_set_start_mode(uint32_t mode);

/**
 * \brief Set TDES algorithm mode.
 *  \param algo algorithm mode.
 */
extern void tdes_set_algo(uint32_t algo);

/**
 * \brief Set TDES operation mode.
 *  \param algo  operation mode.
 */
extern void tdes_set_op_mode(uint32_t mode);

/**
 * \brief Set TDES key mode.
 *  \param mode Key mode.
 */
extern void tdes_set_key_mode(uint32_t key_mode);

/**
 * \brief Set TDES processing mode
 *
 * \param encryt  encrypt true, decrypt otherwise.
 */
extern void tdes_encrypt_enable(bool encrypt);

/**
 * \brief Enables the selected interrupts sources on a TDES peripheral.
 * \param sources  Bitwise OR of selected interrupt sources.
 */
extern void tdes_enable_it(uint32_t sources);

/**
 * \brief Disables the selected interrupts sources on a TDES peripheral.
 * \param sources  Bitwise OR of selected interrupt sources.
 */
extern void tdes_disable_it(uint32_t sources);

/**
 * \brief Get the current status register of the given TDES peripheral.
 * \return  TDES status register.
 */
extern uint32_t tdes_get_status(void);

/**
 * \brief Set KEY 1/2/3.
 * \param key_word0  Key x, word 0
 * \param key_word1  Key x, word 1
 */
extern void tdes_write_key1(uint32_t key_word0, uint32_t key_word1);
extern void tdes_write_key2(uint32_t key_word0, uint32_t key_word1);
extern void tdes_write_key3(uint32_t key_word0, uint32_t key_word1);

/**
 * \brief Set the two 32-bit input data registers. Allows to set the 64-bit data
 * block used for encryption/decryption.
 * \param data0  Corresponds to the first word of the data to be processed.
 * \param data1  Corresponds to the last word of the data to be processed, skip 
	IDATA1 if data1 is NULL.
 */
extern void tdes_set_input(uint32_t* data0, uint32_t* data1);

/**
 * \brief Read from the two 32-bit data registers containing the 64-bit data
 * block which has been encrypted/decrypted.
 * \param data0  Points to the first word.
 * \param data1  Points to the second word, skip IODATA1 if data1 is NULL.
 */
extern void tdes_get_output(uint32_t *data0, uint32_t *data1);

/**
 * \brief Set the 64-bit initialization vector data block, which is used by
 * specific modes of operation as an additional initial input.
 * \param v0  Corresponds to the first word of the initialization vector.
 * \param v1  Corresponds to the second word of the initialization vector.
 */
extern void tdes_set_vector(uint32_t v0, uint32_t v1);

/**
 * \brief Set the 6-bit complete rounds.
 * \param rounds  Corresponds to rounds+1 complete round.
 */
extern void tdes_set_xtea_rounds(uint32_t rounds);

#endif /* CONFIG_HAVE_TDES */

#endif				/* #ifndef _TDES_ */
