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

/** Extract from pkcs-1v2-1d2-vec/oaep-vect.txt
# Example 10: A 2048-bit RSA key pair
# -----------------------------------

# Modulus:
ae 45 ed 56 01 ce c6 b8 cc 05 f8 03 93 5c 67 4d
db e0 d7 5c 4c 09 fd 79 51 fc 6b 0c ae c3 13 a8
df 39 97 0c 51 8b ff ba 5e d6 8f 3f 0d 7f 22 a4
02 9d 41 3f 1a e0 7e 4e be 9e 41 77 ce 23 e7 f5
40 4b 56 9e 4e e1 bd cf 3c 1f b0 3e f1 13 80 2d
4f 85 5e b9 b5 13 4b 5a 7c 80 85 ad ca e6 fa 2f
a1 41 7e c3 76 3b e1 71 b0 c6 2b 76 0e de 23 c1
2a d9 2b 98 08 84 c6 41 f5 a8 fa c2 6b da d4 a0
33 81 a2 2f e1 b7 54 88 50 94 c8 25 06 d4 01 9a
53 5a 28 6a fe b2 71 bb 9b a5 92 de 18 dc f6 00
c2 ae ea e5 6e 02 f7 cf 79 fc 14 cf 3b dc 7c d8
4f eb bb f9 50 ca 90 30 4b 22 19 a7 aa 06 3a ef
a2 c3 c1 98 0e 56 0c d6 4a fe 77 95 85 b6 10 76
57 b9 57 85 7e fd e6 01 09 88 ab 7d e4 17 fc 88
d8 f3 84 c4 e6 e7 2c 3f 94 3e 0c 31 c0 c4 a5 cc
36 f8 79 d8 a3 ac 9d 7d 59 86 0e aa da 6b 83 bb

# Public Exponent:
01 00 01

# Private Exponent:
05 6b 04 21 6f e5 f3 54 ac 77 25 0a 4b 6b 0c 85
25 a8 5c 59 b0 bd 80 c5 64 50 a2 2d 5f 43 8e 59
6a 33 3a a8 75 e2 91 dd 43 f4 8c b8 8b 9d 5f c0
d4 99 f9 fc d1 c3 97 f9 af c0 70 cd 9e 39 8c 8d
19 e6 1d b7 c7 41 0a 6b 26 75 df bf 5d 34 5b 80
4d 20 1a dd 50 2d 5c e2 df cb 09 1c e9 99 7b be
be 57 30 6f 38 3e 4d 58 81 03 f0 36 f7 e8 5d 19
34 d1 52 a3 23 e4 a8 db 45 1d 6f 4a 5b 1b 0f 10
2c c1 50 e0 2f ee e2 b8 8d ea 4a d4 c1 ba cc b2
4d 84 07 2d 14 e1 d2 4a 67 71 f7 40 8e e3 05 64
fb 86 d4 39 3a 34 bc f0 b7 88 50 1d 19 33 03 f1
3a 22 84 b0 01 f0 f6 49 ea f7 93 28 d4 ac 5c 43
0a b4 41 49 20 a9 46 0e d1 b7 bc 40 ec 65 3e 87
6d 09 ab c5 09 ae 45 b5 25 19 01 16 a0 c2 61 01
84 82 98 50 9c 1c 3b f3 a4 83 e7 27 40 54 e1 5e
97 07 50 36 e9 89 f6 09 32 80 7b 52 57 75 1e 79

# OAEP Example 10.4
# Message:
bc dd 19 0d a3 b7 d3 00 df 9a 06 e2 2c aa e2 a7
5f 10 c9 1f f6 67 b7 c1 6b de 8b 53 06 4a 26 49
a9 40 45 c9

# Seed:
5c ac a6 a0 f7 64 16 1a 96 84 f8 5d 92 b6 e0 ef
37 ca 8b 65

# Encryption:
63 18 e9 fb 5c 0d 05 e5 30 7e 16 83 43 6e 90 32
93 ac 46 42 35 8a aa 22 3d 71 63 01 3a ba 87 e2
df da 8e 60 c6 86 0e 29 a1 e9 26 86 16 3e a0 b9
17 5f 32 9c a3 b1 31 a1 ed d3 a7 77 59 a8 b9 7b
ad 6a 4f 8f 43 96 f2 8c f6 f3 9c a5 81 12 e4 81
60 d6 e2 03 da a5 85 6f 3a ca 5f fe d5 77 af 49
94 08 e3 df d2 33 e3 e6 04 db e3 4a 9c 4c 90 82
de 65 52 7c ac 63 31 d2 9d c8 0e 05 08 a0 fa 71
22 e7 f3 29 f6 cc a5 cf a3 4d 4d 1d a4 17 80 54
57 e0 08 be c5 49 e4 78 ff 9e 12 a7 63 c4 77 d1
5b bb 78 f5 b6 9b d5 78 30 fc 2c 4e d6 86 d7 9b
c7 2a 95 d8 5f 88 13 4c 6b 0a fe 56 a8 cc fb c8
55 82 8b b3 39 bd 17 90 9c f1 d7 0d e3 33 5a e0
70 39 09 3e 60 6d 65 53 65 de 65 50 b8 72 cd 6d
e1 d4 40 ee 03 1b 61 94 5f 62 9a d8 a3 53 b0 d4
09 39 e9 6a 3c 45 0d 2a 8d 5e ee 9f 67 80 93 c8

*/

