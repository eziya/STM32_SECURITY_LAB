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
#include "rng.h"
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

void printHexArray(const uint8_t* arr, const size_t len)
{
  for(size_t i = 0; i < len; i++)
  {
    printf("%02X ", arr[i]);
  }

  printf("\r\n");
}

/** Extract from SigGen.txt

ECDSA signing algorithm calculates a message's hash, then generates a random integer k and calculates the signature (a pair of integers {R, S}).
R is calculated from k, and S is calculated using the message hash + the private key + the random number k.
So, the signature is non-deterministic due to a randomness.

[P-256,SHA-224]
Msg              = ff624d0ba02c7b6370c1622eec3fa2186ea681d1659e0a845448e777b75a8e77a77bb26e5733179d58ef9bc8a4e8b6971aef2539f77ab0963a3415bbd6258339bd1bf55de65db520c63f5b8eab3d55debd05e9494212170f5d65b3286b8b668705b1e2b2b5568610617abb51d2dd0cb450ef59df4b907da90cfa7b268de8c4c2
private key      = 708309a7449e156b0db70e5b52e606c7e094ed676ce8953bf6c14757c826f590
public key       = 29578c7ab6ce0d11493c95d5ea05d299d536801ca9cbd50e9924e43b733b83ab08c8049879c6278b2273348474158515accaa38344106ef96803c5a05adc4800
k(random number) = 58f741771620bdc428e91a32d86d230873e9140336fcfb1e122892ee1d501bdc
R,S(signature)   = 4a19274429e40522234b8785dc25fc524f179dcc95ff09b3c9770fc71f54ca0d58982b79a65b7320f5b92d13bdaecdd1259e760f0f718ba933fd098f6f75d4b7
*/

//message
const uint8_t message[] =
{
    0xff, 0x62, 0x4d, 0x0b, 0xa0, 0x2c, 0x7b, 0x63, 0x70, 0xc1, 0x62, 0x2e, 0xec, 0x3f, 0xa2, 0x18,
    0x6e, 0xa6, 0x81, 0xd1, 0x65, 0x9e, 0x0a, 0x84, 0x54, 0x48, 0xe7, 0x77, 0xb7, 0x5a, 0x8e, 0x77,
    0xa7, 0x7b, 0xb2, 0x6e, 0x57, 0x33, 0x17, 0x9d, 0x58, 0xef, 0x9b, 0xc8, 0xa4, 0xe8, 0xb6, 0x97,
    0x1a, 0xef, 0x25, 0x39, 0xf7, 0x7a, 0xb0, 0x96, 0x3a, 0x34, 0x15, 0xbb, 0xd6, 0x25, 0x83, 0x39,
    0xbd, 0x1b, 0xf5, 0x5d, 0xe6, 0x5d, 0xb5, 0x20, 0xc6, 0x3f, 0x5b, 0x8e, 0xab, 0x3d, 0x55, 0xde,
    0xbd, 0x05, 0xe9, 0x49, 0x42, 0x12, 0x17, 0x0f, 0x5d, 0x65, 0xb3, 0x28, 0x6b, 0x8b, 0x66, 0x87,
    0x05, 0xb1, 0xe2, 0xb2, 0xb5, 0x56, 0x86, 0x10, 0x61, 0x7a, 0xbb, 0x51, 0xd2, 0xdd, 0x0c, 0xb4,
    0x50, 0xef, 0x59, 0xdf, 0x4b, 0x90, 0x7d, 0xa9, 0x0c, 0xfa, 0x7b, 0x26, 0x8d, 0xe8, 0xc4, 0xc2
};

//private key
const uint8_t privKey[] =
{
    0x70, 0x83, 0x09, 0xa7, 0x44, 0x9e, 0x15, 0x6b, 0x0d, 0xb7, 0x0e, 0x5b, 0x52, 0xe6, 0x06, 0xc7,
    0xe0, 0x94, 0xed, 0x67, 0x6c, 0xe8, 0x95, 0x3b, 0xf6, 0xc1, 0x47, 0x57, 0xc8, 0x26, 0xf5, 0x90
};

