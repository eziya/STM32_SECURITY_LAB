/**
  ******************************************************************************
  * @file    pairing.h
  * @author  MCD Application Team
  * @brief   PCROP functions
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

#ifndef PAIRING_H
#define PAIRING_H

#include "flash_if.h"



#ifdef __cplusplus

extern "C" {

#endif /* __cplusplus */

#define PCROP_BASE_ENC (PCROP_ADDR + 1)

/* Structure will contains STSAFE-MAC-KEY */

typedef struct
{
  uint8_t p0;
  uint8_t p1;
  uint8_t p2;
  uint8_t p3;

} key_mac_t;

/* Structure will contains STSAFE-ENCRYPTION-KEY */

typedef struct
{
  uint8_t p0;
  uint8_t p1;
  uint8_t p2;
  uint8_t p3;

} key_enc_t;

/**
  * @brief  Execute code already stored in flash that allows to get aHostCipherKey & aHostMacKey
  * @param  out : *key array which contains key value
  * @retval void
  */

extern void (*PCROP_Write_Mac_Enc_Keys_To_Registers)(unsigned char *key);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PAIRING_H */
