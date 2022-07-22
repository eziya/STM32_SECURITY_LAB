/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "cmox_crypto.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

int __io_putchar (int ch)
{
  HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

void printHexArray(uint8_t* arr, size_t len)
{
  for(size_t i = 0; i < len; i++)
  {
    printf("%02X ", arr[i]);
  }

  printf("\r\n");
}

cmox_ecc_handle_t hECC;
uint8_t eccBuffer[2000];

const uint8_t myPrivKey[] =
{
    0x7d, 0x7d, 0xc5, 0xf7, 0x1e, 0xb2, 0x9d, 0xda, 0xf8, 0x0d, 0x62, 0x14, 0x63, 0x2e, 0xea, 0xe0,
    0x3d, 0x90, 0x58, 0xaf, 0x1f, 0xb6, 0xd2, 0x2e, 0xd8, 0x0b, 0xad, 0xb6, 0x2b, 0xc1, 0xa5, 0x34
};

const uint8_t remotePubKey[] =
{
    0x70, 0x0c, 0x48, 0xf7, 0x7f, 0x56, 0x58, 0x4c, 0x5c, 0xc6, 0x32, 0xca, 0x65, 0x64, 0x0d, 0xb9,
    0x1b, 0x6b, 0xac, 0xce, 0x3a, 0x4d, 0xf6, 0xb4, 0x2c, 0xe7, 0xcc, 0x83, 0x88, 0x33, 0xd2, 0x87,
    0xdb, 0x71, 0xe5, 0x09, 0xe3, 0xfd, 0x9b, 0x06, 0x0d, 0xdb, 0x20, 0xba, 0x5c, 0x51, 0xdc, 0xc5,
    0x94, 0x8d, 0x46, 0xfb, 0xf6, 0x40, 0xdf, 0xe0, 0x44, 0x17, 0x82, 0xca, 0xb8, 0x5f, 0xa4, 0xac
};

const uint8_t expectedSecret[] =
{
    0x46, 0xfc, 0x62, 0x10, 0x64, 0x20, 0xff, 0x01, 0x2e, 0x54, 0xa4, 0x34, 0xfb, 0xdd, 0x2d, 0x25,
    0xcc, 0xc5, 0x85, 0x20, 0x60, 0x56, 0x1e, 0x68, 0x04, 0x0d, 0xd7, 0x77, 0x89, 0x97, 0xbd, 0x7b
};

uint8_t sharedSecret[CMOX_ECC_SECP256R1_SECRET_LEN];


bool MakeECDHSecret(void)
{
  cmox_ecc_retval_t retVal;
  size_t outputLen;

  printf("Testing MakeECDHSecret.\r\n");

  //allocate resources
  cmox_ecc_construct(&hECC, CMOX_ECC256_MATH_FUNCS, eccBuffer, sizeof(eccBuffer));

  //make ECDH shared secret
  retVal = cmox_ecdh(
      &hECC,                      //ECC handle
      CMOX_ECC_CURVE_SECP256R1,   //SECP256R1 ECC curve
      myPrivKey,                  //private key
      sizeof(myPrivKey),          //size of private key
      remotePubKey,               //remote public key
      sizeof(remotePubKey),       //size of remote public key
      sharedSecret,               //secret buffer pointer
      &outputLen);                //size of secret

  //check the result
  if( (retVal != CMOX_ECC_SUCCESS) ||
      (outputLen != sizeof(sharedSecret)) ||
      (memcmp(sharedSecret, expectedSecret, sizeof(expectedSecret)) != 0)) return false;

  //clean up resources
  cmox_ecc_cleanup(&hECC);

  printHexArray(sharedSecret, sizeof(sharedSecret));

  return true;
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  printf("Print myPrivKey.\r\n");
  printHexArray(myPrivKey, sizeof(myPrivKey));

  printf("Print remotePubKey.\r\n");
  printHexArray(remotePubKey, sizeof(remotePubKey));

  printf("Print expectedSecret.\r\n");
  printHexArray(expectedSecret, sizeof(expectedSecret));

  /* Initialize cryptographic library */
  if(cmox_initialize(NULL) != CMOX_INIT_SUCCESS)
  {
    Error_Handler();
  }

  //test ECDH
  if(!MakeECDHSecret())
  {
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
  }

  /* No more need of cryptographic services, finalize cryptographic library */
  if (cmox_finalize(NULL) != CMOX_INIT_SUCCESS)
  {
    Error_Handler();
  }


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
      |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
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
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
