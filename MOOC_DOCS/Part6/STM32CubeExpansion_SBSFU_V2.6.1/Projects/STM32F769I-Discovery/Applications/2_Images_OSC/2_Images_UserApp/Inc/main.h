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
#include "stm32f7xx_hal.h"
#include "stm32f769i_discovery.h"
#include "com.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* User can use this section to tailor UARTx instance used and associated
   resources */

#define BUTTON_INIT()         BSP_PB_Init(BUTTON_USER,BUTTON_MODE_GPIO);
#define BUTTON_PUSHED()      (BSP_PB_GetState(BUTTON_USER) == GPIO_PIN_SET)

/* Definition for USARTx clock resources */
#define SFU_UART                                USART1
#define SFU_UART_CLK_ENABLE()                   __HAL_RCC_USART1_CLK_ENABLE()
#define SFU_UART_CLK_DISABLE()                  __HAL_RCC_USART1_CLK_DISABLE()

#define SFU_UART_TX_AF                          GPIO_AF7_USART1
#define SFU_UART_TX_GPIO_PORT                   GPIOA
#define SFU_UART_TX_PIN                         GPIO_PIN_9
#define SFU_UART_TX_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOA_CLK_ENABLE()
#define SFU_UART_TX_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOA_CLK_DISABLE()

#define SFU_UART_RX_AF                          GPIO_AF7_USART1
#define SFU_UART_RX_GPIO_PORT                   GPIOA
#define SFU_UART_RX_PIN                         GPIO_PIN_10
#define SFU_UART_RX_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOA_CLK_ENABLE()
#define SFU_UART_RX_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOA_CLK_DISABLE()

/* Maximum Timeout values for flags waiting loops.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */
#define SFU_UART_TIMEOUT_MAX                   1000U

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* MAIN_H */