//plain text
const uint8_t message[] =
{
    0xbc, 0xdd, 0x19, 0x0d, 0xa3, 0xb7, 0xd3, 0x00, 0xdf, 0x9a, 0x06, 0xe2, 0x2c, 0xaa, 0xe2, 0xa7,
    0x5f, 0x10, 0xc9, 0x1f, 0xf6, 0x67, 0xb7, 0xc1, 0x6b, 0xde, 0x8b, 0x53, 0x06, 0x4a, 0x26, 0x49,
    0xa9, 0x40, 0x45, 0xc9
};

//seed
const uint8_t seed[] =
{
    0x5c, 0xac, 0xa6, 0xa0, 0xf7, 0x64, 0x16, 0x1a, 0x96, 0x84, 0xf8, 0x5d, 0x92, 0xb6, 0xe0, 0xef,
    0x37, 0xca, 0x8b, 0x65
};

//encrypted text
const uint8_t encryptedMessage[] =
{
    0x63, 0x18, 0xe9, 0xfb, 0x5c, 0x0d, 0x05, 0xe5, 0x30, 0x7e, 0x16, 0x83, 0x43, 0x6e, 0x90, 0x32,
    0x93, 0xac, 0x46, 0x42, 0x35, 0x8a, 0xaa, 0x22, 0x3d, 0x71, 0x63, 0x01, 0x3a, 0xba, 0x87, 0xe2,
    0xdf, 0xda, 0x8e, 0x60, 0xc6, 0x86, 0x0e, 0x29, 0xa1, 0xe9, 0x26, 0x86, 0x16, 0x3e, 0xa0, 0xb9,
    0x17, 0x5f, 0x32, 0x9c, 0xa3, 0xb1, 0x31, 0xa1, 0xed, 0xd3, 0xa7, 0x77, 0x59, 0xa8, 0xb9, 0x7b,
    0xad, 0x6a, 0x4f, 0x8f, 0x43, 0x96, 0xf2, 0x8c, 0xf6, 0xf3, 0x9c, 0xa5, 0x81, 0x12, 0xe4, 0x81,
    0x60, 0xd6, 0xe2, 0x03, 0xda, 0xa5, 0x85, 0x6f, 0x3a, 0xca, 0x5f, 0xfe, 0xd5, 0x77, 0xaf, 0x49,
    0x94, 0x08, 0xe3, 0xdf, 0xd2, 0x33, 0xe3, 0xe6, 0x04, 0xdb, 0xe3, 0x4a, 0x9c, 0x4c, 0x90, 0x82,
    0xde, 0x65, 0x52, 0x7c, 0xac, 0x63, 0x31, 0xd2, 0x9d, 0xc8, 0x0e, 0x05, 0x08, 0xa0, 0xfa, 0x71,
    0x22, 0xe7, 0xf3, 0x29, 0xf6, 0xcc, 0xa5, 0xcf, 0xa3, 0x4d, 0x4d, 0x1d, 0xa4, 0x17, 0x80, 0x54,
    0x57, 0xe0, 0x08, 0xbe, 0xc5, 0x49, 0xe4, 0x78, 0xff, 0x9e, 0x12, 0xa7, 0x63, 0xc4, 0x77, 0xd1,
    0x5b, 0xbb, 0x78, 0xf5, 0xb6, 0x9b, 0xd5, 0x78, 0x30, 0xfc, 0x2c, 0x4e, 0xd6, 0x86, 0xd7, 0x9b,
    0xc7, 0x2a, 0x95, 0xd8, 0x5f, 0x88, 0x13, 0x4c, 0x6b, 0x0a, 0xfe, 0x56, 0xa8, 0xcc, 0xfb, 0xc8,
    0x55, 0x82, 0x8b, 0xb3, 0x39, 0xbd, 0x17, 0x90, 0x9c, 0xf1, 0xd7, 0x0d, 0xe3, 0x33, 0x5a, 0xe0,
    0x70, 0x39, 0x09, 0x3e, 0x60, 0x6d, 0x65, 0x53, 0x65, 0xde, 0x65, 0x50, 0xb8, 0x72, 0xcd, 0x6d,
    0xe1, 0xd4, 0x40, 0xee, 0x03, 0x1b, 0x61, 0x94, 0x5f, 0x62, 0x9a, 0xd8, 0xa3, 0x53, 0xb0, 0xd4,
    0x09, 0x39, 0xe9, 0x6a, 0x3c, 0x45, 0x0d, 0x2a, 0x8d, 0x5e, 0xee, 0x9f, 0x67, 0x80, 0x93, 0xc8
};

