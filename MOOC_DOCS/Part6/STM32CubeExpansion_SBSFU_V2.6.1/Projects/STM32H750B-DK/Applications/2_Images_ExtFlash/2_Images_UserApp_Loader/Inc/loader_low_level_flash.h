/**
  ******************************************************************************
  * @file    loader_low_level_flash.h
  * @author  MCD Application Team
  * @brief   This file contains definitions for Secure Firmware Update flash
  *          low level interface.
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
#ifndef LOADER_LOW_LEVEL_FLASH_H
#define LOADER_LOW_LEVEL_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Includes using previous definition -------------------------*/
#include "loader_low_level_flash_int.h"
#include "loader_low_level_flash_ext.h"

/* Exported constants --------------------------------------------------------*/
#define INVALID_SECTOR 0xFFFFFFFFU
#define INVALID_PAGE   0xFFFFFFFFU
#define INVALID_BANK   0xFFFFFFFFU

/* Exported functions ------------------------------------------------------- */
HAL_StatusTypeDef LOADER_LL_FLASH_Init(void);
HAL_StatusTypeDef LOADER_LL_FLASH_Erase_Size(uint8_t *pStart, uint32_t Length);
HAL_StatusTypeDef LOADER_LL_FLASH_Write(uint8_t *pDestination, const uint8_t *pSource, uint32_t Length);
HAL_StatusTypeDef LOADER_LL_FLASH_Read(uint8_t *pDestination, const uint8_t *pSource, uint32_t Length);

uint32_t LOADER_LL_FLASH_GetSector(uint32_t Addr);

#ifdef __cplusplus
}
#endif

#endif /* SFU_LOW_LEVEL_FLASH_H */
