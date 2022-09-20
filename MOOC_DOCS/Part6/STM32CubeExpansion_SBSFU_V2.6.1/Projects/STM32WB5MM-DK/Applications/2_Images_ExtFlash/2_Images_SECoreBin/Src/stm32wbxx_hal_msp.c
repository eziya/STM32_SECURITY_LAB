/**
  ******************************************************************************
  * @file    stm32wbxx_hal_msp.c
  * @author  MCD Application Team
  * @brief   This file provides code for the MSP Initialization
  *          and de-Initialization codes.
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
#include "se_low_level.h"

/**
  * Initializes the Global MSP.
  */

/**
  * @brief CRYP MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hcryp: CRYP handle pointer
  * @retval None
  */
void HAL_CRYP_MspInit(CRYP_HandleTypeDef *hcryp)
{
  if (hcryp->Instance == AES1)
  {
    /* Peripheral clock enable */
    __HAL_RCC_AES1_CLK_ENABLE();
  }
}

/**
  * @brief AES MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param haes: AES handle pointer
  * @retval None
  */
void HAL_CRYP_MspDeInit(CRYP_HandleTypeDef *hcryp)
{
  if (hcryp->Instance == AES1)
  {
    /* Peripheral clock disable */
    __HAL_RCC_AES1_CLK_DISABLE();
  }
}

/**
  * @brief PKA MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hpka: PKA handle pointer
  * @retval None
  */
void HAL_PKA_MspInit(PKA_HandleTypeDef *hpka)
{
  if (hpka->Instance == PKA)
  {
    /* Peripheral clock enable */
    __HAL_RCC_PKA_CLK_ENABLE();
  }
}

/**
  * @brief PKA MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hpka: PKA handle pointer
  * @retval None
  */
void HAL_PKA_MspDeInit(PKA_HandleTypeDef *hpka)
{
  if (hpka->Instance == PKA)
  {
    /* Enable PKA reset state */
    __HAL_RCC_PKA_FORCE_RESET();

    /* Release PKA from reset state */
    __HAL_RCC_PKA_RELEASE_RESET();

    /* Peripheral clock disable */
    __HAL_RCC_PKA_CLK_DISABLE();
  }
}

/**
  * @}
  */

/**
  * @}
  */

