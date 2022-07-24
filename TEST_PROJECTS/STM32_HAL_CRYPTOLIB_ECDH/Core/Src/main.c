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

//original test case
//pub key = 700c48f77f56584c5cc632ca65640db91b6bacce3a4df6b42ce7cc838833d287db71e509e3fd9b060ddb20ba5c51dcc5948d46fbf640dfe0441782cab85fa4ac
//priv key = 7d7dc5f71eb29ddaf80d6214632eeae03d9058af1fb6d22ed80badb62bc1a534
//secret = 46fc62106420ff012e54a434fbdd2d25ccc5852060561e68040dd7778997bd7b

//my test case
//alice_priv = 4b30e7bad96997b3e7803e9c1a8be4d98fdc73676b8fa15727e181457fd78f80
//bob_pub = 81e4a63a537d717aac12f2c2ccf81853433060179a07020aeb33be99552fe0b12a4cf3d0ec8b0191c62cf19521dedd48c7fca5d363eb1e79165ef73fb6053829
//secret = 0e154aafacdcd686fc35279299ff0b9f335ad24ab26e9d9ec2acbaf9fb3a9a89

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
    0x4b, 0x30, 0xe7, 0xba, 0xd9, 0x69, 0x97, 0xb3, 0xe7, 0x80, 0x3e, 0x9c, 0x1a, 0x8b, 0xe4, 0xd9,
    0x8f, 0xdc, 0x73, 0x67, 0x6b, 0x8f, 0xa1, 0x57, 0x27, 0xe1, 0x81, 0x45, 0x7f, 0xd7, 0x8f, 0x80
};

const uint8_t remotePubKey[] =
{
    0x81, 0xe4, 0xa6, 0x3a, 0x53, 0x7d, 0x71, 0x7a, 0xac, 0x12, 0xf2, 0xc2, 0xcc, 0xf8, 0x18, 0x53,
    0x43, 0x30, 0x60, 0x17, 0x9a, 0x07, 0x02, 0x0a, 0xeb, 0x33, 0xbe, 0x99, 0x55, 0x2f, 0xe0, 0xb1,
    0x2a, 0x4c, 0xf3, 0xd0, 0xec, 0x8b, 0x01, 0x91, 0xc6, 0x2c, 0xf1, 0x95, 0x21, 0xde, 0xdd, 0x48,
    0xc7, 0xfc, 0xa5, 0xd3, 0x63, 0xeb, 0x1e, 0x79, 0x16, 0x5e, 0xf7, 0x3f, 0xb6, 0x05, 0x38, 0x29
};

const uint8_t expectedSecret[] =
{
    0x0e, 0x15, 0x4a, 0xaf, 0xac, 0xdc, 0xd6, 0x86, 0xfc, 0x35, 0x27, 0x92, 0x99, 0xff, 0x0b, 0x9f,
    0x33, 0x5a, 0xd2, 0x4a, 0xb2, 0x6e, 0x9d, 0x9e, 0xc2, 0xac, 0xba, 0xf9, 0xfb, 0x3a, 0x9a, 0x89
};

//openssl returns 32 bytes not 64 bytes
uint8_t sharedSecret[CMOX_ECC_SECP256R1_SECRET_LEN] = {0,};

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

  //openssl returns 32 bytes not 64 bytes
  printHexArray(sharedSecret, sizeof(expectedSecret));
  printf("\r\n");

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
  printHexArray((uint8_t*)myPrivKey, sizeof(myPrivKey));
  printf("\r\n");

  printf("Print remotePubKey.\r\n");
  printHexArray((uint8_t*)remotePubKey, sizeof(remotePubKey));
  printf("\r\n");

  printf("Print expectedSecret.\r\n");
  printHexArray((uint8_t*)expectedSecret, sizeof(expectedSecret));
  printf("\r\n");

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
