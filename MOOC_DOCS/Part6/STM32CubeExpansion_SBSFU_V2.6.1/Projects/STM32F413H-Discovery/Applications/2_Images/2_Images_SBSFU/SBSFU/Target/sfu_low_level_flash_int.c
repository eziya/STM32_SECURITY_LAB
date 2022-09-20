/**
  ******************************************************************************
  * @file    sfu_low_level_flash_int.c
  * @author  MCD Application Team
  * @brief   SFU Flash Low Level Interface module
  *          This file provides set of firmware functions to manage SFU internal
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
#include "sfu_low_level_flash_int.h"
#include "sfu_low_level_security.h"
#include "se_interface_bootloader.h"
#include "string.h"
#include "sfu_fsm_states.h"
#include "sfu_error.h"

/* Private defines -----------------------------------------------------------*/
#define NB_PAGE_SECTOR_PER_ERASE  2U    /*!< Nb page erased per erase */

/* Private variables ---------------------------------------------------------*/

uint32_t FlashSectorsAddress[] = {0x08000000U, 0x08004000U, 0x08008000U, 0x0800C000U, 0x08010000U, 0x08020000U,
                                  0x08040000U, 0x08060000U, 0x08080000U, 0x080A0000U, 0x080C0000U, 0x080E0000U,
                                  0x08100000U, 0x08120000U, 0x08140000U, 0x08160000U, 0x08180000U
                                 };

static SFU_ErrorStatus SFU_LL_FLASH_INT_Clear_Error(void);

/* Functions Definition ------------------------------------------------------*/
/**
  * @brief  This function initialize the internal flash interface if required
  * @param  none
  * @retval SFU_ErrorStatus SFU_SUCCESS if successful, SFU_ERROR otherwise.
  */
SFU_ErrorStatus SFU_LL_FLASH_INT_Init(void)
{
  return SFU_SUCCESS;
}

/**
  * @brief  This function does an erase of n (depends on Length) pages in user flash area
  * @param  pFlashStatus: SFU_FLASH Status pointer
  * @param  pStart: flash address to be erased
  * @param  Length: number of bytes
  * @retval SFU_ErrorStatus SFU_SUCCESS if successful, SFU_ERROR otherwise.
  */
SFU_ErrorStatus SFU_LL_FLASH_INT_Erase_Size(SFU_FLASH_StatusTypeDef *pFlashStatus, uint8_t *pStart, uint32_t Length)
{
  uint32_t sector_error = 0U;
  uint32_t start = (uint32_t)pStart;
  FLASH_EraseInitTypeDef p_erase_init;
  SFU_ErrorStatus e_ret_status = SFU_ERROR;
  uint32_t first_sector;
  uint32_t nb_sectors;
  uint32_t chunk_nb_sectors;
  uint32_t length = Length;

  /* Check the pointers allocation */
  if (pFlashStatus == NULL)
  {
    return SFU_ERROR;
  }

  *pFlashStatus = SFU_FLASH_SUCCESS;

  /* Clear error flags raised during previous operation */
  e_ret_status = SFU_LL_FLASH_INT_Clear_Error();

  if (e_ret_status == SFU_SUCCESS)
  {
    /* Unlock the Flash to enable the flash control register access *************/
    if (HAL_FLASH_Unlock() == HAL_OK)
    {
      first_sector = SFU_LL_FLASH_INT_GetSector(start);
      /* Get the number of sectors to erase from 1st sector */
      nb_sectors = SFU_LL_FLASH_INT_GetSector(start + length - 1U) - first_sector + 1U;

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
          e_ret_status = SFU_ERROR;
          *pFlashStatus = SFU_FLASH_ERR_ERASE;
        }
        SFU_LL_SECU_IWDG_Refresh(); /* calling this function which checks the compiler switch */
      } while (nb_sectors > 0U);

      /* Lock the Flash to disable the flash control register access (recommended
      to protect the FLASH memory against possible unwanted operation) *********/
      if (HAL_FLASH_Lock() != HAL_OK)
      {
        e_ret_status = SFU_ERROR;
        *pFlashStatus = SFU_FLASH_ERR_HAL;
      }
    }
    else
    {
      *pFlashStatus = SFU_FLASH_ERR_HAL;
    }
  }

  return e_ret_status;
}

