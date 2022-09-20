/**
  ******************************************************************************
  * @file    se_cm0.h
  * @author  MCD Application Team
  * @brief   This file contains definitions for communication with CM0
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SE_CM0_H
#define SE_CM0_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "tl.h"
#include "shci_tl.h"
#include "shci.h"
#include "app_conf.h"
#include "se_def_core.h"

/* Exported constants --------------------------------------------------------*/
/* AES Key index */
#define SBSFU_AES_KEY_IDX 0x01U

/* CM0 state during upgrade process */
#define CM0_UPDATE_NOT_INITIALIZED 0x00000000U
#define CM0_UPDATE_TO_BE_STARTED   0xDF23AC61U
#define CM0_UPDATE_IN_PROGRESS     0x98FCA513U
#define CM0_UPDATE_DONE            0xF234B97AU

#define CM0_UPDATE_PROCESS (*(uint32_t *)(LOADER_COM_REGION_RAM_START + 4U))

/* Exported functions -------------------------------------------------------- */
void CM0_Init(void);
void CM0_DeInit(void);
SE_ErrorStatus CM0_Update(void);


#ifdef __cplusplus
}
#endif

#endif /* SE_CM0_H */
