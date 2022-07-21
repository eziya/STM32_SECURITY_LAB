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
#include <stdbool.h>
#include <string.h>
#include "cmox_crypto.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// define CHUNK_SIZE for multiple call
#define CHUNK_SIZE  48u

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

cmox_cbc_handle_t hCBC;

/** Extract from NIST Special Publication 800-38A
F.2.1 CBC-AES128.Encrypt
key 2b7e151628aed2a6abf7158809cf4f3c
IV 000102030405060708090a0b0c0d0e0f

plainText 6bc1bee22e409f96e93d7e117393172a
plainText ae2d8a571e03ac9c9eb76fac45af8e51
plainText 30c81c46a35ce411e5fbc1191a0a52ef
plainText f69f2445df4f9b17ad2b417be66c3710

Ciphertext 7649abac8119b246cee98e9b12e9197d
Ciphertext 5086cb9b507219ee95db113a917678b2
Ciphertext 73bed6b8e3c1743b7116e69e22229516
Ciphertext 3ff1caa1681fac09120eca307586e1a7
 */

const uint8_t key[] =
{
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};

const uint8_t IV[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};

const uint8_t plainText[] =
{
    0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
    0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
    0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
    0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10
};

const uint8_t encryptedText[] =
{
    0x76, 0x49, 0xab, 0xac, 0x81, 0x19, 0xb2, 0x46, 0xce, 0xe9, 0x8e, 0x9b, 0x12, 0xe9, 0x19, 0x7d,
    0x50, 0x86, 0xcb, 0x9b, 0x50, 0x72, 0x19, 0xee, 0x95, 0xdb, 0x11, 0x3a, 0x91, 0x76, 0x78, 0xb2,
    0x73, 0xbe, 0xd6, 0xb8, 0xe3, 0xc1, 0x74, 0x3b, 0x71, 0x16, 0xe6, 0x9e, 0x22, 0x22, 0x95, 0x16,
    0x3f, 0xf1, 0xca, 0xa1, 0x68, 0x1f, 0xac, 0x09, 0x12, 0x0e, 0xca, 0x30, 0x75, 0x86, 0xe1, 0xa7
};

uint8_t encBuffer[sizeof(encryptedText)];
uint8_t decBuffer[sizeof(plainText)];

bool SingleCallEncrypt(void)
{
  cmox_cipher_retval_t retVal;
  size_t outputLen;

  printf("Testing SingleCallEncrypt.\r\n");

  //encrypt plainText
  retVal = cmox_cipher_encrypt(
      CMOX_AES_CBC_ENC_ALGO,        //AES-CBC
      plainText,                    //plain text
      sizeof(plainText),            //size of plain text
      key,                          //key
      sizeof(key),                  //size of key
      IV,                           //initialization vector
      sizeof(IV),                   //size of IV
      encBuffer,                    //encrypted text
      &outputLen);                  //size of encrypted text


  //check the result
  if( (retVal != CMOX_CIPHER_SUCCESS) ||
      (outputLen != sizeof(encryptedText)) ||
      (memcmp(encryptedText, encBuffer, outputLen) != 0) ) return false;

  printHexArray(encBuffer, outputLen);
  printf("\r\n");

  return true;
}

bool SingleCallDecrypt(void)
{
  cmox_cipher_retval_t retVal;
  size_t outputLen;

  printf("Testing SingleCallDecrypt.\r\n");

  //decrypt encryptedText
  retVal = cmox_cipher_decrypt(
      CMOX_AES_CBC_DEC_ALGO,        //AES-CBC
      encryptedText,                //encrypted text
      sizeof(encryptedText),        //size of encrypted text
      key,                          //key
      sizeof(key),                  //size of key
      IV,                           //initialization vector
      sizeof(IV),                   //size of IV
      decBuffer,                    //decrypted text
      &outputLen);                  //size of decrypted text

  //check the result
  if( (retVal != CMOX_CIPHER_SUCCESS) ||
      (outputLen != sizeof(plainText)) ||
      (memcmp(plainText, decBuffer, outputLen) != 0) ) return false;

  printHexArray(decBuffer, outputLen);
  printf("\r\n");

  return true;
}

