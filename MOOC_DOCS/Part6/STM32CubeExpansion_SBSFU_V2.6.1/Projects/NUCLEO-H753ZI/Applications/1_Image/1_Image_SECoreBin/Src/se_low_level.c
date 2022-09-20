/**
  ******************************************************************************
  * @file    se_low_level.c
  * @author  MCD Application Team
  * @brief   Secure Engine Interface module.
  *          This file provides set of firmware functions to manage SE low level
  *          interface functionalities.
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

#define SE_LOW_LEVEL_C

/* Includes ------------------------------------------------------------------*/
#include "se_low_level.h"
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
#include "mapping_sbsfu.h"
#endif /* __CC_ARM */
#include "se_exception.h"
#include "string.h"

/** @addtogroup SE Secure Engine
  * @{
  */
/** @defgroup  SE_HARDWARE SE Hardware Interface
  * @{
  */

/** @defgroup SE_HARDWARE_Private_Variables Private Variables
  * @{
  */
static CRC_HandleTypeDef    CrcHandle;                  /*!< SE Crc Handle*/

static __IO uint32_t SE_DoubleECC_Error_Counter = 0U;

/**
  * @}
  */

/** @defgroup SE_HARDWARE_Private_Functions Private Functions
  * @{
  */
static uint32_t SE_LL_GetBank(uint32_t Address);

static uint32_t SE_LL_GetSector(uint32_t Address);
/**
  * @}
  */

/** @defgroup SE_HARDWARE_Exported_Variables Exported Variables
  * @{
  */

/**
  * @}
  */

/** @defgroup SE_HARDWARE_Exported_Functions Exported Functions
  * @{
  */

/** @defgroup SE_HARDWARE_Exported_CRC_Functions CRC Exported Functions
  * @{
  */

/**
  * @brief  Set CRC configuration and call HAL CRC initialization function.
  * @param  None.
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise
  */
SE_ErrorStatus SE_LL_CRC_Config(void)
{
  SE_ErrorStatus e_ret_status = SE_ERROR;

  CrcHandle.Instance = CRC;
  /* The input data are not inverted */
  CrcHandle.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;

  /* The output data are not inverted */
  CrcHandle.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;

  /* The Default polynomial is used */
  CrcHandle.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  /* The default init value is used */
  CrcHandle.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  /* The input data are 32-bit long words */
  CrcHandle.InputDataFormat = CRC_INPUTDATA_FORMAT_WORDS;
  /* CRC Init*/
  if (HAL_CRC_Init(&CrcHandle) == HAL_OK)
  {
    e_ret_status = SE_SUCCESS;
  }

  return e_ret_status;
}

/**
  * @brief  Wrapper to HAL CRC initialization function.
  * @param  None
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_LL_CRC_Init(void)
{
  /* CRC Peripheral clock enable */
  __HAL_RCC_CRC_CLK_ENABLE();

  return SE_LL_CRC_Config();
}

/**
  * @brief  Wrapper to HAL CRC de-initialization function.
  * @param  None
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_LL_CRC_DeInit(void)
{
  SE_ErrorStatus e_ret_status = SE_ERROR;

  if (HAL_CRC_DeInit(&CrcHandle) == HAL_OK)
  {
    /* Initialization OK */
    e_ret_status = SE_SUCCESS;
  }

  return e_ret_status;
}

/**
  * @brief  Wrapper to HAL CRC Calculate function.
  * @param  pBuffer: pointer to data buffer.
  * @param  uBufferLength: buffer length in 32-bits word.
  * @retval uint32_t CRC (returned value LSBs for CRC shorter than 32 bits)
  */
uint32_t SE_LL_CRC_Calculate(uint32_t pBuffer[], uint32_t uBufferLength)
{
  return HAL_CRC_Calculate(&CrcHandle, pBuffer, uBufferLength);
}

/**
  * @}
  */

/** @defgroup SE_HARDWARE_Exported_FLASH_Functions FLASH Exported Functions
  * @{
  */

