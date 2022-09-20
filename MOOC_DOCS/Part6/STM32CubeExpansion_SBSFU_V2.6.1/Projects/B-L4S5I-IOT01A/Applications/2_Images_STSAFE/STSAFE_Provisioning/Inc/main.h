/* USER CODE BEGIN Header */

/**
  ******************************************************************************
  * @file    main.h
  * @author  MCD Application Team
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* USER CODE END Header */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include <string.h>
#include "stsafea_core.h"
#include "Commonappli.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

#define ST_LINK_UART1_TX_PIN GPIO_PIN_6
#define ST_LINK_UART1_TX_GPIO_PORT GPIOB
#define ST_LINK_UART1_RX_PIN GPIO_PIN_7
#define ST_LINK_UART1_RX_GPIO_PORT GPIOB

/* USER CODE BEGIN Private defines */

#define ZONE_CERT_CA_FIRMW 2
#define ZONE_CERT_OEM_FIRMW 3
#define DEFAULT_KEYS 0  /* use default keys and meanwhile set buf = NULL */
#define RANDOM_KEYS 1   /* use random generated keys and meanwhile set buf = NULL */
#define USER_KEYS 2   /* use the keys customers prefer and meanwhile set buf = preferred key */
#define SIZE_HOST_KEYS 32
#define UART_SPEED 115200

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MAIN_H */
