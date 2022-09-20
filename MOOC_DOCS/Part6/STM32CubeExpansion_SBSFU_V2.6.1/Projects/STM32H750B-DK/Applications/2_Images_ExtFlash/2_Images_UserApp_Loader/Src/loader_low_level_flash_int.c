/**
  ******************************************************************************
  * @file    loader_low_level_flash_int.c
  * @author  MCD Application Team
  * @brief   Flash Low Level Interface module
  *          This file provides set of firmware functions to manage internal flash
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
#include "main.h"
#include "loader_low_level_flash.h"
#include <string.h>


/* Private defines -----------------------------------------------------------*/
#define NB_PAGE_SECTOR_PER_ERASE  2U    /*!< Nb page erased per erase */


/* Private variables ---------------------------------------------------------*/
static __IO uint32_t DoubleECC_Error_Counter = 0U;



/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef LOADER_LL_FLASH_INT_Clear_Error(void);


/* Functions Definition ------------------------------------------------------*/
/**
  * @brief  This function initialize the internal flash interface if required
  * @param  none
  * @retval HAL_OK if successful, HAL_ERROR otherwise.
  */
HAL_StatusTypeDef LOADER_LL_FLASH_INT_Init(void)
{
  /* Enable Bus Fault exception for double ECC error management */
  SET_BIT(SCB->SHCSR, ((uint32_t)SCB_SHCSR_BUSFAULTENA_Msk));

  return HAL_OK;
}

/**
  * @brief  This function does an erase of n (depends on Length) pages in user flash area
  * @param  pStart: pointer to  user flash area
  * @param  Length: number of bytes.
  * @retval HAL_OK if successful, HAL_ERROR otherwise.
  */
HAL_StatusTypeDef LOADER_LL_FLASH_INT_Erase_Size(uint8_t *pStart, uint32_t Length)
{
  uint32_t sector_error = 0U;
  uint32_t start = (uint32_t)pStart;
  uint32_t length = Length;
  FLASH_EraseInitTypeDef p_erase_init;
  HAL_StatusTypeDef e_ret_status;
  uint32_t first_sector;
  uint32_t nb_sectors;
  uint32_t chunk_nb_sectors;

  /* Initialize Flash - erase application code area*/
  e_ret_status = LOADER_LL_FLASH_INT_Clear_Error();

  if (e_ret_status == HAL_OK)
  {
    /* Unlock the Flash to enable the flash control register access *************/
    e_ret_status = HAL_FLASH_Unlock();
    if (e_ret_status == HAL_OK)
    {
      first_sector = LOADER_LL_FLASH_INT_GetSector(start);
      /* Get the number of sectors to erase from 1st sector */
      nb_sectors = LOADER_LL_FLASH_INT_GetSector(start + length - 1U) - first_sector + 1U;

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
          e_ret_status = HAL_ERROR;
        }

        /*
         * If the SecureBoot configured the IWDG, loader must reload IWDG counter with value defined in the reload
         * register
         */
        WRITE_REG(IWDG1->KR, IWDG_KEY_RELOAD);
      } while (nb_sectors > 0U);

      /*
       * Lock the Flash to disable the flash control register access (recommended
       * to protect the FLASH memory against possible unwanted operation)
       */
      (void) HAL_FLASH_Lock();

    }
  }

  return e_ret_status;
}


/**
  * @brief  This function writes a data buffer in flash (data are 32-bit aligned).
  * @note   After writing data buffer, the flash content is checked.
  * @param  pDestination: Start address for target location. It has to be 32 bytes aligned.
  * @param  pSource: pointer on buffer with data to write
  * @param  Length: Length of data buffer in bytes. It has to be 32 bytes aligned.
  * @retval HAL_OK if successful, HAL_ERROR otherwise.
  */
