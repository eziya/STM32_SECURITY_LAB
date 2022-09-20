/**
  ******************************************************************************
  * @file    sfu_low_level.h
  * @author  MCD Application Team
  * @brief   This file contains definitions for Secure Firmware Update low level
  *          interface.
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
#ifndef  LOADER_LOW_LEVEL_H
#define  LOADER_LOW_LEVEL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported constants --------------------------------------------------------*/
#define LOADER_UART                                USART1
#define LOADER_UART_CLK_ENABLE()                   __HAL_RCC_USART1_CLK_ENABLE()
#define LOADER_UART_CLK_DISABLE()                  __HAL_RCC_USART1_CLK_DISABLE()

#define LOADER_UART_TX_AF                          GPIO_AF7_USART1
#define LOADER_UART_TX_GPIO_PORT                   GPIOA
#define LOADER_UART_TX_PIN                         GPIO_PIN_9
#define LOADER_UART_TX_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOA_CLK_ENABLE()
#define LOADER_UART_TX_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOA_CLK_DISABLE()

#define LOADER_UART_RX_AF                          GPIO_AF7_USART1
#define LOADER_UART_RX_GPIO_PORT                   GPIOA
#define LOADER_UART_RX_PIN                         GPIO_PIN_10
#define LOADER_UART_RX_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOA_CLK_ENABLE()
#define LOADER_UART_RX_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOA_CLK_DISABLE()

/* Redirect of RTC Tamper Event Callback */
#define  CALLBACK_Antitamper                       HAL_RTCEx_Tamper3EventCallback

/* Exported functions ------------------------------------------------------- */
HAL_StatusTypeDef    LOADER_LL_CRC_Init(void);
uint32_t             LOADER_LL_CRC_Calculate(uint32_t pBuffer[], uint32_t BufferLength);

HAL_StatusTypeDef    LOADER_LL_UART_Init(void);
HAL_StatusTypeDef    LOADER_LL_UART_DeInit(void);
HAL_StatusTypeDef    LOADER_LL_UART_Transmit(uint8_t *pData, uint16_t DataLength, uint32_t Timeout);
HAL_StatusTypeDef    LOADER_LL_UART_Receive(uint8_t *pData, uint16_t DataLength, uint32_t Timeout);
HAL_StatusTypeDef    LOADER_LL_UART_Flush(void);

#ifdef __cplusplus
}
#endif

#endif /*  LOADER_LOW_LEVEL_H */
