/**
  ******************************************************************************
  * @file    sfu_low_level_security_rss.h
  * @author  MCD Application Team
  * @brief   This file contains definitions for Secure Firmware Update security
  *          RSS low level interface.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file in
  * the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SFU_LOW_LEVEL_SECURITY_RSS_H
#define SFU_LOW_LEVEL_SECURITY_RSS_H

#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "sfu_fwimg_regions.h"
#include "sfu_def.h"
#include "sfu_low_level_security.h"

/* Exported types ------------------------------------------------------------*/
/**
  * @brief  Secure User Memory area
  * One secure area of configurable size can be set in each bank.
  * The size of each area can be set from 512 bytes to full bank with a granularity of 256 bytes.
  */
typedef struct
{
  uint32_t sizeInBytes;           /*!< pass 0 for an empty secure area */
  uint32_t startAddress;          /*!< pass NULL for an empty secure area */
  uint32_t removeDuringBankErase; /*!< if 0, keep area during bank/mass erase. else area will be removed */
} SFU_LL_RSS_SecureArea_t;

/**
  * @brief API to handle the Secure User Memory Areas
  * RSS software cannot be accessed (read, write, execute and debug) when the STM32H7x3
  * operates in Standard mode.
  */
typedef struct
{
  /**
    * This service is used to exit from secure user software and jump to user main application.
    * There is no system reset triggered by this service
    */
  void (*exitSecureArea)(uint32_t vectors);
  /**
    * This service sets Secure user area boundaries.
    * This service can be used only when a secure area is set for the first time.
    * A system reset is triggered after service completion.
    */
  void (*resetAndInitializeSecureAreas)(uint32_t nbAreas, SFU_LL_RSS_SecureArea_t *areas); /*!< nbAreas=1 or 2;
                                                                                                1 per bank */

} SFU_LL_RSS_API_Table_t;

/* Exported constants --------------------------------------------------------*/
#define SFU_LL_RSS_NB_SECURE_AREAS (2U) /*!< One secure area of configurable size can be set in each bank. */

/**
  * @brief RSS code
  * When the STM32H7x3 is configured in Secure access mode, the RSS software is executed
  * only once at reset. When RSS software execution is complete, it cannot be accessed
  * anymore (no other code can jump directly to RSS).
  * Any RSS routine is called by performing a reset.
  */
#define SFU_LL_RSS ((SFU_LL_RSS_API_Table_t*)0x1FF09514)

#ifdef __cplusplus
}
#endif

#endif /* SFU_LOW_LEVEL_SECURITY_RSS_H */