//modulus
const uint8_t modulus[] =
{
    0xae, 0x45, 0xed, 0x56, 0x01, 0xce, 0xc6, 0xb8, 0xcc, 0x05, 0xf8, 0x03, 0x93, 0x5c, 0x67, 0x4d,
    0xdb, 0xe0, 0xd7, 0x5c, 0x4c, 0x09, 0xfd, 0x79, 0x51, 0xfc, 0x6b, 0x0c, 0xae, 0xc3, 0x13, 0xa8,
    0xdf, 0x39, 0x97, 0x0c, 0x51, 0x8b, 0xff, 0xba, 0x5e, 0xd6, 0x8f, 0x3f, 0x0d, 0x7f, 0x22, 0xa4,
    0x02, 0x9d, 0x41, 0x3f, 0x1a, 0xe0, 0x7e, 0x4e, 0xbe, 0x9e, 0x41, 0x77, 0xce, 0x23, 0xe7, 0xf5,
    0x40, 0x4b, 0x56, 0x9e, 0x4e, 0xe1, 0xbd, 0xcf, 0x3c, 0x1f, 0xb0, 0x3e, 0xf1, 0x13, 0x80, 0x2d,
    0x4f, 0x85, 0x5e, 0xb9, 0xb5, 0x13, 0x4b, 0x5a, 0x7c, 0x80, 0x85, 0xad, 0xca, 0xe6, 0xfa, 0x2f,
    0xa1, 0x41, 0x7e, 0xc3, 0x76, 0x3b, 0xe1, 0x71, 0xb0, 0xc6, 0x2b, 0x76, 0x0e, 0xde, 0x23, 0xc1,
    0x2a, 0xd9, 0x2b, 0x98, 0x08, 0x84, 0xc6, 0x41, 0xf5, 0xa8, 0xfa, 0xc2, 0x6b, 0xda, 0xd4, 0xa0,
    0x33, 0x81, 0xa2, 0x2f, 0xe1, 0xb7, 0x54, 0x88, 0x50, 0x94, 0xc8, 0x25, 0x06, 0xd4, 0x01, 0x9a,
    0x53, 0x5a, 0x28, 0x6a, 0xfe, 0xb2, 0x71, 0xbb, 0x9b, 0xa5, 0x92, 0xde, 0x18, 0xdc, 0xf6, 0x00,
    0xc2, 0xae, 0xea, 0xe5, 0x6e, 0x02, 0xf7, 0xcf, 0x79, 0xfc, 0x14, 0xcf, 0x3b, 0xdc, 0x7c, 0xd8,
    0x4f, 0xeb, 0xbb, 0xf9, 0x50, 0xca, 0x90, 0x30, 0x4b, 0x22, 0x19, 0xa7, 0xaa, 0x06, 0x3a, 0xef,
    0xa2, 0xc3, 0xc1, 0x98, 0x0e, 0x56, 0x0c, 0xd6, 0x4a, 0xfe, 0x77, 0x95, 0x85, 0xb6, 0x10, 0x76,
    0x57, 0xb9, 0x57, 0x85, 0x7e, 0xfd, 0xe6, 0x01, 0x09, 0x88, 0xab, 0x7d, 0xe4, 0x17, 0xfc, 0x88,
    0xd8, 0xf3, 0x84, 0xc4, 0xe6, 0xe7, 0x2c, 0x3f, 0x94, 0x3e, 0x0c, 0x31, 0xc0, 0xc4, 0xa5, 0xcc,
    0x36, 0xf8, 0x79, 0xd8, 0xa3, 0xac, 0x9d, 0x7d, 0x59, 0x86, 0x0e, 0xaa, 0xda, 0x6b, 0x83, 0xbb
};

