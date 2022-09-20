/**
  ******************************************************************************
  * @file    Pairing.c
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

#include "pairing.h"
#include "Commonappli.h"


/**
  * @brief  Execute code already stored in flash that allows to get aHostCipherKey & aHostMacKey
  * @param  out : *key array which contains key value
  * @retval void
  */

void (*PCROP_Write_Mac_Enc_Keys_To_Registers)(unsigned char *key) = (void (*)(unsigned char *))(PCROP_BASE_ENC);
