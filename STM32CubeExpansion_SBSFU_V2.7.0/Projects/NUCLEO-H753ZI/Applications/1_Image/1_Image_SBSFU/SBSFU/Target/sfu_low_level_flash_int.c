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
#define SFU_FLASH_BANK_SIZE 0x100000UL  /*!< Bank size = 1 MB*/

/* Private variables ---------------------------------------------------------*/
static __IO uint32_t DoubleECC_Error_Counter = 0U;


static uint32_t SFU_LL_FLASH_INT_GetBankAddr(uint32_t bank);
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
  uint32_t erase_command = 0U;
  uint32_t bank_number;

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
      /* Get the 1st page to erase */
      do
      {
        first_sector = SFU_LL_FLASH_INT_GetSector(start);
        bank_number = SFU_LL_FLASH_INT_GetBank(start);
        if (SFU_LL_FLASH_INT_GetBank(start + length - 1U) == bank_number)
        {
          /* Get the number of pages to erase from 1st page */
          nb_sectors = SFU_LL_FLASH_INT_GetSector(start + length - 1U) - first_sector + 1U;
          /* Get the bank */

          /* Fill EraseInit structure*/
          p_erase_init.TypeErase     = FLASH_TYPEERASE_SECTORS;
          p_erase_init.Banks         = bank_number;
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
          erase_command = 1U;
        }
        else
        {
          uint32_t startbank2 = SFU_LL_FLASH_INT_GetBankAddr(FLASH_BANK_2);
          nb_sectors = SFU_LL_FLASH_INT_GetSector(startbank2 - 1U) - first_sector + 1U;
          length = length  - (startbank2 - start);
          start = startbank2;

          /* Fill EraseInit structure*/
          p_erase_init.TypeErase     = FLASH_TYPEERASE_SECTORS;
          p_erase_init.Banks         = bank_number;
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
        }
      } while (erase_command == 0U);

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
  * @param  pDestination: Start address for target location. It has to be 32 bytes aligned.
  * @param  pSource: pointer on buffer with data to write
  * @param  Length: Length of data buffer in bytes. It has to be 32 bytes aligned.
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
          if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, destination, (source + i)) == HAL_OK)
          {
            /* Check the written value */
            if (memcmp((void *)destination, (void *)(source + i), sizeof(SFU_LL_FLASH_write_t)) != 0)
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
    DoubleECC_Error_Counter = 0U;
    se_ret_status = SE_SFU_IMG_Read(&se_status, pDestination, (uint8_t *)source, Length);
    if ((se_ret_status == SE_SUCCESS) && (DoubleECC_Error_Counter == 0U))
    {
      e_ret_status = SFU_SUCCESS;
    }
    DoubleECC_Error_Counter = 0U;
  }
  /* Reading operation executed by SBSFU */
  else
  {
    DoubleECC_Error_Counter = 0U;
    /* Do not use memcpy from lib : ECC error should be checked at each loop */
    for (i = 0; (i < Length) && (DoubleECC_Error_Counter == 0U); i++, pDestination++, pSource++)
    {
      *pDestination = *pSource;
    }
    if (DoubleECC_Error_Counter == 0U)
    {
      e_ret_status = SFU_SUCCESS;
    }
    DoubleECC_Error_Counter = 0U;
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
  DoubleECC_Error_Counter = 0U;
  for ( i = 0U; i < Length; i += 4U)
  {
    if ((*(uint32_t *)(flash + i) != Pattern1) &&  (*(uint32_t *)(flash + i) != Pattern2))
    {
      return SFU_ERROR;
    }
  }
  /* Verify loop exit status */
  if (i != Length)
  {
    return SFU_ERROR;
  }
  if (DoubleECC_Error_Counter != 0U)
  {
    return SFU_ERROR;
  }
  DoubleECC_Error_Counter = 0U;
  return SFU_SUCCESS;
}

/**
  * @brief  Gets the sector of a given address
  * @param  Add: flash address
  * @retval The sector of a given address
  */
uint32_t SFU_LL_FLASH_INT_GetSector(uint32_t Add)
{
  uint32_t sector;

  if (Add < (FLASH_BASE + (SFU_FLASH_BANK_SIZE)))
  {
    /* Bank 1 */
    sector = (Add - FLASH_BASE) / FLASH_SECTOR_SIZE;
  }
  else
  {
    /* Bank 2 */
    sector = (Add - (FLASH_BASE + (SFU_FLASH_BANK_SIZE))) / FLASH_SECTOR_SIZE;
  }

  return sector;
}



