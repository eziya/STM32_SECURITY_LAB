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
#include "stm32l4xx_hal.h"
#include "stm32l4xx_nucleo_32.h"
#include "com.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* User can use this section to tailor UARTx instance used and associated
   resources */

#define BUTTON_INIT() do { \
                           GPIO_InitTypeDef  GPIO_InitStruct; \
                           GPIO_InitStruct.Pin = GPIO_PIN_12; \
                           GPIO_InitStruct.Pull = GPIO_PULLUP; \
                           GPIO_InitStruct.Mode = GPIO_MODE_INPUT; \
                           GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; \
                           __HAL_RCC_GPIOA_CLK_ENABLE(); \
                           HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); \
                         } while (0)
#define BUTTON_PUSHED()      (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12)\
                              == GPIO_PIN_SET)

/* Definition for USARTx clock resources */
#define SFU_UART                                USART2
#define SFU_UART_CLK_ENABLE()                   __HAL_RCC_USART2_CLK_ENABLE()
#define SFU_UART_CLK_DISABLE()                  __HAL_RCC_USART2_CLK_DISABLE()

#define SFU_UART_TX_AF                          GPIO_AF7_USART2
#define SFU_UART_TX_GPIO_PORT                   GPIOA
#define SFU_UART_TX_PIN                         GPIO_PIN_2
#define SFU_UART_TX_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOA_CLK_ENABLE()
#define SFU_UART_TX_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOA_CLK_DISABLE()

#define SFU_UART_RX_AF                          GPIO_AF3_USART2
#define SFU_UART_RX_GPIO_PORT                   GPIOA
#define SFU_UART_RX_PIN                         GPIO_PIN_15
#define SFU_UART_RX_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOA_CLK_ENABLE()
#define SFU_UART_RX_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOA_CLK_DISABLE()

/* Maximum Timeout values for flags waiting loops.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */
#define SFU_UART_TIMEOUT_MAX                   1000U

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* MAIN_H */
