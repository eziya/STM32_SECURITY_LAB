/**
  ******************************************************************************
  * File Name          : ext_flash_driver.c
  * Description        : Single core External Flash driver
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file in
  * the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "app_common.h"

#include "ext_flash_driver.h"
#include "shci.h"
#include "utilities_conf.h"
#include "stm32wb5mm_dk_qspi.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#define ERASE_BLOC_SIZE           0x10000U           /*!< 64 Kbytes */
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
uint32_t EXT_FD_Init( void )
{
#ifdef OTA_SBSFU
  BSP_QSPI_Init_t init ;
  init.TransferRate= BSP_QSPI_STR_TRANSFER;
  init.DualFlashMode= BSP_QSPI_DUALFLASH_DISABLE;
  init.InterfaceMode = BSP_QSPI_QPI_MODE;
  if (BSP_QSPI_Init(0, &init) != BSP_ERROR_NONE)
  {
    return EXT_FLASH_OPERATION_ERROR;
  }
  else
  {
    return EXT_FLASH_OPERATION_OK;
  }
#else /* OTA_SBSFU */
  return EXT_FLASH_OPERATION_ERROR;
#endif /* OTA_SBSFU */
}

uint32_t EXT_FD_EraseSectors(uint32_t Address, uint32_t Length)
{
#ifdef OTA_SBSFU
  uint32_t return_value = EXT_FLASH_OPERATION_OK;
  uint32_t loop_flash;

  /* flash address to erase is the offset from begin of external flash */
  Address -= EXTERNAL_FLASH_ADDRESS;

  /* Loop on 64KBytes block */
  for (loop_flash = 0U ; loop_flash < (((Length - 1U) / ERASE_BLOC_SIZE) + 1U); loop_flash++)
  {
    if (BSP_QSPI_EraseBlock(0, Address, BSP_QSPI_ERASE_64K) != BSP_ERROR_NONE)
    {
      return_value = EXT_FLASH_OPERATION_ERROR;
    }

    /* next 64KBytes block */
    Address += ERASE_BLOC_SIZE;
  }
  return return_value;
#else /* OTA_SBSFU */
  return EXT_FLASH_OPERATION_ERROR;
#endif /* OTA_SBSFU */
}

uint32_t EXT_FD_WriteData(uint32_t DestAddress, uint8_t * pSrcBuffer, uint32_t Length)
{
#ifdef OTA_SBSFU
  /* Do nothing if Length equal to 0 */
  if (Length == 0U)
  {
    return HAL_OK;
  }

  /* flash address to write is the offset from begin of external flash */
  if (BSP_QSPI_Write(0, pSrcBuffer, DestAddress - EXTERNAL_FLASH_ADDRESS, Length) != BSP_ERROR_NONE)
  {
    return EXT_FLASH_OPERATION_ERROR;
  }
  else
  {
    return EXT_FLASH_OPERATION_OK;
  }
#else /* OTA_SBSFU */
  return EXT_FLASH_OPERATION_ERROR;
#endif /* OTA_SBSFU */
}
