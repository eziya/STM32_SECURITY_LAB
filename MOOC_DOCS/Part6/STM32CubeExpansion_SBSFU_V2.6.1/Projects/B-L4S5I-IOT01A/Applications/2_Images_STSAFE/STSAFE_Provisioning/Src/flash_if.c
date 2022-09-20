/**
  ******************************************************************************
  * @file    flash_if.c
  * @author  SMD Application Team
  * @brief   This file provides the Flash memory related operation functions.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file in
  * the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/** @addtogroup STM32L4xx_Flash_Management
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "flash_if.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Gets the page of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The page of a given address
  */
uint32_t GetPage(uint32_t Addr)
{
  uint32_t page = 0U;

  page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE_128_BITS;

  return page;
}

/**
  * @brief  Gets the bank of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The bank of a given address
  */
uint32_t GetBank(uint32_t Addr)
{
  uint32_t bank = 0;

  if (READ_BIT(SYSCFG->MEMRMP, SYSCFG_MEMRMP_FB_MODE) == 0)
  {
    /* No Bank swap */
    if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
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
    if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
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
  * @brief  Read flash at given address
  * @param_in : start_address : Address of the STM32 FLASH Memory
  * @param_out : *output_data : flash contents
  * @retval void
  */

void flash_read_data(uint32_t start_address, uint16_t num_elements, uint8_t *output_data)
{
  uint16_t iter;
  uint32_t *memory_ptr = (uint32_t *)start_address;

  for (iter = 0; iter < num_elements / 4; iter++)
  {
    *(uint32_t *)output_data = *(memory_ptr + iter);
    output_data += 4;
  }
}

/**
  * @}
  */
