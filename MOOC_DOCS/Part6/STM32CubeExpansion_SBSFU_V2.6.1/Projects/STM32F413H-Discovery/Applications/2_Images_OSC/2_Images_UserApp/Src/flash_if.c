/**
  ******************************************************************************
  * @file    flash_if.c
  * @author  MCD Application Team
  * @brief   FLASH Interface module.
  *          This file provides set of firmware functions to manage Flash
  *          Interface functionalities.
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

/** @addtogroup USER_APP User App Example
  * @{
  */

/** @addtogroup USER_APP_COMMON Common
  * @{
  */
/* Includes ------------------------------------------------------------------*/
#include "flash_if.h"
#include "string.h"
#include <stdio.h>

/* Uncomment the line below if you want some debug logs */
#define FLASH_IF_DBG
#ifdef FLASH_IF_DBG
#define FLASH_IF_TRACE printf
#else
#define FLASH_IF_TRACE(...)
#endif /* FLASH_IF_DBG */


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define NB_PAGE_SECTOR_PER_ERASE  2U    /*!< Nb page erased per erase */
#define EXTERNAL_FLASH_ADDRESS    0x90000000U


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/**
  * @brief  Gets the the sector of a given address
  * @param  uAddr: Address of the FLASH Memory
  * @retval The sector of a given address
  */
uint32_t FlashSectorsAddress[] = {0x08000000U, 0x08004000U, 0x08008000U, 0x0800C000U, 0x08010000U, 0x08020000U,
                                  0x08040000U, 0x08060000U, 0x08080000U, 0x080A0000U, 0x080C0000U, 0x080E0000U,
                                  0x08100000U, 0x08120000U, 0x08140000U, 0x08160000U, 0x08180000U
                                 };


/* Private function prototypes -----------------------------------------------*/
static uint32_t GetSector(uint32_t uAddr);
static HAL_StatusTypeDef FLASH_INT_If_Clear_Error(void);

/* Public functions : wrapper ---------------------------------------------------------*/

/**
  * @brief  Initialize internal and external flash interface (OSPI/QSPI)
  * @param  none
  * @retval HAL status.
  */
HAL_StatusTypeDef FLASH_If_Init(void)
{
  HAL_StatusTypeDef e_ret_status = HAL_OK;

  e_ret_status = FLASH_INT_Init();
  if (e_ret_status == HAL_OK)
  {
    e_ret_status = FLASH_EXT_Init();
  }
  return e_ret_status;
}
/**
  * @brief  Depending on start address, this function will call internal or external (OSPI/QSPI) flash driver
  * @param  pStart: flash address to be erased
  * @param  uLength: number of bytes
  * @retval HAL status.
  */
HAL_StatusTypeDef FLASH_If_Erase_Size(void *pStart, uint32_t uLength)
{
  /* Check Flash start address */
  if ((uint32_t) pStart < EXTERNAL_FLASH_ADDRESS)
  {
    return FLASH_INT_If_Erase_Size(pStart, uLength);
  }
  else
  {
    return FLASH_EXT_If_Erase_Size(pStart, uLength);
  }
}

/**
  * @brief  Depending on destination address, this function will call internal or external (OSPI/QSPI) flash driver
  * @param  pDestination: flash address to write
  * @param  pSource: pointer on buffer with data to write
  * @param  uLength: number of bytes
  * @retval HAL Status.
  */
HAL_StatusTypeDef FLASH_If_Write(void *pDestination, const void *pSource, uint32_t uLength)
{
  /* Check Flash destination address */
  if ((uint32_t) pDestination < EXTERNAL_FLASH_ADDRESS)
  {
    return FLASH_INT_If_Write(pDestination, pSource, uLength);
  }
  else
  {
    return FLASH_EXT_If_Write(pDestination, pSource, uLength);
  }
}

/**
  * @brief  Depending on destination address, this function will call internal or external (OSPI/QSPI) flash driver
  * @brief  Depending on source address, this function will call internal or external (OSPI/QSPI) flash driver
  * @param  pDestination: pointer on buffer to store data
  * @param  pSource: flash address to read
  * @param  uLength: number of bytes
  * @retval HAL Status.
  */
HAL_StatusTypeDef FLASH_If_Read(void *pDestination, const void *pSource, uint32_t uLength)
{
  /* Check Flash source address */
  if ((uint32_t) pSource < EXTERNAL_FLASH_ADDRESS)
  {
    return FLASH_INT_If_Read(pDestination, pSource, uLength);
  }
  else
  {
    return FLASH_EXT_If_Read(pDestination, pSource, uLength);
  }
}

/* Public functions : internal flash --------------------------------------------------------- */
/**
  * @brief  This function initialize the internal flash interface if required
  * @param  none
  * @retval HAL status.
  */
HAL_StatusTypeDef FLASH_INT_Init(void)
{
  return HAL_OK;
}

/**
  * @brief  This function does an erase of n (depends on Length) sectors in user flash area
  * @param  pStart: Start of user flash area
  * @param  uLength: number of bytes.
  * @retval HAL status.
  */