/**
  * @brief  This function does an erase of nb sectors in user flash area
  * @param  pStart: pointer to  user flash area
  * @param  Length: number of bytes.
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_LL_FLASH_Erase(uint8_t *pStart, uint32_t Length)
{
  uint32_t sector_error = 0U;
  uint32_t start = (uint32_t)pStart;
  FLASH_EraseInitTypeDef p_erase_init;
  SE_ErrorStatus e_ret_status = SE_SUCCESS;

  /* Unlock the Flash to enable the flash control register access *************/
  if (HAL_FLASH_Unlock() == HAL_OK)
  {
    /* Fill EraseInit structure*/
    p_erase_init.TypeErase     = FLASH_TYPEERASE_SECTORS;
    p_erase_init.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
    p_erase_init.Banks         = SE_LL_GetBank(start);
    p_erase_init.Sector        = SE_LL_GetSector(start);
    p_erase_init.NbSectors     = SE_LL_GetSector(start + Length - 1U) - p_erase_init.Sector + 1U;
    if (HAL_FLASHEx_Erase(&p_erase_init, &sector_error) != HAL_OK)
    {
      e_ret_status = SE_ERROR;
    }

    /* Lock the Flash to disable the flash control register access (recommended
    to protect the FLASH memory against possible unwanted operation) *********/
    (void)HAL_FLASH_Lock();
  }
  else
  {
    e_ret_status = SE_ERROR;
  }

  return e_ret_status;
}

/**
  * @brief  Write in Flash  protected area
  * @param  pDestination pointer to destination area in Flash
  * @param  pSource pointer to input buffer
  * @param  Length number of bytes to be written
  * @retval SE_SUCCESS if successful, otherwise SE_ERROR
  */

SE_ErrorStatus SE_LL_FLASH_Write(uint8_t *pDestination, const uint8_t *pSource, uint32_t Length)
{
  SE_ErrorStatus ret = SE_SUCCESS;
  uint32_t i;
  uint32_t verified = 0U;
  uint32_t destination = (uint32_t)pDestination;
  uint32_t source = (uint32_t)pSource;

  if (Length == 0U)
  {
    return SE_ERROR;
  }

  /* Area should be inside 1 of the firmware image headers */
  for (i = 0U; i < SFU_NB_MAX_ACTIVE_IMAGE; i++)
  {
    if ((destination >= SlotHeaderAdd[SLOT_ACTIVE_1 + i]) &&
        ((destination + Length) <= (SlotHeaderAdd[SLOT_ACTIVE_1 + i] + SFU_IMG_IMAGE_OFFSET)))
    {
      verified = 1U;
    }
  }
  if (verified == 0U)
  {
    return SE_ERROR;
  }

  /* Unlock the Flash to enable the flash control register access *************/
  if (HAL_FLASH_Unlock() == HAL_OK)
  {
    for (i = 0U; i < Length; i += 32U)
    {
      if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, (destination + i), (source + i)) != HAL_OK)
      {
        ret = SE_ERROR;
        break;
      }
    }

    /* Lock the Flash to disable the flash control register access (recommended
    to protect the FLASH memory against possible unwanted operation) */
    (void)HAL_FLASH_Lock();
  }
  else
  {
    ret = SE_ERROR;
  }
  return ret;
}

/**
  * @brief  Read in Flash protected area
  * @param  pDestination: Start address for target location
  * @param  pSource: pointer on buffer with data to read
  * @param  Length: Length in bytes of data buffer
  * @retval SFU_ErrorStatus SFU_SUCCESS if successful, SFU_ERROR otherwise.
  */
SE_ErrorStatus SE_LL_FLASH_Read(uint8_t *pDestination, const uint8_t *pSource, uint32_t Length)
{
  uint32_t source = (uint32_t)pSource;
  SE_ErrorStatus e_ret_status = SE_ERROR;
  uint32_t i;
  uint32_t verified = 0U;

  /* Area should be inside 1 of the firmware image headers
     or inside 1 of the download areas */
  for (i = 0U; i < SFU_NB_MAX_ACTIVE_IMAGE; i++)
  {
    if ((source >= SlotHeaderAdd[SLOT_ACTIVE_1 + i]) &&
        ((source + Length) <= (SlotHeaderAdd[SLOT_ACTIVE_1 + i] + SFU_IMG_IMAGE_OFFSET)))
    {
      verified = 1U;
    }
  }


  if (verified == 0U)
  {
    return SE_ERROR;
  }

  SE_DoubleECC_Error_Counter = 0U;
  /* Do not use memcpy from lib : ECC error should be checked at each loop */
  for (i = 0; (i < Length) && (SE_DoubleECC_Error_Counter == 0U); i++, pDestination++, pSource++)
  {
    *pDestination = *pSource;
  }
  if (SE_DoubleECC_Error_Counter == 0U)
  {
    e_ret_status = SE_SUCCESS;
  }
  SE_DoubleECC_Error_Counter = 0U;
  return e_ret_status;
}

/**
  * @}
  */