/**
  * @brief  Gets the bank of a given address
  * @param  Addr: flash address
  * @retval The bank of a given address
  */
uint32_t SFU_LL_FLASH_INT_GetBank(uint32_t Addr)
{
  uint32_t bank;
  if (READ_BIT(FLASH->OPTCCR, FLASH_OPTCR_SWAP_BANK) == 0U)
  {
    /* No Bank swap */
    if (Addr < (FLASH_BASE + (SFU_FLASH_BANK_SIZE)))
    {
      bank = FLASH_BANK_1;
    }
    else
    {
      bank = FLASH_BANK_2;
    }
  }
  else
  {
    /* Bank swap */
    if (Addr < (FLASH_BASE + (SFU_FLASH_BANK_SIZE)))
    {
      bank = FLASH_BANK_2;
    }
    else
    {
      bank = FLASH_BANK_1;
    }
  }

  return bank;
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
    /* Clear all FLASH flags */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS_BANK1);
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS_BANK2);

    /* Unlock the Program memory */
    if (HAL_FLASH_Lock() == HAL_OK)
    {
      e_ret_status = SFU_SUCCESS;
    }
  }

  return e_ret_status;

}

/**
  * @brief  Gets the address of a bank
  * @param  Bank: Bank ID
  * @retval Address of the bank
  */
static uint32_t SFU_LL_FLASH_INT_GetBankAddr(uint32_t Bank)
{
  if (Bank == FLASH_BANK_2)
  {
    return  FLASH_BASE + (SFU_FLASH_BANK_SIZE);
  }
  else
  {
    return FLASH_BASE;
  }
}

/**
  * @brief  Get Link Register value (LR)
  * @param  None.
  * @retval LR Register value
  */
__attribute__((always_inline)) __STATIC_INLINE uint32_t get_LR(void)
{
  register uint32_t result;

#if defined ( __ICCARM__ ) || defined ( __GNUC__ ) || defined ( __ARMCC_VERSION )
  __ASM volatile("MOV %0, LR" : "=r"(result));
#elif defined ( __CC_ARM )
  __ASM volatile("MOV result, __return_address()");
#endif /* ( __ICCARM__ ) || ( __GNUC__ ) */

  return result;
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
  * @brief  Hard Fault Handler present for handling Double ECC Bus Fault interrupt
  * @param  None.
  * @retval SFU_ErrorStatus SFU_SUCCESS if ECC fault managed, SFU_ERROR otherwise.
  */
void HardFault_Handler(void)
{
  uint32_t *p_sp;
  uint32_t lr;
  uint16_t opcode_msb;

  if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_DBECCE_BANK1RR) || __HAL_FLASH_GET_FLAG(FLASH_FLAG_DBECCE_BANK2RR))
  {
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_DBECCE_BANK1RR);
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_DBECCE_BANK2RR);

    /* Memorize error to ignore the read value */
    DoubleECC_Error_Counter++;

    lr = get_LR();

    /* Check EXC_RETURN value in LR to know which SP was used prior entering exception */
    if (((lr) & (0xFU)) == 0xDU)
    {
      /* interrupted code was using Process Stack Pointer */
      p_sp = (uint32_t *)__get_PSP();
    }
    else
    {
      /* interrupted code was using Main Stack Pointer */
      p_sp = (uint32_t *)__get_MSP();
    }

    /* Test caller mode T bit from CPSR in stack */
    if ((*(p_sp + 7U) & (1U << xPSR_T_Pos)) != 0U)
    {
      /* Thumb  mode.
         Test PC in stack.
         If bits [15:11] of the halfword being decoded take any of the following values,
         the halfword is the first halfword of a 32-bit instruction: 0b11101, 0b11110, 0b11111.
         Otherwise, the halfword is a 16-bit instruction.
      */
      opcode_msb = (*(uint16_t *)(*(p_sp + 6) & 0xFFFFFFFEU) & 0xF800U);
      if ((opcode_msb == 0xE800U) || (opcode_msb == 0xF000U) || (opcode_msb == 0xF800U))
      {
        /* execute next instruction PC +4  */
        *(p_sp + 6U) += 4U;
      }
      else
      {
        /* execute next instruction PC +2  */
        *(p_sp + 6U) += 2U;
      }
    }
    else
    {
      /* ARM mode execute next instruction PC +4 */
      *(p_sp + 6U) += 4U;
    }
  }
  else
  {
    SFU_EXCPT_IrqExceptionHandler(SFU_EXCPT_HARD_FAULT);
  }
}
