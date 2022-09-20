/**
  ******************************************************************************
  * @file    HAL_STSAFE-Axx_INTERNAL.h
  * @author  MCD Application team
  * @brief   Secure storage API, functions not exported to the outside world
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */


#ifndef HAL_STSAFEAxx_INTERNAL_H
#define HAL_STSAFEAxx_INTERNAL_H

#include "HAL_STSAFE-Axx_INTERNAL.h"
#include "HAL_STSAFE-Axx.h"
#include "Commonappli.h"
#include "stsafea_conf.h"
#include "stsafea_core.h"
#include "stsafea_interface_conf.h"

#define STSAFE_A_MALLOC  malloc
#define STSAFE_A_CALLOC  calloc
#define STSAFE_A_REALLOC realloc
#define STSAFE_A_FREE    free
#define SIZE_HOST_MAC_CIPHER_KEY 32

/* Public function prototypes -----------------------------------------------*/

/*!
 * \brief Update data through zone partition
 *
 * \param *handle                : Handle for STSAFEA_A interface (must be created before any usage)
 * \param zone                   : Zone number
 * \param in_Data                : Data to copy inside zone
 * \param offset                 : Offset in selected zone
 * \return OK if no error
 */

uint8_t UpdateZone(void *handle, uint8_t zone, StSafeA_LVBuffer_t in_Data, uint16_t offset);

/*!
 * \brief Check host keys presence and if not existing enable it in STSAFE-Axx and store value in flash
 *
 * \param *handle                : Handle for STSAFEA_A interface (must be created before any usage)
 * \param perso_type             : type of perso as explained below
 *
 *    type=0: use default keys and meanwhile set buf = NULL
 *    type=1: use random generated keys and meanwhile set buf = NULL
 *    type=2: use the keys customers prefer and meanwhile set buf = preferred key.
 *
 * \param buf                   : Host Mac and Cipher key value
 * \return OK if no error
 */

uint8_t Check_Host_Keys(void *handle, uint8_t perso_type, uint8_t *buf);

/* Private function prototypes -----------------------------------------------*/

/*!
 * \brief Write unsigned char array in STM32 flash at chosen address
 *
 * \param in : *MyArray : Array to write
 * \param in : SizeMyArray : zize of array
 * \param in : Addr : Address to write in STM32 flash
 * \return OK if no error
 */

uint8_t WriteUnsignedCharArrayToFlash(unsigned char *MyArray, uint8_t SizeMyArray, uint32_t Addr);

/*!
 * \brief challenge inside a Bytes Array
 *
 * \param in : *handle_se : Handle for STSAFE-Axx
 * \param in : size      : size of challenge to be generated
 * \param in : out_Data  : challenge
 * \return OK if no error
 */

uint8_t GenerateUnsignedChallenge(void *handle, uint32_t size, uint8_t *buf);

#endif /* HAL_STSAFEA100_INTERNAL_H */
