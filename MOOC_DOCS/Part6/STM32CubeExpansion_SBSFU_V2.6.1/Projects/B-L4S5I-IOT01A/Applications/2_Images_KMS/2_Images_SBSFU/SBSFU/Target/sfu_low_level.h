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
#ifndef  SFU_LOW_LEVEL_H
#define  SFU_LOW_LEVEL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "sfu_def.h"

/* Exported types ------------------------------------------------------------*/
/**
  * @brief  SFU CRC Configuration definition
  */
typedef enum
{
  SFU_CRC_CONFIG_NONE = 0U,   /*!< None */
  SFU_CRC_CONFIG_DEFAULT,     /*!< Default configuration */
  SFU_CRC_CONFIG_16BIT        /*!< 16 bit configuration */
} SFU_CRC_ConfigTypeDef;

/* Exported constants --------------------------------------------------------*/
#define SFU_UART                                USART1
#define SFU_UART_CLK_ENABLE()                   __HAL_RCC_USART1_CLK_ENABLE()
#define SFU_UART_CLK_DISABLE()                  __HAL_RCC_USART1_CLK_DISABLE()

#define SFU_UART_TX_AF                          GPIO_AF7_USART1
#define SFU_UART_TX_GPIO_PORT                   GPIOB
#define SFU_UART_TX_PIN                         GPIO_PIN_6
#define SFU_UART_TX_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOB_CLK_ENABLE()
#define SFU_UART_TX_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOB_CLK_DISABLE()

#define SFU_UART_RX_AF                          GPIO_AF7_USART1
#define SFU_UART_RX_GPIO_PORT                   GPIOB
#define SFU_UART_RX_PIN                         GPIO_PIN_7
#define SFU_UART_RX_GPIO_CLK_ENABLE()           __HAL_RCC_GPIOB_CLK_ENABLE()
#define SFU_UART_RX_GPIO_CLK_DISABLE()          __HAL_RCC_GPIOB_CLK_DISABLE()

/* Uncomment to enable the adequate Clock Source */
#define RTC_CLOCK_SOURCE_LSI
/*#define RTC_CLOCK_SOURCE_LSE*/

#ifdef RTC_CLOCK_SOURCE_LSI
#define RTC_ASYNCH_PREDIV    0x7FU
#define RTC_SYNCH_PREDIV     0xF9
#endif /* RTC_CLOCK_SOURCE_LSI */

#ifdef RTC_CLOCK_SOURCE_LSE
#define RTC_ASYNCH_PREDIV  0x7FU
#define RTC_SYNCH_PREDIV   0x00FFU
#endif /* RTC_CLOCK_SOURCE_LSE */

/* External variables --------------------------------------------------------*/
extern __IO uint32_t  DummyMemAccess;
#if defined(SFU_LOW_LEVEL_C)
/*
 * See errata sheet: ES0393 - Rev 6 - October 2019
 * DummyMemAccess variable used in se_interface_exception_XXX.c to force data access outside FWALL protected SRAM1
 * and outside the 18 LSB range protected by FWALL.
 */
#if defined ( __ICCARM__ )
__IO uint32_t DummyMemAccess @ ".SB_dummy_mem_access";
#elif defined ( __CC_ARM ) || defined(__GNUC__) || defined(__ARMCC_VERSION)
__IO uint32_t DummyMemAccess __attribute__((section(".SB_dummy_mem_access")));
#endif /* defined ( __ICCARM__ )*/
#endif /* defined(SFU_LOW_LEVEL_C) */


/* Exported macros -----------------------------------------------------------*/
#define IS_SFU_CRC_CONF(CONF) (((CONF) == SFU_CRC_CONFIG_NONE) || \
                               ((CONF) == SFU_CRC_CONFIG_DEFAULT) || \
                               ((CONF) == SFU_CRC_CONFIG_16BIT))

/*
 * See errata sheet: ES0393 - Rev 6 - October 2019
 * DummyMemAccess variable used in se_interface_exception_XXX.c to force data access outside FWALL protected SRAM1
 * and outside the 18 LSB range protected by FWALL.
 */
#define SFU_LL_DummyAccess() \
  do{ \
    DummyMemAccess = 0xAAAAAAAA; \
  }while(0)


/* Exported functions ------------------------------------------------------- */
SFU_ErrorStatus    SFU_LL_Init(void);
SFU_ErrorStatus    SFU_LL_DeInit(void);

SFU_ErrorStatus    SFU_LL_CRC_Init(void);
SFU_ErrorStatus    SFU_LL_CRC_Config(SFU_CRC_ConfigTypeDef eCRCConfg);
uint32_t           SFU_LL_CRC_Calculate(uint32_t pBuffer[], uint32_t BufferLength);

SFU_ErrorStatus    SFU_LL_UART_Init(void);
SFU_ErrorStatus    SFU_LL_UART_DeInit(void);
SFU_ErrorStatus    SFU_LL_UART_Transmit(uint8_t *pData, uint16_t DataLength, uint32_t Timeout);
SFU_ErrorStatus    SFU_LL_UART_Receive(uint8_t *pData, uint16_t DataLength, uint32_t Timeout);
SFU_ErrorStatus    SFU_LL_UART_Flush(void);

#ifndef SFU_TAMPER_PROTECT_ENABLE
SFU_ErrorStatus SFU_LL_RTC_Init(void);
SFU_ErrorStatus SFU_LL_RTC_DeInit(void);
#endif /*SFU_TAMPER_PROTECT_ENABLE*/

void  SFU_LL_RTC_MspInit(RTC_HandleTypeDef *hrtc);
void  SFU_LL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc);
void  SFU_LL_UART_MspInit(UART_HandleTypeDef *huart);
void  SFU_LL_UART_MspDeInit(UART_HandleTypeDef *huart);


#ifdef __cplusplus
}
#endif

#endif /*  SFU_LOW_LEVEL_H */
