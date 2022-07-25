/**
  ******************************************************************************
  * @file    App/Src/app.c
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
#include "app.h"
#include "com.h"
#include <stdio.h>    
    
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define RX_TIMEOUT          ((uint32_t)2000U)
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static GPIO_InitTypeDef  GPIO_InitStruct = {0};

/* Private function prototypes -----------------------------------------------*/
static void TEST_PROTECTIONS_PrintTestingMenu(void);
static void TEST_PROTECTIONS_Read_SecUserMem(void);
static void APP_Test_Prot_Run(void);
/* Private functions ---------------------------------------------------------*/

  /**
  * @brief  Initialize COM port
  * @param  None
  * @retval None
  */
void APP_Init(void)
{
  /* -1- Enable GPIO Clock (to be able to program the configuration registers) */
  LED4_GPIO_CLK_ENABLE();

  /* -2- Configure IO in output push-pull mode to drive external LEDs */
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

  GPIO_InitStruct.Pin = LED4_PIN;
  HAL_GPIO_Init(LED4_GPIO_PORT, &GPIO_InitStruct);
  
  printf("\r\n ========= %s ==========\r\n", __FUNCTION__);
  
  return;
}

  /**
  * @brief  Application will do GPIO init and toggle the LED
  * @param  None
  * @retval None
  */
void APP_Run(void)
{
  printf("\r\n ========= %s ==========\r\n", __FUNCTION__);
  APP_Test_Prot_Run();
  
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    HAL_GPIO_TogglePin(LED4_GPIO_PORT, LED4_PIN);
    /* Insert delay 100 ms */
    HAL_Delay(100);

  }
}

  /**
  * @brief  Print Test Menu
  * @param  None
  * @retval None
  */
static void TEST_PROTECTIONS_PrintTestingMenu(void)
{
  printf("\r\n=================== Test Menu ============================\r\n\n");
  printf("  Test Protection: Secure User memory Read -------------- 1\r\n\n");
  printf("  Previous Menu ----------------------------------------- x\r\n\n");
}

  /**
  * @brief  Test read secure user memory area from application
  * @param  None
  * @retval None
  */
static void TEST_PROTECTIONS_Read_SecUserMem(void)
{
  FLASH_OBProgramInitTypeDef flash_option_bytes;
  
  /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();

  /* Clear OPTVERR bit set on virgin samples */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);

  /* Unlock the Options Bytes *************************************************/
  HAL_FLASH_OB_Unlock();

  /* Get Option Bytes status for WRP AREA_A  **********/
  flash_option_bytes.WRPArea     = OB_WRPAREA_ZONE_A;
  HAL_FLASHEx_OBGetConfig(&flash_option_bytes);
  
  /* Lock the Options Bytes ***************************************************/
  HAL_FLASH_OB_Lock();

  /* Lock the Flash to disable the flash control register access (recommended
  to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();

  /* 128 bit key + 1 char for NULL-terminated string */
  volatile uint32_t *pdata[] = {(uint32_t*)0x08000000, (uint32_t*)0x080061FC};
  
  printf("\r\n====== Test Protection: Secure User Memory =================\r\n\n");
  printf("If the Secure User Memory is enabled you should not be able to read the content.\r\n\n");
  printf("  -- Secure User Area size config [0x%08x]\r\n\n", (flash_option_bytes.SecSize * FLASH_PAGE_SIZE) + FLASH_BASE);
  printf("  -- Flash CR SEC_PROT bit value: 0x%d\r\n\n", (FLASH->CR & 0x10000000)>>28);
  
  printf("  -- Reading from address [0x%08x], [0x%08x]\r\n\n", pdata[0], pdata[1]);
  printf("\r\t [0x%08x]  [0x%08x]\r\n\n", *pdata[0], *pdata[1]);
  
  return;
}

void APP_Test_Prot_Run(void)
{
  uint8_t key = 0U;
  uint8_t exit = 0U;

  /* Print Main Menu message*/
  TEST_PROTECTIONS_PrintTestingMenu();

  while (exit == 0U)
  {
    key = 0U;

    /* Clean the input path */
    COM_Flush();

    /* Receive key */
    if (COM_Receive(&key, 1U, RX_TIMEOUT) == HAL_OK)
    {
      switch (key)
      {
        case '1' :
          TEST_PROTECTIONS_Read_SecUserMem();
          break;
        case 'x' :
          exit = 1U;
          break;

        default:
          printf("Invalid Number !\r");
          break;
      }

      /*Print Main Menu message*/
      TEST_PROTECTIONS_PrintTestingMenu();
    }
  }
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