//public key
const uint8_t pubKey[] =
{
    0x29, 0x57, 0x8c, 0x7a, 0xb6, 0xce, 0x0d, 0x11, 0x49, 0x3c, 0x95, 0xd5, 0xea, 0x05, 0xd2, 0x99,
    0xd5, 0x36, 0x80, 0x1c, 0xa9, 0xcb, 0xd5, 0x0e, 0x99, 0x24, 0xe4, 0x3b, 0x73, 0x3b, 0x83, 0xab,
    0x08, 0xc8, 0x04, 0x98, 0x79, 0xc6, 0x27, 0x8b, 0x22, 0x73, 0x34, 0x84, 0x74, 0x15, 0x85, 0x15,
    0xac, 0xca, 0xa3, 0x83, 0x44, 0x10, 0x6e, 0xf9, 0x68, 0x03, 0xc5, 0xa0, 0x5a, 0xdc, 0x48, 0x00
};

//known random values
const uint8_t knownRandom[] =
{
    0x58, 0xf7, 0x41, 0x77, 0x16, 0x20, 0xbd, 0xc4, 0x28, 0xe9, 0x1a, 0x32, 0xd8, 0x6d, 0x23, 0x08,
    0x73, 0xe9, 0x14, 0x03, 0x36, 0xfc, 0xfb, 0x1e, 0x12, 0x28, 0x92, 0xee, 0x1d, 0x50, 0x1b, 0xdb
};

//signature of known random values
const uint8_t knownSign[] =
{
    0x4a, 0x19, 0x27, 0x44, 0x29, 0xe4, 0x05, 0x22, 0x23, 0x4b, 0x87, 0x85, 0xdc, 0x25, 0xfc, 0x52,
    0x4f, 0x17, 0x9d, 0xcc, 0x95, 0xff, 0x09, 0xb3, 0xc9, 0x77, 0x0f, 0xc7, 0x1f, 0x54, 0xca, 0x0d,
    0x58, 0x98, 0x2b, 0x79, 0xa6, 0x5b, 0x73, 0x20, 0xf5, 0xb9, 0x2d, 0x13, 0xbd, 0xae, 0xcd, 0xd1,
    0x25, 0x9e, 0x76, 0x0f, 0x0f, 0x71, 0x8b, 0xa9, 0x33, 0xfd, 0x09, 0x8f, 0x6f, 0x75, 0xd4, 0xb7
};


cmox_ecc_handle_t hECC;     //ECC context handle
uint8_t eccBuffer[2000];    //ECC buffer

uint32_t arrRandom[8];                          //buffer for TRNG values
uint8_t hash[CMOX_SHA224_SIZE];                 //hash buffer
uint8_t signature[CMOX_ECC_SECP256R1_SIG_LEN];  //signature buffer

//1. calculate hash for data
bool CalculateHash(const uint8_t* data, size_t dataLen, uint8_t* hash)
{
  cmox_hash_retval_t retVal;
  size_t outputLen;

  //calculate hash
  retVal = cmox_hash_compute(
      CMOX_SHA224_ALGO,         //SHA224
      data,                     //source
      dataLen,                  //source length
      hash,                     //hash buffer
      CMOX_SHA224_SIZE,         //hash length
      &outputLen);              //output length

  //check the result
  if( (retVal != CMOX_HASH_SUCCESS) || (outputLen != CMOX_SHA224_SIZE) ) return false;

  return true;
}

//2. generate signature with random, hash and private key
bool SignDataWithPrivKey(const uint8_t* random, size_t randomLen, const uint8_t* privKey, size_t privKeyLen, const uint8_t* hash, size_t hashLen, uint8_t* signature)
{
  cmox_ecc_retval_t retVal;
  size_t outputLen;

  //allocate resources
  cmox_ecc_construct(&hECC, CMOX_ECC256_MATH_FUNCS, eccBuffer, sizeof(eccBuffer));

  //sign data
  retVal = cmox_ecdsa_sign(
      &hECC,
      CMOX_ECC_CURVE_SECP256R1,     //SECP256R1 curve
      random,                       //random
      randomLen,                    //random length
      privKey,                      //private key to sign
      privKeyLen,                   //length of private key
      hash,                         //hash output
      hashLen,                      //length of hash
      signature,                    //signature output
      &outputLen);                  //output length

  //check the result
  if(retVal != CMOX_ECC_SUCCESS) return false;

  //clean up resources
  cmox_ecc_cleanup(&hECC);

  return true;
}