/** @defgroup SE_HARDWARE_Exported_FLASH_EXT_Functions External FLASH Exported Functions
  * @{
  */

/**
  * @brief Initialisation of external flash On The Fly DECryption (OTFDEC)
  * @param pxSE_Metadata: Firmware metadata.
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_LL_FLASH_EXT_Decrypt_Init(SE_FwRawHeaderTypeDef *pxSE_Metadata)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(pxSE_Metadata);
  return SE_SUCCESS;
}

/**
  * @}
  */

/**
  * @brief Check if an array is inside the RAM of the product
  * @param Addr : address  of array
  * @param Length : length of array in byte
  */
SE_ErrorStatus SE_LL_Buffer_in_ram(void *pBuff, uint32_t Length)
{
  SE_ErrorStatus e_ret_status;
  uint32_t addr_start = (uint32_t)pBuff;
  uint32_t addr_end = addr_start + Length - 1U;

  /* Check if length is positive with no overflow */
  if ((Length != 0U) && (!((0xFFFFFFFFUL - addr_start) < Length))
      && ((addr_end  <= SB_REGION_RAM_END) && (addr_start >= (SE_REGION_RAM_END + 1U))))
  {
    e_ret_status = SE_SUCCESS;
  }
  else
  {
    e_ret_status = SE_ERROR;

    /* Could be an attack ==> Reset */
    NVIC_SystemReset();
  }
  return e_ret_status;
}

/**
  * @brief function checking if a buffer is in sbsfu ram.
  * @param pBuff: address of buffer
  * @param Length: length of buffer in bytes
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_LL_Buffer_in_SBSFU_ram(const void *pBuff, uint32_t Length)
{
  SE_ErrorStatus e_ret_status;
  uint32_t addr_start = (uint32_t)pBuff;
  uint32_t addr_end = addr_start + Length - 1U;

  /* Check if length is positive with no overflow */
  if ((Length != 0U) && (!((0xFFFFFFFFUL - addr_start) < Length))
      && ((addr_end  <= SB_REGION_RAM_END) && (addr_start >= SB_REGION_RAM_START)))
  {
    e_ret_status = SE_SUCCESS;
  }
  else
  {
    e_ret_status = SE_ERROR;

    /* Could be an attack ==> Reset */
    NVIC_SystemReset();
  }
  return e_ret_status;
}

/**
  * @brief function checking if a buffer is PARTLY in se ram.
  * @param pBuff: address of buffer
  * @param Length: length of buffer in bytes
  * @retval SE_ErrorStatus SE_SUCCESS for buffer in NOT se ram, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_LL_Buffer_part_of_SE_ram(const void *pBuff, uint32_t Length)
{
  SE_ErrorStatus e_ret_status;
  uint32_t addr_start = (uint32_t)pBuff;
  uint32_t addr_end = addr_start + Length - 1U;

  /* Check if length is positive with no overflow */
  if (((Length != 0U) && (!(((addr_start < SE_REGION_RAM_START) && (addr_end < SE_REGION_RAM_START)) ||
                            ((addr_start > SE_REGION_RAM_END) && (addr_end > SE_REGION_RAM_END)))))
      || ((0xFFFFFFFFUL - addr_start) < Length))
  {
    e_ret_status = SE_SUCCESS;

    /* Could be an attack ==> Reset */
    NVIC_SystemReset();
  }
  else
  {
    e_ret_status = SE_ERROR;
  }
  return e_ret_status;
}

/**
  * @brief function checking if a buffer is PARTLY in se rom.
  * @param pBuff: address of buffer
  * @param Length: length of buffer in bytes
  * @retval SE_ErrorStatus SE_SUCCESS for buffer in NOT se rom, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_LL_Buffer_part_of_SE_rom(const void *pBuff, uint32_t Length)
{
  SE_ErrorStatus e_ret_status;
  uint32_t addr_start = (uint32_t)pBuff;
  uint32_t addr_end = addr_start + Length - 1U;

  /* Check if length is positive with no overflow */
  if (((Length != 0U) && (!(((addr_start < SE_CODE_REGION_ROM_START) && (addr_end < SE_CODE_REGION_ROM_START))
                            || ((addr_start > SE_CODE_REGION_ROM_END) && (addr_end > SE_CODE_REGION_ROM_END))
                           )))
      || ((0xFFFFFFFFUL - addr_start) < Length))
  {
    e_ret_status = SE_SUCCESS;

    /* Could be an attack ==> Reset */
    NVIC_SystemReset();
  }
  else
  {
    e_ret_status = SE_ERROR;
  }
  return e_ret_status;
}

