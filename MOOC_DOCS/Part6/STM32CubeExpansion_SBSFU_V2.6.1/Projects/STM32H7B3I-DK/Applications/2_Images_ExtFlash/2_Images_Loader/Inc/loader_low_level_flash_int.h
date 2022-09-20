/**
  ******************************************************************************
  * @file    loader_low_level_flash_int.h
  * @author  MCD Application Team
  * @brief   This file contains definitions for internal flash low level interface.
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
#ifndef LOADER_LOW_LEVEL_FLASH_INT_H
#define LOADER_LOW_LEVEL_FLASH_INT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/* STM32H7B3: The write access type is 128 bits */
typedef struct
{
  uint32_t tmp[4];
} LOADER_LL_FLASH_write_t;



/* Exported functions ------------------------------------------------------- */
HAL_StatusTypeDef LOADER_LL_FLASH_INT_Init(void);
HAL_StatusTypeDef LOADER_LL_FLASH_INT_Erase_Size(uint8_t *pStart, uint32_t Length);
HAL_StatusTypeDef LOADER_LL_FLASH_INT_Write(uint8_t *pDestination, const uint8_t *pSource, uint32_t Length);
HAL_StatusTypeDef LOADER_LL_FLASH_INT_Read(uint8_t *pDestination, const uint8_t *pSource, uint32_t Length);
uint32_t LOADER_LL_FLASH_INT_GetSector(uint32_t Addr);
uint32_t LOADER_LL_FLASH_INT_GetBank(uint32_t Addr);
void NMI_Handler(void);
void BusFault_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_LOW_LEVEL_FLASH_H */
