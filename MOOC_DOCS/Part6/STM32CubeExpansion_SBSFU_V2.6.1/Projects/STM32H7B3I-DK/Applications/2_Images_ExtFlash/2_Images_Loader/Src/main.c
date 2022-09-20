/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @brief   Main application file.
  *          This application demonstrates Firmware Update, protections
  *          and crypto testing functionalities.
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

#define MAIN_C

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "loader.h"
#include "loader_low_level.h"
#include "loader_low_level_flash.h"
#include "sfu_standalone_loader.h"            /* Standalone loader states definition in SHARED RAM */

/* Global variables ----------------------------------------------------------*/
/* List of slot header address */
const uint32_t  SlotHeaderAdd[NB_SLOTS] = { 0U,
                                            SLOT_ACTIVE_1_HEADER,
                                            SLOT_ACTIVE_2_HEADER,
                                            SLOT_ACTIVE_3_HEADER,
                                            SLOT_DWL_1_START,
                                            SLOT_DWL_2_START,
                                            SLOT_DWL_3_START,
                                            SWAP_START,
                                          };
/* List of slot start address */
const uint32_t  SlotStartAdd[NB_SLOTS]  = { 0U,
                                            SLOT_ACTIVE_1_START,
                                            SLOT_ACTIVE_2_START,
                                            SLOT_ACTIVE_3_START,
                                            SLOT_DWL_1_START,
                                            SLOT_DWL_2_START,
                                            SLOT_DWL_3_START,
                                            SWAP_START,
                                          };
/* List of slot end address */
const uint32_t  SlotEndAdd[NB_SLOTS]    = { 0U,
                                            SLOT_ACTIVE_1_END,
                                            SLOT_ACTIVE_2_END,
                                            SLOT_ACTIVE_3_END,
                                            SLOT_DWL_1_END,
                                            SLOT_DWL_2_END,
                                            SLOT_DWL_3_END,
                                            SWAP_END,
                                          };

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void CPU_CACHE_Enable(void);

/* Functions Definition ------------------------------------------------------*/
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  LOADER_StatusTypeDef e_ret_status;
  uint32_t rx_size;
  uint32_t dwl_type;
  uint32_t dwl_slot;

  
  /* Enable the CPU Cache */
  CPU_CACHE_Enable();

  /* STM32H7xx HAL library initialization:
  - Configure the Flash prefetch
  - Systick timer is configured by default as source of time base, but user
  can eventually implement his proper time base source (a general purpose
  timer for example or other time source), keeping in mind that Time base
  duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
  handled in milliseconds basis.
  - Set NVIC Group Priority to 4
  - Low Level Initialization
  */
  (void) HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* LED Init*/
  (void) BSP_LED_Init(LOADER_STATUS_LED);

  /* Initialize external flash interface (OSPI/QSPI) */
  (void) LOADER_LL_FLASH_Init();

  /* If the SecureBoot configured the IWDG, loader must reload IWDG counter with value defined in the reload register */
  WRITE_REG(IWDG1->KR, IWDG_KEY_RELOAD);

  /* Configure Communication module */
  (void) LOADER_LL_UART_Init();

  TRACE("\r\n======================================================================");
  TRACE("\r\n=                           Loader                                   =");
  TRACE("\r\n======================================================================");
  TRACE("\r\n");

  /* Loader initialization */
  e_ret_status = LOADER_Init();
  if (e_ret_status != LOADER_OK)
  {
    TRACE("Initialization failure : reset !");
    NVIC_SystemReset();
  }

  /* Standalone loader communication : do not memorize the DWL request
     to avoid to be stuck in DWL state in case of reset */
  STANDALONE_LOADER_STATE = STANDALONE_LOADER_NO_REQ;

  /* Download new firmware */
  e_ret_status = LOADER_DownloadNewUserFw(&rx_size, &dwl_slot, &dwl_type);

  if (e_ret_status == LOADER_OK)
  {
    /* dwl_type not used in this example */
    UNUSED(dwl_type);

    /* Standalone loader communication : FW installation requested */
    STANDALONE_LOADER_STATE = STANDALONE_LOADER_INSTALL_REQ;
    TRACE("\r\nDownload successful : %d bytes received\r\n", rx_size);
  }
  else
  {
    TRACE("\r\nDownload failed (%d)\r\n", e_ret_status);
    (void) LOADER_LL_FLASH_Erase_Size((uint8_t *) SlotStartAdd[dwl_slot], SLOT_SIZE(dwl_slot));
  }

  /* Reset to restart SBSFU */
  NVIC_SystemReset();
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 280000000 (CPU Clock)
  *            HCLK(Hz)                       = 280000000 (Bus matrix and AHBs Clock)
  *            AHB Prescaler                  = 1
  *            CD APB3 Prescaler              = 2 (APB3 Clock  140MHz)
  *            CD APB1 Prescaler              = 2 (APB1 Clock  140MHz)
  *            CD APB2 Prescaler              = 2 (APB2 Clock  140MHz)
  *            SRD APB4 Prescaler             = 2 (APB4 Clock  140MHz)
  *            HSE Frequency(Hz)              = 24000000
  *            PLL_M                          = 12
  *            PLL_N                          = 280
  *            PLL_P                          = 2
  *            PLL_Q                          = 2
  *            PLL_R                          = 2
  *            VDD(V)                         = 3.3
  *            Flash Latency(WS)              = 6
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;

  /*!< Supply configuration update enable */
  HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

  /* The voltage scaling allows optimizing the power consumption when the device is
  clocked below the maximum system frequency, to update the voltage scaling value
  regarding system frequency refer to product datasheet.
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 280;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;

  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1;
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
  while(1) {};
  }

  /* Select PLL as system clock source and configure  bus clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_PCLK1 | \
    RCC_CLOCKTYPE_PCLK2  | RCC_CLOCKTYPE_D3PCLK1);

  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6);
  if(ret != HAL_OK)
  {
  while(1) {};
  }
}


/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: TRACE("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */
