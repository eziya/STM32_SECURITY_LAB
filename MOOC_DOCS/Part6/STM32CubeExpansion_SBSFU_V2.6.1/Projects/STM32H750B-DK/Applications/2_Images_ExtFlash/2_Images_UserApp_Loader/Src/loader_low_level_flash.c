/**
  ******************************************************************************
  * @file    loader_low_level_flash.c
  * @author  MCD Application Team
  * @brief   SFU Flash Low Level Interface module
  *          This file provides set of firmware functions to manage SFU flash
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

/* Includes ------------------------------------------------------------------*/
#include "loader_low_level_flash.h"

/* Functions Definition ------------------------------------------------------*/
/**
  * @brief  Initialize internal and external flash interface (OSPI/QSPI)
  * @param  none
  * @retval HAL_OK if successful, HAL_ERROR otherwise.
  */
HAL_StatusTypeDef LOADER_LL_FLASH_Init(void)
{
  HAL_StatusTypeDef e_ret_status;

  e_ret_status = LOADER_LL_FLASH_INT_Init();
  if (e_ret_status == HAL_OK)
  {
    e_ret_status = LOADER_LL_FLASH_EXT_Init();
  }
  return e_ret_status;
}

/**
  * @brief  Depending on start address, this function will call internal or external (OSPI/QSPI) flash driver
  * @param  pStart: flash address to be erased
  * @param  Length: number of bytes
  * @retval HAL_OK if successful, HAL_ERROR otherwise.
  */
HAL_StatusTypeDef LOADER_LL_FLASH_Erase_Size(uint8_t *pStart, uint32_t Length)
{

  /* Check Flash start address */
  if ((uint32_t) pStart < EXTERNAL_FLASH_ADDRESS)
  {
    return LOADER_LL_FLASH_INT_Erase_Size(pStart, Length);
  }
  else
  {
    return LOADER_LL_FLASH_EXT_Erase_Size(pStart, Length);
  }
}

/**
  * @brief  Depending on destination address, this function will call internal or external (OSPI/QSPI) flash driver
  * @param  pDestination: flash address to write
  * @param  pSource: pointer on buffer with data to write
  * @param  Length: number of bytes
  * @retval HAL_OK if successful, HAL_ERROR otherwise.
  */
HAL_StatusTypeDef LOADER_LL_FLASH_Write(uint8_t  *pDestination, const uint8_t *pSource, uint32_t Length)
{
  /* Check Flash destination address */
  if ((uint32_t) pDestination < EXTERNAL_FLASH_ADDRESS)
  {
    return LOADER_LL_FLASH_INT_Write(pDestination, pSource, Length);
  }
  else
  {
    return LOADER_LL_FLASH_EXT_Write(pDestination, pSource, Length);
  }
}

/**
  * @brief  Depending on source address, this function will call internal or external (OSPI/QSPI) flash driver
  * @param  pDestination: pointer on buffer to store data
  * @param  pSource: flash address to read
  * @param  Length: number of bytes
  * @retval HAL_OK if successful, HAL_ERROR otherwise.
  */
HAL_StatusTypeDef LOADER_LL_FLASH_Read(uint8_t *pDestination, const uint8_t *pSource, uint32_t Length)
{
  /* Check Flash source address */
  if ((uint32_t) pSource < EXTERNAL_FLASH_ADDRESS)
  {
    return LOADER_LL_FLASH_INT_Read(pDestination, pSource, Length);
  }
  else
  {
    return LOADER_LL_FLASH_EXT_Read(pDestination, pSource, Length);
  }
}

/**
  * @brief  Gets the sector of a given address
  * @param  Addr: flash address
  * @retval The sector of a given address
  */
uint32_t LOADER_LL_FLASH_GetSector(uint32_t Addr)
{
  /* Check Flash address */
  if (Addr < EXTERNAL_FLASH_ADDRESS)
  {
    return LOADER_LL_FLASH_INT_GetSector(Addr);
  }
  else
  {
    return INVALID_SECTOR;                       /* Sector number is not used in SBSFU application for external flash */
  }
}