/**
  * @brief  This function writes a data buffer in flash (data are 32-bit aligned).
  * @note   After writing data buffer, the flash content is checked.
  * @param  pFlashStatus: FLASH_StatusTypeDef
  * @param  pDestination: Start address for target location. It has to be 1 byte aligned.
  * @param  pSource: pointer on buffer with data to write
  * @param  Length: Length of data buffer in bytes. It has to be 1 byte aligned.
  * @retval SFU_ErrorStatus SFU_SUCCESS if successful, SFU_ERROR otherwise.
  */
SFU_ErrorStatus SFU_LL_FLASH_INT_Write(SFU_FLASH_StatusTypeDef *pFlashStatus, uint8_t  *pDestination,
                                       const uint8_t *pSource, uint32_t Length)
{
  SFU_ErrorStatus e_ret_status = SFU_ERROR;
  uint32_t i;
  uint32_t inside_header = 0U;
  uint32_t destination = (uint32_t)pDestination;
  uint32_t source = (uint32_t)pSource;

  /* Check the pointers allocation */
  if ((pFlashStatus == NULL) || (pSource == NULL))
  {
    return SFU_ERROR;
  }

  /* Is destination area inside 1 of the firmware image headers ? */
  for (i = 0U; i < SFU_NB_MAX_ACTIVE_IMAGE; i++)
  {
    if ((destination >= SlotHeaderAdd[SLOT_ACTIVE_1 + i]) &&
        ((destination + Length) <= (SlotHeaderAdd[SLOT_ACTIVE_1 + i] + SFU_IMG_IMAGE_OFFSET)))
    {
      inside_header = 1U;
    }
  }

  /* Destination area part of 1 of the firmware image headers :
     writing operation should be executed inside secure environment */
  if ((inside_header == 1U) && (Length != 0U))
  {
    /* SE Access */
    SE_StatusTypeDef se_status;
    SE_ErrorStatus se_ret_status = SE_SFU_IMG_Write(&se_status, (uint8_t *)destination, pSource, Length);
    if (se_ret_status == SE_SUCCESS)
    {
      e_ret_status = SFU_SUCCESS;
      *pFlashStatus = SFU_FLASH_SUCCESS;
    }
    else
    {
      e_ret_status = SFU_ERROR;
      *pFlashStatus = SFU_FLASH_ERROR;
    }
  }
  /* Writing operation executed by SBSFU */
  else
  {
    *pFlashStatus = SFU_FLASH_ERROR;

    /* Clear error flags raised during previous operation */
    e_ret_status = SFU_LL_FLASH_INT_Clear_Error();

    if (e_ret_status == SFU_SUCCESS)
    {
      /* Unlock the Flash to enable the flash control register access *************/
      if (HAL_FLASH_Unlock() != HAL_OK)
      {
        *pFlashStatus = SFU_FLASH_ERR_HAL;

      }
      else
      {
        for (i = 0U; (i < Length) && (e_ret_status == SFU_SUCCESS); i += sizeof(SFU_LL_FLASH_write_t))
        {
          *pFlashStatus = SFU_FLASH_ERROR;
          if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, destination, *((uint8_t *)(source + i))) == HAL_OK)
          {
            /* Check the written value */
            if (*(uint8_t *)destination != *(uint8_t *)(source + i))
            {
              /* Flash content doesn't match SRAM content */
              *pFlashStatus = SFU_FLASH_ERR_WRITINGCTRL;
              e_ret_status = SFU_ERROR;
            }
            else
            {
              /* Increment FLASH Destination address */
              destination = destination + sizeof(SFU_LL_FLASH_write_t);
              e_ret_status = SFU_SUCCESS;
              *pFlashStatus = SFU_FLASH_SUCCESS;
            }
          }
          else
          {
            /* Error occurred while writing data in Flash memory */
            *pFlashStatus = SFU_FLASH_ERR_WRITING;
            e_ret_status = SFU_ERROR;
          }
        }
        /* Lock the Flash to disable the flash control register access (recommended
        to protect the FLASH memory against possible unwanted operation) */
        if (HAL_FLASH_Lock() != HAL_OK)
        {
          e_ret_status = SFU_ERROR;
          *pFlashStatus = SFU_FLASH_ERR_HAL;
        }
      }
    }
  }
  return e_ret_status;
}

/**
  * @brief  This function reads flash
  * @param  pDestination: Start address for target location
  * @param  pSource: flash address to read
  * @param  Length: number of bytes
  * @retval SFU_ErrorStatus SFU_SUCCESS if successful, SFU_ERROR otherwise.
  */
