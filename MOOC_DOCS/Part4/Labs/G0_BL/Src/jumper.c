/**
  ******************************************************************************
  * @file    Boot/Src/jumper.c
  * @brief   This example describes how to configure and use GPIOs through
  *          the STM32L4xx HAL API.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "jumper.h"



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#if defined (__ICCARM__)
extern uint32_t __ICFEDIT_region_RAM_start__;
#define BL_SRAM1_START ((uint32_t)& __ICFEDIT_region_RAM_start__)
extern uint32_t __ICFEDIT_region_RAM_end__ ;
#define BL_SRAM1_END ((uint32_t)& __ICFEDIT_region_RAM_end__)
#elif defined (__CC_ARM)
extern uint32_t Image$$ER_IROM1$$Limit;
extern uint32_t Image$$RW_IRAM1$$Base;
extern uint32_t Image$$RW_IRAM1$$Limit;
#define BL_SRAM1_START       ((uint32_t)&Image$$RW_IRAM1$$Base)
#define BL_SRAM1_END         ((uint32_t)&Image$$RW_IRAM1$$Limit)
#elif defined(__GNUC__)
extern uint32_t _sdata;
#define BL_SRAM1_START ((uint32_t)&_sdata)
extern uint32_t _eRAM;
#define BL_SRAM1_END ((uint32_t)&_eRAM)

#endif

#define SRAM_Erase() \
  {\
    uint32_t *pRam;\
    for (pRam = (uint32_t *)BL_SRAM1_START; pRam < (uint32_t *)BL_SRAM1_END; pRam++)\
    {\
      *pRam = 0U;\
    }\
  }

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void jump_to_application(void);

  /**
  * @brief  A new UserApp Fw was available. Start to download it
  * @param  None
  * @note   This function must set the next State Machine State
  * @retval None
  */
void jump_to_application(void)
{
  uint32_t jump_address ;
  typedef void (*Function_Pointer)(void);
  Function_Pointer  p_jump_to_function;

  /* Destroy the Volatile data and CSTACK in SRAM used by Secure Boot in order to prevent any access to sensitive data
     from the loader.
  */
  SRAM_Erase();

  /* Initialize address to jump */
  jump_address = *(__IO uint32_t *)(((uint32_t)APP_REGION_ROM_START + 4));
  p_jump_to_function = (Function_Pointer) jump_address;

  /* Initialize loader's Stack Pointer */
  __set_MSP(*(__IO uint32_t *)(APP_REGION_ROM_START));

  /* Jump into loader */
  p_jump_to_function();

  /* The point below should NOT be reached */
  return;
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
