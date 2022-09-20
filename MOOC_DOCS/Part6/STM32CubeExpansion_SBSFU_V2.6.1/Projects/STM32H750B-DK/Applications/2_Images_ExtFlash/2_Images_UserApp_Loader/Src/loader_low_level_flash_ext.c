/**
  ******************************************************************************
  * @file    loader_low_level_flash_ext.c
  * @author  MCD Application Team
  * @brief   SFU Flash Low Level Interface module
  *          This file provides set of firmware functions to manage SFU external
  *          flash low level interface.
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
#include "main.h"
#include "loader_low_level_flash.h"
#include "stm32h750b_discovery_qspi.h"

/* Private defines -----------------------------------------------------------*/
#define ERASE_BLOC_SIZE           0x2000U            /*!< 8 Kbytes */

/* Functions Definition ------------------------------------------------------*/
/**
  * @brief  This function initialize QSPI interface
  * @param  none
  * @retval HAL_OK if successful, HAL_ERROR otherwise.
  */
HAL_StatusTypeDef LOADER_LL_FLASH_EXT_Init(void)
{
  if (BSP_QSPI_Init() != QSPI_OK)
  {
    return HAL_ERROR;
  }
  else
  {
    return HAL_OK;
  }
}

/**
  * @brief  This function does an erase of n (depends on Length) pages in external OSPI flash
  * @param  pStart: flash address to be erased
  * @param  Length: number of bytes
  * @retval HAL_OK if successful, HAL_ERROR otherwise.
  */
HAL_StatusTypeDef LOADER_LL_FLASH_EXT_Erase_Size(uint8_t *pStart, uint32_t Length)
{
  HAL_StatusTypeDef e_ret_status = HAL_OK;
  uint32_t i;
  uint32_t address = (uint32_t) pStart;

  /* flash address to erase is the offset from begin of external flash */
  address -= EXTERNAL_FLASH_ADDRESS;

  /* Loop on 64KBytes block */
  for (i = 0U ; i < (((Length - 1U) / ERASE_BLOC_SIZE) + 1U); i++)
  {
    if (BSP_QSPI_Erase_Block(address) != QSPI_OK)
    {
      e_ret_status = HAL_ERROR;
    }

    /* next 64KBytes block */
    address += ERASE_BLOC_SIZE;

    /* reload watch dog */
    WRITE_REG(IWDG1->KR, IWDG_KEY_RELOAD);
  }
  return e_ret_status;
}

/**
  * @brief  This function writes a data buffer in external QSPI flash.
  * @param  pDestination: flash address to write
  * @param  pSource: pointer on buffer with data to write
  * @param  Length: number of bytes
  * @retval HAL_OK if successful, HAL_ERROR otherwise.
  */
HAL_StatusTypeDef LOADER_LL_FLASH_EXT_Write(uint8_t  *pDestination, const uint8_t *pSource, uint32_t Length)
{
  uint32_t destination = (uint32_t) pDestination;

  /* Do nothing if Length equal to 0 */
  if (Length == 0U)
  {
    return HAL_OK;
  }

  /* flash address to write is the offset from begin of external flash */
  if (BSP_QSPI_Write((uint8_t *) pSource, destination - EXTERNAL_FLASH_ADDRESS, Length) != QSPI_OK)
  {
    return HAL_ERROR;
  }
  else
  {
    return HAL_OK;
  }
}

/**
  * @brief  This function reads external QSPI flash
  * @param  pDestination: pointer on buffer to store data
  * @param  pSource: flash address to read
  * @param  Length: number of bytes
  * @retval HAL_OK if successful, HAL_ERROR otherwise.
  */
HAL_StatusTypeDef LOADER_LL_FLASH_EXT_Read(uint8_t *pDestination, const uint8_t *pSource, uint32_t Length)
{
  uint32_t source = (uint32_t) pSource;

  /* ensure previous operation is finished (erase/write) : GetStatus()
     such verification is done (inside BSP driver) at the beginning of erase or write operation  but
     not for read operation ==> in order to maximise BSP driver execution timing efficiency */
  while (BSP_QSPI_GetStatus() !=  QSPI_OK)
  {
    HAL_Delay(1U);
  }

  /* flash address to read is the offset from begin of external flash */
  if (BSP_QSPI_Read((uint8_t *) pDestination, source - EXTERNAL_FLASH_ADDRESS, Length) != QSPI_OK)
  {
    return HAL_ERROR;
  }
  else
  {
    return HAL_OK;
  }
}