HAL_StatusTypeDef FLASH_INT_If_Erase_Size(void *pStart, uint32_t uLength)
{
  uint32_t sector_error = 0U;
  uint32_t uStart = (uint32_t)pStart;
  FLASH_EraseInitTypeDef p_erase_init;
  HAL_StatusTypeDef e_ret_status = HAL_OK;
  uint32_t first_sector = 0U;
  uint32_t nb_sectors = 0U;
  uint32_t chunk_nb_sectors;

  /* Clear error flags raised during previous operation */
  e_ret_status = FLASH_INT_If_Clear_Error();

  if (e_ret_status == HAL_OK)
  {
    /* Unlock the Flash to enable the flash control register access *************/
    if (HAL_FLASH_Unlock() == HAL_OK)
    {
      first_sector = GetSector(uStart);
      /* Get the number of sectors to erase from 1st sector */
      nb_sectors = GetSector(uStart + uLength - 1U) - first_sector + 1U;

      /* Fill EraseInit structure*/
      p_erase_init.TypeErase     = FLASH_TYPEERASE_SECTORS;
      p_erase_init.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
      /* Erase flash per NB_PAGE_SECTOR_PER_ERASE to avoid watch-dog */
      do
      {
        chunk_nb_sectors = (nb_sectors >= NB_PAGE_SECTOR_PER_ERASE) ? NB_PAGE_SECTOR_PER_ERASE : nb_sectors;
        p_erase_init.Sector = first_sector;
        p_erase_init.NbSectors = chunk_nb_sectors;
        first_sector += chunk_nb_sectors;
        nb_sectors -= chunk_nb_sectors;
        if (HAL_FLASHEx_Erase(&p_erase_init, &sector_error) != HAL_OK)
        {
          HAL_FLASH_GetError();
          e_ret_status = HAL_ERROR;
        }
        /* Refresh Watchdog */
        WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
      } while (nb_sectors > 0);

      /* Lock the Flash to disable the flash control register access (recommended
      to protect the FLASH memory against possible unwanted operation) *********/
      HAL_FLASH_Lock();

    }
    else
    {
      e_ret_status = HAL_ERROR;
    }
  }

  return e_ret_status;
}


/**
  * @brief  This function writes a data buffer in flash (data are 32-bit aligned).
  * @note   After writing data buffer, the flash content is checked.
  * @param  pDestination: Start address for target location. It has to be 1 byte aligned.
  * @param  pSource: pointer on buffer with data to write
  * @param  uLength: Length of data buffer in bytes. It has to be 1 byte aligned.
  * @retval HAL Status.
  */
HAL_StatusTypeDef FLASH_INT_If_Write(void *pDestination, const void *pSource, uint32_t uLength)
{
  HAL_StatusTypeDef e_ret_status = HAL_ERROR;
  uint32_t pdata = (uint32_t)pSource;

  /* Clear error flags raised during previous operation */
  e_ret_status = FLASH_INT_If_Clear_Error();

  if (e_ret_status == HAL_OK)
  {
    /* Unlock the Flash to enable the flash control register access *************/
    if (HAL_FLASH_Unlock() != HAL_OK)
    {
      return HAL_ERROR;

    }
    else
    {
      for (uint32_t i = 0U; i < uLength; i++)
      {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, (uint32_t) pDestination,  *((uint8_t *)(pdata + i))) == HAL_OK)
        {
          /* Check the written value */
          if (*(uint8_t *)pDestination != *(uint8_t *)(pdata + i))
          {
            /* Flash content doesn't match SRAM content */
            e_ret_status = HAL_ERROR;
            break;
          }
          else
          {
            /* Increment FLASH Destination address */
            pDestination = (void *)((uint32_t)pDestination + 1U);
          }
        }
        else
        {
          /* Error occurred while writing data in Flash memory */
          e_ret_status = HAL_ERROR;
          break;
        }
      }
      /* Lock the Flash to disable the flash control register access (recommended
      to protect the FLASH memory against possible unwanted operation) *********/
      HAL_FLASH_Lock();
    }
  }
  return e_ret_status;
}

/**
  * @brief  This function reads flash
  * @param  pDestination: Start address for target location
  * @param  pSource: flash address to read
  * @param  uLength: number of bytes
  * @retval HAL Status.
  */
HAL_StatusTypeDef FLASH_INT_If_Read(void *pDestination, const void *pSource, uint32_t uLength)
{
  memcpy(pDestination, pSource, uLength);
  return HAL_OK;
}

/* Public functions : external flash ---------------------------------------------------------*/

HAL_StatusTypeDef FLASH_EXT_Init(void)
{
  return HAL_OK;
}

/* No external flash available on this product
   ==> return SFU_ERROR */

HAL_StatusTypeDef FLASH_EXT_If_Erase_Size(void *pStart, uint32_t uLength)
{
  return HAL_ERROR;
}

HAL_StatusTypeDef FLASH_EXT_If_Write(void  *pDestination, const void *pSource, uint32_t uLength)
{
  return HAL_ERROR;
}

HAL_StatusTypeDef FLASH_EXT_If_Read(void *pDestination, const void *pSource, uint32_t uLength)
{
  return HAL_ERROR;
}

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Clear error flags raised during previous operation
  * @param  None
  * @retval HAL Status.
  */
HAL_StatusTypeDef FLASH_INT_If_Clear_Error(void)
{
  HAL_StatusTypeDef ret = HAL_ERROR;

  /* Unlock the Program memory */
  if (HAL_FLASH_Unlock() == HAL_OK)
  {

    /* Clear all FLASH flags */
    __HAL_FLASH_CLEAR_FLAG(FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR | FLASH_SR_RDERR);
    /* Unlock the Program memory */
    if (HAL_FLASH_Lock() == HAL_OK)
    {
      ret = HAL_OK;
    }
#ifdef FLASH_IF_DBG
    else
    {
      FLASH_IF_TRACE("[FLASH_IF] Lock failure\r\n");
    }
#endif /* FLASH_IF_DBG */
  }
#ifdef FLASH_IF_DBG
  else
  {
    FLASH_IF_TRACE("[FLASH_IF] Unlock failure\r\n");
  }
#endif /* FLASH_IF_DBG */
  return ret;
}

static uint32_t GetSector(uint32_t Add)
{
  uint32_t sector = 0;

  while (Add >= FlashSectorsAddress[sector + 1])
  {
    sector++;
  }
  return sector;
}


/**
  * @}
  */

/**
  * @}
  */
