/*
 * app.c
 *
 *  Created on: 2022. 7. 28.
 *      Author: eziya76@gmail.com
 */

#include "app.h"
#include <stdio.h>

static void AppPrintMenu(void);
static void AppHandleMenu(void);
static void AppReadSecureMem(void);

extern UART_HandleTypeDef huart2;

void AppConfigStaticProtection(void)
{

}

void AppHashVerify(void)
{

}

void AppRun(void)
{
  AppHandleMenu();

  while(1)
  {
    HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
    HAL_Delay(100);
  }
}

//show menu
static void AppPrintMenu(void)
{
  printf("\r\n");
  printf("=================== Test Menu ===========================\r\n\n");
  printf("Test Protection: Secure User memory Read -------------- 1\r\n\n");
  printf("Previous Menu ----------------------------------------- x\r\n\n");
}

//read & handle uart input
static void AppHandleMenu(void)
{
  uint8_t key = 0U;
  uint8_t exit = 0U;

  AppPrintMenu();

  while(exit == 0U)
  {
    __HAL_UART_FLUSH_DRREGISTER(&huart2);

    HAL_UART_Receive(&huart2, &key, 1U, HAL_MAX_DELAY);

    switch (key)
    {
    case '1' :
      AppReadSecureMem();
      break;
    case 'x' :
      exit = 1U;
      break;
    default:
      printf("Invalid key!\r\n");
      break;
    }

    AppPrintMenu();
  }
}

//read secure memory
static void AppReadSecureMem(void)
{
  FLASH_OBProgramInitTypeDef flash_option_bytes;

  HAL_FLASH_Unlock();
  HAL_FLASH_OB_Unlock();

  //clear option error
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);

  //read option bytes
  flash_option_bytes.WRPArea = OB_WRPAREA_ZONE_A;
  HAL_FLASHEx_OBGetConfig(&flash_option_bytes);

  HAL_FLASH_OB_Lock();
  HAL_FLASH_Lock();

  //configure secure memory address
  volatile uint32_t *pdata[] = {(uint32_t*)0x08000000, (uint32_t*)0x080061FC};

  //print secure memory information
  printf("====== Test Protection: Secure User Memory =================\r\n\n");
  printf("If the Secure User Memory is enabled you should not be able to read the content.\r\n");
  printf("-- Secure User Area size config [0x%08lx]\r\n", (flash_option_bytes.SecSize * FLASH_PAGE_SIZE) + FLASH_BASE);
  printf("-- Flash CR SEC_PROT bit value: 0x%ld\r\n", (FLASH->CR & 0x10000000)>>28);
  printf("-- Reading from address [0x%08lx], [0x%08lx]\r\n", (uint32_t)pdata[0], (uint32_t)pdata[1]);
  printf("-- [0x%08lx]  [0x%08lx]\r\n", *pdata[0], *pdata[1]);
}
