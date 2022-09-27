/**
  ******************************************************************************
  * @file    NUCLEO_F429ZI.h
  * @author  MCD Application Team
  * @brief   This file contains definitions for NUCLEO_F429ZI's LEDs,
  *          push-buttons and COM ports hardware resources.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NUCLEO_F429ZI_H
#define __NUCLEO_F429ZI_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
   
/** @addtogroup BSP
  * @{
  */

/** @addtogroup NUCLEO_F429ZI
  * @{
  */
      
/** @addtogroup NUCLEO_F429ZI_LOW_LEVEL
  * @{
  */ 

/** @defgroup NUCLEO_F429ZI_LOW_LEVEL_Exported_Types STM32F413H DISCOVERY Low Level Exported Types
  * @{
  */
typedef enum 
{
LED1= 0,
LED_GREEN = LED1,
LED2 = 1,
LED_BLUE = LED2,
LED3 = 2,
LED_RED = LED3,
}Led_TypeDef;


typedef enum 
{  
  BUTTON_WAKEUP = 0
}Button_TypeDef;

typedef enum 
{  
  BUTTON_MODE_GPIO = 0,
  BUTTON_MODE_EXTI = 1
}ButtonMode_TypeDef;

/**
  * @}
  */ 

/** @defgroup NUCLEO_F429ZI_LOW_LEVEL_Exported_Constants STM32F413H DISCOVERY Low Level Exported Constants
  * @{
  */ 

/** 
  * @brief  Define for NUCLEO_F429ZI board
  */ 
#if !defined (USE_NUCLEO_F429ZI)
 #define USE_NUCLEO_F429ZI
#endif

/** @addtogroup NUCLEO_F429ZI_LOW_LEVEL_LED STM32F413H DISCOVERY Low Level Led
  * @{
  */
#define LEDn                             ((uint8_t)3)

#define LED1_GPIO_PORT                   GPIOB
#define LED1_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOB_CLK_ENABLE()
#define LED1_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOB_CLK_DISABLE()

#define LED2_GPIO_PORT                   GPIOB
#define LED2_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOB_CLK_ENABLE()
#define LED2_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOB_CLK_DISABLE()

#define LED3_GPIO_PORT                   GPIOB
#define LED3_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOB_CLK_ENABLE()
#define LED3_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOB_CLK_DISABLE()

#define LED1_PIN                         GPIO_PIN_0
#define LED2_PIN                         GPIO_PIN_7
#define LED3_PIN                         GPIO_PIN_14

#define LEDx_GPIO_CLK_ENABLE(__INDEX__)  do{if((__INDEX__) == 0) LED1_GPIO_CLK_ENABLE(); else \
											if((__INDEX__) == 1) LED2_GPIO_CLK_ENABLE(); else \
                                            if((__INDEX__) == 2) LED3_GPIO_CLK_ENABLE(); \
                                            }while(0)

#define LEDx_GPIO_CLK_DISABLE(__INDEX__)  do{if((__INDEX__) == 0) LED1_GPIO_CLK_DISABLE(); else \
											 if((__INDEX__) == 1) LED2_GPIO_CLK_DISABLE(); else \
											 if((__INDEX__) == 2) LED3_GPIO_CLK_DISABLE(); \
                                             }while(0)
/**
  * @}
  */ 
  
/** @addtogroup NUCLEO_F429ZI_LOW_LEVEL_BUTTON STM32F413H DISCOVERY Low Level Button
  * @{
  */ 
/* Only one User/Wakeup button */
#define BUTTONn                             ((uint8_t)1)

/**
  * @brief Wakeup push-button
  */
#define WAKEUP_BUTTON_PIN                   GPIO_PIN_13
#define WAKEUP_BUTTON_GPIO_PORT             GPIOC
#define WAKEUP_BUTTON_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOC_CLK_ENABLE()
#define WAKEUP_BUTTON_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOC_CLK_DISABLE()
#define WAKEUP_BUTTON_EXTI_IRQn             EXTI15_10_IRQn

/**
  * @}
  */ 


/** @defgroup NUCLEO_F429ZI_LOW_LEVEL_Exported_Functions STM32F413H DISCOVERY Low Level Exported Functions
  * @{
  */
uint32_t         BSP_GetVersion(void);  
void             BSP_LED_Init(Led_TypeDef Led);
void             BSP_LED_DeInit(Led_TypeDef Led);
void             BSP_LED_On(Led_TypeDef Led);
void             BSP_LED_Off(Led_TypeDef Led);
void             BSP_LED_Toggle(Led_TypeDef Led);
void             BSP_PB_Init(Button_TypeDef Button, ButtonMode_TypeDef ButtonMode);
void             BSP_PB_DeInit(Button_TypeDef Button);
uint32_t         BSP_PB_GetState(Button_TypeDef Button);

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

#ifdef __cplusplus
}
#endif

#endif /* __NUCLEO_F429ZI_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