//public exponent
const uint8_t publicExponent[] =
{
    0x01, 0x00, 0x01
};

//private exponent
const uint8_t privateExponent[] =
{
    0x05, 0x6b, 0x04, 0x21, 0x6f, 0xe5, 0xf3, 0x54, 0xac, 0x77, 0x25, 0x0a, 0x4b, 0x6b, 0x0c, 0x85,
    0x25, 0xa8, 0x5c, 0x59, 0xb0, 0xbd, 0x80, 0xc5, 0x64, 0x50, 0xa2, 0x2d, 0x5f, 0x43, 0x8e, 0x59,
    0x6a, 0x33, 0x3a, 0xa8, 0x75, 0xe2, 0x91, 0xdd, 0x43, 0xf4, 0x8c, 0xb8, 0x8b, 0x9d, 0x5f, 0xc0,
    0xd4, 0x99, 0xf9, 0xfc, 0xd1, 0xc3, 0x97, 0xf9, 0xaf, 0xc0, 0x70, 0xcd, 0x9e, 0x39, 0x8c, 0x8d,
    0x19, 0xe6, 0x1d, 0xb7, 0xc7, 0x41, 0x0a, 0x6b, 0x26, 0x75, 0xdf, 0xbf, 0x5d, 0x34, 0x5b, 0x80,
    0x4d, 0x20, 0x1a, 0xdd, 0x50, 0x2d, 0x5c, 0xe2, 0xdf, 0xcb, 0x09, 0x1c, 0xe9, 0x99, 0x7b, 0xbe,
    0xbe, 0x57, 0x30, 0x6f, 0x38, 0x3e, 0x4d, 0x58, 0x81, 0x03, 0xf0, 0x36, 0xf7, 0xe8, 0x5d, 0x19,
    0x34, 0xd1, 0x52, 0xa3, 0x23, 0xe4, 0xa8, 0xdb, 0x45, 0x1d, 0x6f, 0x4a, 0x5b, 0x1b, 0x0f, 0x10,
    0x2c, 0xc1, 0x50, 0xe0, 0x2f, 0xee, 0xe2, 0xb8, 0x8d, 0xea, 0x4a, 0xd4, 0xc1, 0xba, 0xcc, 0xb2,
    0x4d, 0x84, 0x07, 0x2d, 0x14, 0xe1, 0xd2, 0x4a, 0x67, 0x71, 0xf7, 0x40, 0x8e, 0xe3, 0x05, 0x64,
    0xfb, 0x86, 0xd4, 0x39, 0x3a, 0x34, 0xbc, 0xf0, 0xb7, 0x88, 0x50, 0x1d, 0x19, 0x33, 0x03, 0xf1,
    0x3a, 0x22, 0x84, 0xb0, 0x01, 0xf0, 0xf6, 0x49, 0xea, 0xf7, 0x93, 0x28, 0xd4, 0xac, 0x5c, 0x43,
    0x0a, 0xb4, 0x41, 0x49, 0x20, 0xa9, 0x46, 0x0e, 0xd1, 0xb7, 0xbc, 0x40, 0xec, 0x65, 0x3e, 0x87,
    0x6d, 0x09, 0xab, 0xc5, 0x09, 0xae, 0x45, 0xb5, 0x25, 0x19, 0x01, 0x16, 0xa0, 0xc2, 0x61, 0x01,
    0x84, 0x82, 0x98, 0x50, 0x9c, 0x1c, 0x3b, 0xf3, 0xa4, 0x83, 0xe7, 0x27, 0x40, 0x54, 0xe1, 0x5e,
    0x97, 0x07, 0x50, 0x36, 0xe9, 0x89, 0xf6, 0x09, 0x32, 0x80, 0x7b, 0x52, 0x57, 0x75, 0x1e, 0x79
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

cmox_rsa_handle_t hRSA;                                 //RSA handle
cmox_rsa_key_t RSAKey;                                  //RSA key
uint8_t workingBuffer[7000];                            //RSA working buffer
uint8_t encBuffer[sizeof(encryptedMessage)];            //encrypted Buffer
uint8_t decBuffer[sizeof(message)];                     //decrypted Buffer
size_t encOutputLen;                                    //encrypted length
size_t decOutputLen;                                    //decrypted length

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

bool RSAEncrypt(const uint8_t* plainText, const size_t plainTextLen, uint8_t* encBuffer, size_t* encBufferLen)
{
  cmox_rsa_retval_t retval;
  cmox_rsa_construct(&hRSA, CMOX_RSA_MATH_FUNCS, CMOX_MODEXP_PUBLIC, workingBuffer, sizeof(workingBuffer));

  //prepare public key
  retval = cmox_rsa_setKey(
      &RSAKey,
      modulus, sizeof(modulus),
      publicExponent, sizeof(publicExponent));

  if(retval != CMOX_RSA_SUCCESS) return false;

  //encrypt with public key
  retval = cmox_rsa_pkcs1v22_encrypt(
      &hRSA,
      &RSAKey,
      plainText, plainTextLen,
      CMOX_RSA_PKCS1V22_HASH_SHA1,
      seed, sizeof(seed),
      NULL, 0,
      encBuffer, encBufferLen);

  if(retval != CMOX_RSA_SUCCESS) return false;

  cmox_rsa_cleanup(&hRSA);

  return true;
}

bool RSADecrypt(const uint8_t* encBuffer, const size_t encBufferLen, uint8_t* decBuffer, size_t* decBufferLen)
{
  cmox_rsa_retval_t retval;
  cmox_rsa_construct(&hRSA, CMOX_RSA_MATH_FUNCS, CMOX_MODEXP_PRIVATE, workingBuffer, sizeof(workingBuffer));

  //prepare private key
  retval = cmox_rsa_setKey(
      &RSAKey,
      modulus, sizeof(modulus),
      privateExponent, sizeof(privateExponent));

  if(retval != CMOX_RSA_SUCCESS) return false;

  //decrypt with private key
  retval = cmox_rsa_pkcs1v22_decrypt(
      &hRSA,
      &RSAKey,
      encBuffer, encBufferLen,
      CMOX_RSA_PKCS1V22_HASH_SHA1,
      NULL, 0,
      decBuffer, decBufferLen);

  if(retval != CMOX_RSA_SUCCESS) return false;

  cmox_rsa_cleanup(&hRSA);

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

  /* Initialize cryptographic library */
  if (cmox_initialize(NULL) != CMOX_INIT_SUCCESS)
  {
    Error_Handler();
  }

  printf("Print message.\r\n");
  printHexArray(message, sizeof(message));
  printf("\r\n");

  printf("Print encrypted message.\r\n");
  printHexArray(encryptedMessage, sizeof(encryptedMessage));
  printf("\r\n");


  if(!RSAEncrypt(message, sizeof(message), encBuffer, &encOutputLen))
  {
    Error_Handler();
  }
  else
  {
    if((memcmp(encBuffer, encryptedMessage, sizeof(encryptedMessage)) != 0) || (sizeof(encryptedMessage) != encOutputLen))
    {
      Error_Handler();
    }
    else
    {
      printf("Print encBuffer.\r\n");
      printHexArray(encBuffer, sizeof(encBuffer));
      printf("\r\n");
    }
  }

  if(!RSADecrypt(encryptedMessage, sizeof(encryptedMessage), decBuffer, &decOutputLen))
  {
    Error_Handler();
  }
  else
  {
    if((memcmp(decBuffer, message, sizeof(message)) != 0) || (sizeof(message) != decOutputLen))
    {
      Error_Handler();
    }
    else
    {
      printf("Print decBuffer.\r\n");
      printHexArray(decBuffer, sizeof(decBuffer));
      printf("\r\n");
    }
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