/**
  * @}
  */

/** @addtogroup SE_BUFFER_CHECK_Exported_IRQ_Functions
  * @{
  */

/**
  * @brief function to disable all IRQ, previously enabled ones are stored in parameter
  * @param pIrqState: IRQ state buffer
  * @param IrqStateNb: Number of IRQ state that can be stored
  * @retval SE_ErrorStatus SE_SUCCESS when complete successfully, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_LL_Save_Disable_Irq(uint32_t *pIrqState, uint32_t IrqStateNb)
{
  SE_ErrorStatus e_ret_status = SE_ERROR;

  /* No running services for this product */
  return e_ret_status;
}

/**
  * @brief function checking if a buffer is PARTLY in se rom.
  * @param pBuff: address of buffer
  * @param Length: length of buffer in bytes
  * @retval SE_ErrorStatus SE_SUCCESS for buffer in NOT se rom, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_LL_Restore_Enable_Irq(uint32_t *pIrqState, uint32_t IrqStateNb)
{
  SE_ErrorStatus e_ret_status = SE_ERROR;

  /* No running services for this product */
  return e_ret_status;
}

/**
  * @}
  */

/**
  * @}
  */

/** @addtogroup SE_HARDWARE_Private_Functions
  * @{
  */

/**
  * @brief  Gets the sector of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The sector of a given address
  */
static uint32_t SE_LL_GetSector(uint32_t Address)
{
  uint32_t sector;

  if (Address < (FLASH_BASE + (FLASH_BANK_SIZE)))
  {
    /* Bank 1 */
    sector = (Address - FLASH_BASE) / FLASH_SECTOR_SIZE;
  }
  else
  {
    /* Bank 2 */
    sector = (Address - (FLASH_BASE + (FLASH_BANK_SIZE))) / FLASH_SECTOR_SIZE;
  }

  return sector;
}

static uint32_t SE_LL_GetBank(uint32_t Address)
{
  uint32_t bank;

  if (READ_BIT(FLASH->OPTCCR, FLASH_OPTCR_SWAP_BANK) == 0U)
  {
    /* No Bank swap */
    if (Address < (FLASH_BASE + (FLASH_BANK_SIZE)))
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
    if (Address < (FLASH_BASE + (FLASH_BANK_SIZE)))
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
  * @brief  Get Link Register value (LR)
  * @param  None.
  * @retval LR Register value
  */
__attribute__((always_inline)) __STATIC_INLINE uint32_t get_LR(void)
{
  register uint32_t result;

#if defined ( __ICCARM__ ) || defined ( __GNUC__ ) || defined (__ARMCC_VERSION)
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
#elif defined(__ARMCC_VERSION)
__attribute__((optnone))
#elif defined ( __GNUC__ )
__attribute__((optimize("O1")))
#endif /* __ICCARM__ */


/**
  * @brief  NMI Handler redirection to SE NMI handler
  * @param  None.
  * @retval None.
  */
void NMI_Handler(void)
{
  SE_NMI_ExceptionHandler();
}

/**
  * @brief  Hard Fault Handler present for handling Double ECC Bus Fault interrupt
  * @param  None.
  * @retvat void
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
    SE_DoubleECC_Error_Counter++;

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
    SE_NMI_ExceptionHandler();
  }
}

/**
  * @brief  Cleanup SE CORE
  * The function is called  during SE_LOCK_RESTRICT_SERVICES.
  *
  */
void  SE_LL_CORE_Cleanup(void)
{
#if defined(__ICCARM__)
#pragma section = ".bss"
#pragma section = ".data"
  uint8_t *begin = __section_begin(".bss");
  uint8_t *end = __section_end(".bss");
  (void)memset(begin, 0x0, (size_t)(end - begin));
  begin = __section_begin(".data");
  end = __section_end(".data");
  (void)memset(begin, 0x0, (size_t)(end - begin));
#elif defined (__CC_ARM) || defined(__ARMCC_VERSION)
  __arm_clean_data();
#elif defined(__GNUC__)
  __gcc_clean_data();
#endif /* __ICCARM__ */
}

/** @addtogroup SE_LOCK_KEYS_Exported_Functions
  * @{
  */

/**
  * @brief  Lock the embedded keys used by SBSFU
  * @param  None
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise
  */
SE_ErrorStatus SE_LL_Lock_Keys(void)
{
  return SE_SUCCESS;
}
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

