/**
  ******************************************************************************
  * @file    hashcheck.h
  * @brief   Header file of fw hash check module.
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
#ifndef __HASHCHECK_H
#define __HASHCHECK_H
#include "main.h"

#if defined (__ICCARM__)
#pragma section = "Firmware"
#define FW_END ((uint32_t)__section_end("Firmware"))
#elif defined (__CC_ARM)
extern uint32_t Image$$ER_IROM1$$Limit;
#define FW_END         ((uint32_t)&Image$$ER_IROM1$$Limit)
#elif  defined(__GNUC__)
extern uint32_t __FW_SECTION_END;
#define FW_END (uint32_t)((uint8_t*)& __FW_SECTION_END)
#else
#error "NOT SUPPORTED"
#endif

/* FW start from user flash base address */
#define FW_START_ADD (FLASH_BASE)
#define FW_SIZE (FW_END - FLASH_BASE)
#define FW_SIZE_PAGE_ALIGNED (FW_SIZE % FLASH_PAGE_SIZE == 0 ? \
                              FW_SIZE : \
                              (FW_SIZE/FLASH_PAGE_SIZE+1)*FLASH_PAGE_SIZE)

/* Hash data is placed at the last page of the flash*/
#define HASH_ADD (FW_START_ADD + FW_SIZE_PAGE_ALIGNED)
#define HASH_SIZE (32)

int32_t STM32_SHA256_HASH_DigestCompute(uint8_t* InputMessage, uint32_t InputMessageLength,
                                        uint8_t *MessageDigest, int32_t* MessageDigestLength);
void FW_Hash_Verify(void);
void Fatal_Error_Handler(void);

#endif /* __HASHCHECK_H */
