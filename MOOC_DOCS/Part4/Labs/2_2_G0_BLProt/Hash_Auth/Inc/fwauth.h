/**
  ******************************************************************************
  * @file    fwauth.h
  * @brief   Header file of security config check module.
  *          This file provides set of firmware functions to manage Com
  *          functionalities.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
#ifndef __FW_AUTH_H
#define __FW_AUTH_H
#include "main.h"
#include "jumper.h"

#define FW_HASH_LEN              32 /* SHA256*/
#define FW_META_SIG_LEN         64 /* ECDSA P256*/
#define FW_MAGIC                'FWMA'
#define FW_META_DATA_ADD        (APP_META_DATA_ADD)
#define FW_ADD                  (APP_REGION_ROM_START)

/* 
 * FW meta data for verification 
 * Totoal size 128 bytes, with 20 reserved bytes not used 
 */
typedef struct {
  uint32_t FWMagic;               /*!< FW Magic 'FWMA'*/
  uint32_t FwSize;                 /*!< Firmware size (bytes)*/
  uint32_t FwVersion;              /*!< Firmware version*/
  uint8_t  FwTag[FW_HASH_LEN];      /*!< Firmware Tag*/
  uint8_t  Reserved[84];          /*!< Reserved for future use: 84 extra bytes to have a header size (to be signed) of 128 bytes */
  uint8_t  MetaTag[FW_HASH_LEN];  /*!< Signature of the header message (before MetaTag)*/
  uint8_t  MetaSig[FW_META_SIG_LEN];  /*!< Signature of the header message (before MetaTag)*/
}FW_Meta_t;



/* Hash data is placed at the last page of the flash*/

int32_t FW_Verify(void);

#endif /* __FW_AUTH_H */