HAL_StatusTypeDef LOADER_LL_FLASH_INT_Write(uint8_t  *pDestination, const uint8_t *pSource, uint32_t Length)
{
  HAL_StatusTypeDef e_ret_status;
  uint32_t i;
  uint32_t pdata = (uint32_t)pSource;
  uint32_t destination = (uint32_t)pDestination;


  /* Initialize Flash - erase application code area*/
  e_ret_status = LOADER_LL_FLASH_INT_Clear_Error();

  if (e_ret_status == HAL_OK)
  {
    /* Unlock the Flash to enable the flash control register access *************/
    e_ret_status = HAL_FLASH_Unlock();
    if (e_ret_status == HAL_OK)
    {
      for (i = 0U; (i < Length) && (e_ret_status == HAL_OK); i += sizeof(LOADER_LL_FLASH_write_t))
      {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, destination, (pdata + i)) == HAL_OK)
        {
          /* Check the written value */
          if (memcmp((uint8_t *)destination, (uint8_t *)(pdata + i), sizeof(LOADER_LL_FLASH_write_t)) != 0)
          {
            /* Flash content doesn't match SRAM content */
            e_ret_status = HAL_ERROR;
          }
          else
          {
            /* Increment FLASH Destination address */
            destination = destination + sizeof(LOADER_LL_FLASH_write_t);
          }
        }
        else
        {
          /* Error occurred while writing data in Flash memory */
          e_ret_status = HAL_ERROR;
        }
      }
      /* Lock the Flash to disable the flash control register access (recommended
      to protect the FLASH memory against possible unwanted operation) */
      if (HAL_FLASH_Lock() != HAL_OK)
      {
        e_ret_status = HAL_ERROR;
      }
    }
  }
  return e_ret_status;
}

/**
  * @brief  This function reads flash
  * @param  pDestination: Start address for target location
  * @param  pSource: pointer on buffer with data to write
  * @param  Length: Length in bytes of data buffer
  * @retval HAL_OK if successful, HAL_ERROR otherwise.
  */
HAL_StatusTypeDef LOADER_LL_FLASH_INT_Read(uint8_t *pDestination, const uint8_t *pSource, uint32_t Length)
{
  HAL_StatusTypeDef e_ret_status = HAL_ERROR;
  uint32_t i;

  DoubleECC_Error_Counter = 0U;
  /* Do not use memcpy from lib : ECC error should be checked at each loop */
  for (i = 0; (i < Length) && (DoubleECC_Error_Counter == 0U); i++, pDestination++, pSource++)
  {
    *pDestination = *pSource;
  }
  if (DoubleECC_Error_Counter == 0U)
  {
    e_ret_status = HAL_OK;
  }
  DoubleECC_Error_Counter = 0U;
  return e_ret_status;
}

/**
  * @brief  Gets the sector of a given address
  * @param  Add: Address of the FLASH Memory
  * @retval The sector of a given address
  */
uint32_t LOADER_LL_FLASH_INT_GetSector(uint32_t Add)
{
  uint32_t sector;

  if (Add < (FLASH_BASE + (FLASH_BANK_SIZE)))
  {
    /* Bank 1 */
    sector = (Add - FLASH_BASE) / FLASH_SECTOR_SIZE;
  }
  else
  {
    /* Bank 2 */
    sector = (Add - (FLASH_BASE + (FLASH_BANK_SIZE))) / FLASH_SECTOR_SIZE;
  }

  return sector;
}





/* Private Functions Definition -------------------------------------------------*/

/**
  * @brief  Clear error flags raised during previous operation
  * @param  None.
  * @retval HAL_OK if successful, HAL_ERROR otherwise.
  */
static HAL_StatusTypeDef LOADER_LL_FLASH_INT_Clear_Error(void)
{
  HAL_StatusTypeDef e_ret_status = HAL_ERROR;

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
      e_ret_status = HAL_OK;
    }
  }

  return e_ret_status;

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
#endif /*  ( __ICCARM__ ) || ( __GNUC__ ) */

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
  * @brief  Bus Fault Handler present for handling Double ECC Bus Fault interrupt
  * @param  None.
  * @retvat void
  */
void BusFault_Handler(void)
{
  uint32_t *p_sp;
  uint32_t lr;
  uint16_t opcode_msb;

  if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_DBECCE_BANK1RR) || __HAL_FLASH_GET_FLAG(FLASH_FLAG_DBECCE_BANK2RR))
  {
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_DBECCE_BANK1RR);
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_DBECCE_BANK2RR);

    /*  memorize error to ignore the read value */
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
    /* This exception occurs for another reason than flash double ECC errors */
    while (1 == 1)
    {
      ;
    }
  }
}
