/**
  ******************************************************************************
  * @file    loader_low_level_flash_ext.h
  * @author  MCD Application Team
  * @brief   This file contains definitions for Secure Firmware Update low level
  *          interface for external flash.
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
#ifndef LOADER_LOW_LEVEL_FLASH_EXT_H
#define LOADER_LOW_LEVEL_FLASH_EXT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "loader_low_level_flash.h"
#include "main.h"

/* Exported constants --------------------------------------------------------*/
#define EXTERNAL_FLASH_ADDRESS  0x90000000U

/* Exported functions ------------------------------------------------------- */
HAL_StatusTypeDef LOADER_LL_FLASH_EXT_Init(void);
HAL_StatusTypeDef LOADER_LL_FLASH_EXT_Erase_Size(uint8_t *pStart, uint32_t Length);
HAL_StatusTypeDef LOADER_LL_FLASH_EXT_Write(uint8_t *pDestination, const uint8_t *pSource, uint32_t Length);
HAL_StatusTypeDef LOADER_LL_FLASH_EXT_Read(uint8_t *pDestination, const uint8_t *pSource, uint32_t Length);

#ifdef __cplusplus
}
#endif

#endif /* SFU_LOW_LEVEL_FLASH_EXT_H */