bool MultiCallEncrypt(void)
{
  cmox_cipher_retval_t retVal;
  size_t outputLen;
  cmox_cipher_handle_t *pCipherCtx;
  uint32_t index;

  printf("Testing MultiCallEncrypt.\r\n");

  //select AES CBC encryption
  pCipherCtx = cmox_cbc_construct(&hCBC, CMOX_AES_CBC_ENC);
  if(pCipherCtx == NULL) return false;

  //initialize CBC
  if(cmox_cipher_init(pCipherCtx) != CMOX_CIPHER_SUCCESS) return false;

  //configure key
  if(cmox_cipher_setKey(pCipherCtx, key, sizeof(key)) != CMOX_CIPHER_SUCCESS) return false;

  //configure IV
  if(cmox_cipher_setIV(pCipherCtx, IV, sizeof(IV)) != CMOX_CIPHER_SUCCESS) return false;

  //encrypt the plaintext in multiple steps by appending chunks of CHUNK_SIZE bytes
  for(index = 0; index < (sizeof(plainText) - CHUNK_SIZE); index += CHUNK_SIZE)
  {
    retVal = cmox_cipher_append(
        pCipherCtx,
        &plainText[index],      //plain text
        CHUNK_SIZE,             //size to encrypt
        encBuffer,              //output buffer
        &outputLen);            //output length

    //check the result
    if( (retVal != CMOX_CIPHER_SUCCESS) ||
        (outputLen != CHUNK_SIZE) ||
        (memcmp(&encryptedText[index], encBuffer, outputLen) != 0) ) return false;

    printHexArray(encBuffer, outputLen);
  }

  //encrypt remaining
  if(index < sizeof(plainText))
  {
    retVal = cmox_cipher_append(
        pCipherCtx,
        &plainText[index],
        sizeof(plainText) - index,
        encBuffer,
        &outputLen);

    //check the result
    if( (retVal != CMOX_CIPHER_SUCCESS) ||
        (outputLen != (sizeof(plainText) - index)) ||
        (memcmp(&encryptedText[index], encBuffer, outputLen) != 0) ) return false;

    printHexArray(encBuffer, outputLen);
  }

  //cleanup context
  if(cmox_cipher_cleanup(pCipherCtx) != CMOX_CIPHER_SUCCESS) return false;
  printf("\r\n");

  return true;
}

bool MultiCallDecrypt(void)
{
  cmox_cipher_retval_t retVal;
  size_t outputLen;
  cmox_cipher_handle_t *pCipherCtx;
  uint32_t index;

  printf("Testing MultiCallDecrypt.\r\n");

  //select AES CBC decryption
  pCipherCtx = cmox_cbc_construct(&hCBC, CMOX_AES_CBC_DEC);
  if(pCipherCtx == NULL) return false;

  //initialize CBC
  if(cmox_cipher_init(pCipherCtx) != CMOX_CIPHER_SUCCESS) return false;

  //configure key
  if(cmox_cipher_setKey(pCipherCtx, key, sizeof(key)) != CMOX_CIPHER_SUCCESS) return false;

  //configure IV
  if(cmox_cipher_setIV(pCipherCtx, IV, sizeof(IV)) != CMOX_CIPHER_SUCCESS) return false;

  //decrypt the plainText in multiple steps by appending chunks of CHUNK_SIZE bytes
  for(index = 0; index < (sizeof(encryptedText) - CHUNK_SIZE); index += CHUNK_SIZE)
  {
    retVal = cmox_cipher_append(
        pCipherCtx,
        &encryptedText[index],
        CHUNK_SIZE,
        decBuffer,
        &outputLen);

    //check the result
    if( (retVal != CMOX_CIPHER_SUCCESS) ||
        (outputLen != CHUNK_SIZE) ||
        (memcmp(&plainText[index], decBuffer, outputLen) != 0) ) return false;

    printHexArray(decBuffer, outputLen);
  }

  //decrypt remaining
  if (index < sizeof(encryptedText))
  {
    retVal = cmox_cipher_append(
        pCipherCtx,
        &encryptedText[index],
        sizeof(encryptedText) - index,
        decBuffer,
        &outputLen);

    //check the result
    if( (retVal != CMOX_CIPHER_SUCCESS) ||
        (outputLen != (sizeof(encryptedText) - index)) ||
        (memcmp(&plainText[index], decBuffer, outputLen) != 0) ) return false;

    printHexArray(decBuffer, outputLen);
  }

  //cleanup context
  if (cmox_cipher_cleanup(pCipherCtx) != CMOX_CIPHER_SUCCESS) return false;
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

  //disable prefetch (PREFETCH_ENABLE == 0)
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

  //print plain text & encrypted text
  printf("Print plainText.\r\n");
  printHexArray((uint8_t*)plainText, sizeof(plainText));
  printf("\r\n");
  printf("Print encryptedText.\r\n");
  printHexArray((uint8_t*)encryptedText, sizeof(encryptedText));
  printf("\r\n");

  //initialize CMOX
  if (cmox_initialize(NULL) != CMOX_INIT_SUCCESS)
  {
    Error_Handler();
  }

  /********** Single call example **********/

  if(!SingleCallEncrypt())
  {
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
  }

  if(!SingleCallDecrypt())
  {
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
  }

  /********** Multiple call example **********/

  if(!MultiCallEncrypt())
  {
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
  }

  if(!MultiCallDecrypt())
  {
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
  }

  //finalize CMOX
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