//3. verify signature with hash and public key
bool VerifySignatureWithPubKey(const uint8_t* pubKey, size_t pubKeyLen, const uint8_t* hash, size_t hashLen, const uint8_t* signature, size_t signatureLen)
{
  cmox_ecc_retval_t retVal;
  uint32_t fault_check = CMOX_ECC_AUTH_FAIL;

  //allocate resources
  cmox_ecc_construct(&hECC, CMOX_ECC256_MATH_FUNCS, eccBuffer, sizeof(eccBuffer));

  //verify signature
  retVal = cmox_ecdsa_verify(
      &hECC,
      CMOX_ECC_CURVE_SECP256R1,   //SECP256R1
      pubKey,                     //public key for verification
      pubKeyLen,                  //public key length
      hash,                       //hash to compare
      hashLen,                    //hash length
      signature,                  //signature to verify
      signatureLen,               //signature length
      &fault_check);              //verification result

  //check the result
  if((retVal != CMOX_ECC_AUTH_SUCCESS) || (fault_check != CMOX_ECC_AUTH_SUCCESS)) return false;

  //clean up resources
  cmox_ecc_cleanup(&hECC);

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
  MX_RNG_Init();
  /* USER CODE BEGIN 2 */

  //initialize cmox
  if (cmox_initialize(NULL) != CMOX_INIT_SUCCESS)
  {
    Error_Handler();
  }

  printf("Print the message.\r\n");
  printHexArray((uint8_t*)message, sizeof(message));
  printf("\r\n");

  //1. Calculate hash
  if(!CalculateHash(message, sizeof(message), hash))
  {
    Error_Handler();
  }
  else
  {
    printf("Print the hash of message.\r\n");
    printHexArray(hash, sizeof(hash));
    printf("\r\n");
  }

  //2. Use known random number & calculate signature by random number + hash + private key
  if(!SignDataWithPrivKey(knownRandom, sizeof(knownRandom), privKey, sizeof(privKey), hash, sizeof(hash), signature))
  {
    Error_Handler();
  }
  else
  {
    if(memcmp(knownSign, signature, sizeof(knownSign)) != 0)
    {
      Error_Handler();
    }
    else
    {
      printf("Print the hash.\r\n");
      printHexArray(hash, sizeof(hash));
      printf("\r\n");

      printf("Print the signature.\r\n");
      printHexArray(signature, sizeof(signature));
      printf("\r\n");

      printf("Print the known signature.\r\n");
      printHexArray((uint8_t*)knownSign, sizeof(knownSign));
      printf("\r\n");
    }
  }

  //3. Verify signature with public key and hash
  if(!VerifySignatureWithPubKey(pubKey, sizeof(pubKey), hash, sizeof(hash), signature, sizeof(signature)))
  {
    Error_Handler();
  }
  else
  {
    printf("signature is verified.\r\n\r\n");
  }

  //2. Generate random number & calculate signature by random number + hash + private key
  for (uint32_t i = 0; i < sizeof(arrRandom) / sizeof(uint32_t); i++)
  {
    HAL_RNG_GenerateRandomNumber(&hrng, &arrRandom[i]);
  }

  if(!SignDataWithPrivKey((const uint8_t*)arrRandom, sizeof(arrRandom), privKey, sizeof(privKey), hash, sizeof(hash), signature))
  {
    Error_Handler();
  }
  else
  {
    printf("Print the hash.\r\n");
    printHexArray(hash, sizeof(hash));
    printf("\r\n");

    printf("Print the signature.\r\n");
    printHexArray(signature, sizeof(signature));
    printf("\r\n");
  }

  //3. Verify signature with public and hash
  if(!VerifySignatureWithPubKey(pubKey, sizeof(pubKey), hash, sizeof(hash), signature, sizeof(signature)))
  {
    Error_Handler();
  }
  else
  {
    printf("signature is verified.\r\n\r\n");
  }

  //finalize cmox
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
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
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
