/**
  ******************************************************************************
  * @file    main.h
  * @author  MCD Application Team
  * @brief   This file contains definitions for main application file.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MAIN_H
#define MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include "stm32h7b3i_discovery.h"
#include "stdio.h"

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
#include "mapping_fwimg.h"
#include "mapping_sbsfu.h"
#elif defined (__ICCARM__) || defined(__GNUC__)
#include "mapping_export.h"
#endif /* __ICCARM__ || __GNUC__ */

/* Exported constants --------------------------------------------------------*/
/**
  *  Slot list : 2 slots per image configuration + swap
  */
#define NB_SLOTS      8U
#define SLOT_INACTIVE 0U     /* this index should not be used ==> no tag found in the header */
#define SLOT_ACTIVE_1 1U
#define SLOT_ACTIVE_2 2U
#define SLOT_ACTIVE_3 3U
#define SLOT_DWL_1    4U
#define SLOT_DWL_2    5U
#define SLOT_DWL_3    6U
#define SLOT_SWAP     7U


/* Calculation of the size of a slot */
#define SLOT_SIZE(a) (SlotEndAdd[a] - SlotStartAdd[a] + 1U)

/* Cortex-M7: align the init vectors on 0x400 instead of 0x200 */
#define IMAGE_OFFSET ((uint32_t)1024U)

#define LOADER_STATUS_LED (LED_BLUE)          /*!< LED to be used to provide the status to the end-user */

/* External variables --------------------------------------------------------*/
extern const uint32_t  SlotHeaderAdd[NB_SLOTS];
extern const uint32_t  SlotStartAdd[NB_SLOTS];
extern const uint32_t  SlotEndAdd[NB_SLOTS];

/* Exported macros -----------------------------------------------------------*/
/* #define DEBUG_MODE */                                 /* Debug printf on console */

#ifdef DEBUG_MODE
#define TRACE   (void) printf
#else
#define TRACE(...)
#endif /* DEBUG_MODE */

/* Exported functions ------------------------------------------------------- */

#endif /* MAIN_H */