SFU_ErrorStatus SFU_LL_FLASH_INT_Read(uint8_t *pDestination, const uint8_t *pSource, uint32_t Length)
{
  SFU_ErrorStatus e_ret_status = SFU_ERROR;
  SE_ErrorStatus se_ret_status;
  SE_StatusTypeDef se_status;
  uint32_t i;
  uint32_t inside_header = 0U;
  uint32_t source = (uint32_t)pSource;

  /* Is destination area inside 1 of the firmware image headers ? */
  for (i = 0U; i < SFU_NB_MAX_ACTIVE_IMAGE; i++)
  {
    if ((source >= SlotHeaderAdd[SLOT_ACTIVE_1 + i]) &&
        ((source + Length) <= (SlotHeaderAdd[SLOT_ACTIVE_1 + i] + SFU_IMG_IMAGE_OFFSET)))
    {
      inside_header = 1U;
    }
  }

  /* Destination area part of 1 of the firmware image headers :
     reading operation should be executed inside secure environment */
  if (inside_header == 1U)
  {
    /* SE Access */
    se_ret_status = SE_SFU_IMG_Read(&se_status, pDestination, (uint8_t *)source, Length);
    if (se_ret_status == SE_SUCCESS)
    {
      e_ret_status = SFU_SUCCESS;
    }
  }
  /* Reading operation executed by SBSFU */
  else
  {
    (void) memcpy(pDestination, (uint8_t *)source, Length);
    e_ret_status = SFU_SUCCESS;
  }
  return e_ret_status;
}

/**
  * @brief  This function compare a buffer with a flash area
  * @note   The flash area should not be located inside the secure area
  * @param  pFlash: address of the flash area
  * @param  Pattern1: first 32 bits pattern to be compared
  * @param  Pattern2: second 32 bits pattern to be compared
  * @param  Length: number of bytes to be compared
  * @retval SFU_ErrorStatus SFU_SUCCESS if successful, SFU_ERROR otherwise.
  */
SFU_ErrorStatus SFU_LL_FLASH_INT_Compare(const uint8_t *pFlash, const uint32_t Pattern1, const uint32_t Pattern2, uint32_t Length)
{
  uint32_t flash = (uint32_t) pFlash;
  uint32_t i;

  /* Comparison executed by SBSFU ==> flash area could not be located inside secured environment */
  for ( i = 0U; i < Length; i += 4U)
  {
    if ((*(uint32_t *)(flash + i) != Pattern1) &&  (*(uint32_t *)(flash + i) != Pattern2))
    {
      return SFU_ERROR;
    }
  }
  return SFU_SUCCESS;
}

/**
  * @brief  Gets the sector of a given address
  * @param  Add: flash address
  * @retval The sector of a given address
  */
uint32_t SFU_LL_FLASH_INT_GetSector(uint32_t Add)
{
  uint32_t sector = 0U;

  while (Add >= FlashSectorsAddress[sector + 1U])
  {
    sector++;
  }
  return sector;
}




/**
  * @brief  Clear error flags raised during previous operation
  * @param  None.
  * @retval SFU_ErrorStatus SFU_SUCCESS if successful, SFU_ERROR otherwise.
  */
static SFU_ErrorStatus SFU_LL_FLASH_INT_Clear_Error(void)
{
  SFU_ErrorStatus e_ret_status = SFU_ERROR;

  /* Unlock the Program memory */
  if (HAL_FLASH_Unlock() == HAL_OK)
  {

    /* Clear all FLASH flags */
    __HAL_FLASH_CLEAR_FLAG(FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR | FLASH_SR_RDERR);

    /* Unlock the Program memory */
    if (HAL_FLASH_Lock() == HAL_OK)
    {
      e_ret_status = SFU_SUCCESS;
    }
  }

  return e_ret_status;

}



/*
   As this handler code relies on stack pointer position to manipulate the PC return value, it is important
   not to use extra registers (on top of scratch registers), because it would change the stack pointer
   position. Then compiler optimizations are customized to ensure that.
*/
#if defined(__ICCARM__)
#pragma optimize=none
#elif defined(__CC_ARM)
#pragma O0
#elif defined (__ARMCC_VERSION)
__attribute__((optnone))
#elif defined ( __GNUC__ )
__attribute__((optimize("O1")))
#endif /* __ICCARM__ */

/**
  * @brief  NMI Handler
  * @param  None.
  * @retval None.
  */
void NMI_Handler(void)
{
  while (1 == 1)
  {
    ;
  }
}


/**
  * @brief  HardFault Handler
  * @param  None.
  * @retval None.
  */
void HardFault_Handler(void)
{
  SFU_EXCPT_IrqExceptionHandler(SFU_EXCPT_HARD_FAULT);
}
