/**
  ******************************************************************************
  * @file    app_hw.h
  * @author  MCD Application Team
  * @brief   This file contains definitions for Secure Firmware Update hardware
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
#ifndef APP_HW_H
#define APP_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/* Exported macros -----------------------------------------------------------*/
#define BUTTON_INIT() do { \
                           GPIO_InitTypeDef  GPIO_InitStruct; \
                           GPIO_InitStruct.Pin = GPIO_PIN_15; \
                           GPIO_InitStruct.Pull = GPIO_PULLUP; \
                           GPIO_InitStruct.Mode = GPIO_MODE_INPUT; \
                           GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; \
                           __HAL_RCC_GPIOA_CLK_ENABLE(); \
                           HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); \
                         } while (0)
#define BUTTON_PUSHED()      (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15)\
                              == GPIO_PIN_SET)


#ifdef __cplusplus
}
#endif

#endif /* APP_HW_H */

