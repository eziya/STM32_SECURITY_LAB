/**
  ******************************************************************************
  * @file    flash_if.h
  * @author  MCD Application Team
  * @brief   This file provides all the memory related operation functions.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_IF_H
#define __FLASH_IF_H

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Exported types ------------------------------------------------------------*/

/* PCROP definition */

#define PCROP_ADDR 0x080FF800 /* Last page of flash */
#define KEY_ADDR 0x080FF000 /* Last page of flash */

/* Exported constants --------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Error code */
enum
{
  FLASHIF_OK = 0,
  FLASHIF_ERASEKO,
  FLASHIF_WRITINGCTRL_ERROR,
  FLASHIF_WRITING_ERROR,
  FLASHIF_PROTECTION_ERRROR
};

/* protection type */
enum
{
  FLASHIF_PROTECTION_NONE         = 0,
  FLASHIF_PROTECTION_PCROPENABLED = 0x1,
  FLASHIF_PROTECTION_WRPENABLED   = 0x2,
  FLASHIF_PROTECTION_RDPENABLED   = 0x4,
};

/* protection update */
enum
{
  FLASHIF_WRP_ENABLE,
  FLASHIF_WRP_DISABLE
};

/* Define the address from where user application will be loaded.
   Note: this area is reserved for the IAP code                  */
#define FLASH_PAGE_STEP  FLASH_PAGE_SIZE           /* Size of page : 2 Kbytes */
#define APPLICATION_ADDRESS  (uint32_t)0x08000000      /* Code is WRP at application address */

/* Notable Flash addresses */
#define USER_FLASH_END_ADDRESS  0x08100000

/* Define the user application size */
#define USER_FLASH_SIZE  ((uint32_t)0x000011000) /* Small default template application */


/* Exported macro ------------------------------------------------------------*/
/* Absolute value */
#define ABS_RETURN(x,y)               ((x) < (y)) ? ((y)-(x)) : ((x)-(y))

/* Get the number of sectors from where the user program will be loaded */
#define FLASH_SECTOR_NUMBER           ((uint32_t)(ABS_RETURN(APPLICATION_ADDRESS,FLASH_START_BANK1))>>12)

/* Compute the mask to test if the Flash memory, where the user program will be
  loaded, is write protected */
#define FLASH_PROTECTED_SECTORS       (~(uint32_t)((1 << FLASH_SECTOR_NUMBER) - 1))

/* Exported functions ------------------------------------------------------- */
void FLASH_If_Init(void);

uint32_t FLASH_If_Erase(uint32_t StartSector);
uint32_t FLASH_If_Erase_NumPages(uint32_t start, uint32_t length);
uint32_t FLASH_If_Write(uint32_t destination, uint32_t *p_source, uint32_t length);
uint32_t GetPage(uint32_t Addr);
uint32_t GetBank(uint32_t Addr);
void flash_read_data(uint32_t start_address, uint16_t num_elements, uint8_t *output_data);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __FLASH_IF_H */
