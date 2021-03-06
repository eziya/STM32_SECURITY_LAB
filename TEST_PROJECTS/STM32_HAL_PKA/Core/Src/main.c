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
#include "pka.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <memory.h>
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
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/*
Tools> openssl rsa -in .\MyPrivKey.pem -text
Private-Key: (1024 bit)
modulus:
    00:c5:dc:8a:7f:82:57:70:06:fa:89:49:61:65:30:
    3b:e5:9d:cb:3f:bd:22:dd:30:51:a3:54:56:9c:f7:
    6e:a4:b7:f7:85:d8:eb:dd:d2:f1:0f:1a:71:48:20:
    7b:aa:42:b8:49:d7:b8:a8:28:7f:3a:9f:34:3e:d7:
    60:f9:11:9b:12:42:01:6c:63:40:33:51:fb:72:b3:
    a5:97:32:76:91:fb:ea:16:73:c2:01:f4:f7:ed:ea:
    22:f7:a2:44:1f:c8:84:00:96:bb:d2:04:23:e6:01:
    d1:92:bd:dd:2b:38:0b:30:31:dd:7d:4f:b6:52:e0:
    01:88:f1:0b:43:ba:d9:d1:2b
publicExponent: 65537 (0x10001)
privateExponent:
    0c:a4:d7:44:e6:b3:67:b9:4a:71:53:7b:21:d8:3f:
    3b:a1:25:c7:e0:f2:45:68:3d:0f:4e:e8:6a:07:75:
    ce:98:e2:fc:6e:ac:43:f4:a2:ea:f0:29:a5:0e:4e:
    58:c7:02:b7:d5:4d:3b:91:ba:ef:a8:14:d2:9f:38:
    d7:31:c4:d2:fc:f6:5b:2d:9f:5a:61:da:74:2f:d7:
    7c:e5:19:aa:64:b8:55:0f:d7:26:60:bb:42:44:47:
    65:a0:09:2f:18:74:ca:9a:b1:52:33:3f:fd:fc:b9:
    13:af:c1:7f:f7:2f:4f:ba:7d:a5:f7:ce:be:fc:cd:
    d9:9c:ad:c7:a2:b5:96:81
prime1:
    00:f8:03:0a:d8:b6:70:b9:b0:c0:03:f2:0b:d9:51:
    1b:dd:38:c9:36:88:0b:97:6b:86:8b:8f:a6:5a:29:
    ba:6a:fc:47:9b:4d:1f:f5:7f:19:09:85:d1:a2:f3:
    61:c7:35:30:c1:dd:e9:48:86:62:a5:dc:d8:c4:fb:
    79:80:b6:62:71
prime2:
    00:cc:3b:fd:0c:b4:cb:14:34:cf:e3:11:63:f2:4b:
    e1:50:29:3f:0c:11:3d:09:17:d5:ab:bd:03:12:df:
    85:c8:e0:4a:bd:93:8f:ca:89:57:12:c2:8f:6b:48:
    ac:4d:6d:2b:95:52:ed:6d:d6:47:0e:14:a5:bf:89:
    89:28:75:83:5b
exponent1:
    19:73:0e:78:e0:77:be:c6:41:5d:e6:47:8a:fc:44:
    b4:b5:b0:08:15:7b:c3:04:f8:a0:77:94:25:6a:64:
    c3:b7:b1:8d:61:23:a7:68:6c:64:4a:c4:11:49:92:
    3e:03:da:45:fa:84:86:b9:ab:83:c3:9e:c6:cf:db:
    4e:f6:e8:31
exponent2:
    00:cb:71:5a:52:44:ea:22:97:b5:19:e8:e1:d9:a3:
    92:ae:ea:57:5e:6a:f1:94:d4:44:49:4d:ff:f8:d5:
    e6:0b:c2:b9:50:fc:e3:f8:8a:34:cd:80:37:a3:54:
    02:27:22:7f:8e:c0:cb:b8:65:01:e4:d9:d6:7d:e4:
    c0:57:28:30:0f
coefficient:
    65:21:7d:89:f1:8f:8c:a4:7f:c7:09:b3:01:11:ad:
    f2:77:72:77:1f:23:ac:9a:52:cd:df:e0:00:13:59:
    87:39:92:de:4a:7d:29:f2:a8:80:7a:34:7d:65:e1:
    e4:66:8f:d6:e5:f5:5e:ff:45:a7:88:cf:e7:c2:76:
    8b:c3:db:cc
writing RSA key
-----BEGIN RSA PRIVATE KEY-----
MIICXAIBAAKBgQDF3Ip/gldwBvqJSWFlMDvlncs/vSLdMFGjVFac926kt/eF2Ovd
0vEPGnFIIHuqQrhJ17ioKH86nzQ+12D5EZsSQgFsY0AzUftys6WXMnaR++oWc8IB
9Pft6iL3okQfyIQAlrvSBCPmAdGSvd0rOAswMd19T7ZS4AGI8QtDutnRKwIDAQAB
AoGADKTXROazZ7lKcVN7Idg/O6Elx+DyRWg9D07oagd1zpji/G6sQ/Si6vAppQ5O
WMcCt9VNO5G676gU0p841zHE0vz2Wy2fWmHadC/XfOUZqmS4VQ/XJmC7QkRHZaAJ
Lxh0ypqxUjM//fy5E6/Bf/cvT7p9pffOvvzN2Zytx6K1loECQQD4AwrYtnC5sMAD
8gvZURvdOMk2iAuXa4aLj6ZaKbpq/EebTR/1fxkJhdGi82HHNTDB3elIhmKl3NjE
+3mAtmJxAkEAzDv9DLTLFDTP4xFj8kvhUCk/DBE9CRfVq70DEt+FyOBKvZOPyolX
EsKPa0isTW0rlVLtbdZHDhSlv4mJKHWDWwJAGXMOeOB3vsZBXeZHivxEtLWwCBV7
wwT4oHeUJWpkw7exjWEjp2hsZErEEUmSPgPaRfqEhrmrg8Oexs/bTvboMQJBAMtx
WlJE6iKXtRno4dmjkq7qV15q8ZTURElN//jV5gvCuVD84/iKNM2AN6NUAicif47A
y7hlAeTZ1n3kwFcoMA8CQGUhfYnxj4ykf8cJswERrfJ3cncfI6yaUs3f4AATWYc5
kt5KfSnyqIB6NH1l4eRmj9bl9V7/RaeIz+fCdovD28w=
-----END RSA PRIVATE KEY-----
*/

uint8_t modulus[] = {
    0x00,0xc5,0xdc,0x8a,0x7f,0x82,0x57,0x70,0x06,0xfa,0x89,0x49,0x61,0x65,0x30,
    0x3b,0xe5,0x9d,0xcb,0x3f,0xbd,0x22,0xdd,0x30,0x51,0xa3,0x54,0x56,0x9c,0xf7,
    0x6e,0xa4,0xb7,0xf7,0x85,0xd8,0xeb,0xdd,0xd2,0xf1,0x0f,0x1a,0x71,0x48,0x20,
    0x7b,0xaa,0x42,0xb8,0x49,0xd7,0xb8,0xa8,0x28,0x7f,0x3a,0x9f,0x34,0x3e,0xd7,
    0x60,0xf9,0x11,0x9b,0x12,0x42,0x01,0x6c,0x63,0x40,0x33,0x51,0xfb,0x72,0xb3,
    0xa5,0x97,0x32,0x76,0x91,0xfb,0xea,0x16,0x73,0xc2,0x01,0xf4,0xf7,0xed,0xea,
    0x22,0xf7,0xa2,0x44,0x1f,0xc8,0x84,0x00,0x96,0xbb,0xd2,0x04,0x23,0xe6,0x01,
    0xd1,0x92,0xbd,0xdd,0x2b,0x38,0x0b,0x30,0x31,0xdd,0x7d,0x4f,0xb6,0x52,0xe0,
    0x01,0x88,0xf1,0x0b,0x43,0xba,0xd9,0xd1,0x2b};

uint8_t publicExponent[] = {0x00,0x01,0x00,0x01};

uint8_t privateExponent[] = {
    0x0c,0xa4,0xd7,0x44,0xe6,0xb3,0x67,0xb9,0x4a,0x71,0x53,0x7b,0x21,0xd8,0x3f,
    0x3b,0xa1,0x25,0xc7,0xe0,0xf2,0x45,0x68,0x3d,0x0f,0x4e,0xe8,0x6a,0x07,0x75,
    0xce,0x98,0xe2,0xfc,0x6e,0xac,0x43,0xf4,0xa2,0xea,0xf0,0x29,0xa5,0x0e,0x4e,
    0x58,0xc7,0x02,0xb7,0xd5,0x4d,0x3b,0x91,0xba,0xef,0xa8,0x14,0xd2,0x9f,0x38,
    0xd7,0x31,0xc4,0xd2,0xfc,0xf6,0x5b,0x2d,0x9f,0x5a,0x61,0xda,0x74,0x2f,0xd7,
    0x7c,0xe5,0x19,0xaa,0x64,0xb8,0x55,0x0f,0xd7,0x26,0x60,0xbb,0x42,0x44,0x47,
    0x65,0xa0,0x09,0x2f,0x18,0x74,0xca,0x9a,0xb1,0x52,0x33,0x3f,0xfd,0xfc,0xb9,
    0x13,0xaf,0xc1,0x7f,0xf7,0x2f,0x4f,0xba,0x7d,0xa5,0xf7,0xce,0xbe,0xfc,0xcd,
    0xd9,0x9c,0xad,0xc7,0xa2,0xb5,0x96,0x81};

uint8_t plainText[] = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f
};

uint8_t encryptedText[] = {
    0x67,0xf1,0x99,0xc9,0x55,0xc3,0xdb,0x9c,0xa7,0x1e,0x8e,0x90,0xea,0x65,0x07,0x57,
    0x09,0xbe,0x74,0x2b,0x86,0x83,0x63,0xb4,0x2e,0x45,0x58,0xb8,0x04,0x4d,0x21,0xa8,
    0x60,0x62,0xc1,0xe2,0xe6,0x18,0xe3,0xf8,0xa0,0x3d,0xd3,0x43,0xc2,0xcd,0x98,0xe0,
    0xa5,0x1f,0x08,0x09,0x6d,0xd6,0xca,0xe8,0x1f,0x3e,0x54,0xb1,0x88,0x48,0xf4,0x19,
    0x4d,0x86,0x02,0x80,0x40,0x43,0xcb,0x13,0xd4,0x5f,0x89,0xee,0xff,0x03,0x9c,0xf7,
    0xfc,0x7c,0x6f,0x31,0x38,0x0f,0x52,0xbb,0xfa,0x15,0x36,0xc2,0x3e,0xbb,0xaa,0x4a,
    0x75,0xa7,0xe2,0x66,0x0a,0x3a,0xf8,0x56,0xe5,0xa7,0x52,0x7a,0x94,0x67,0x89,0x07,
    0xcb,0x40,0x4a,0x24,0xac,0xfb,0x9b,0x00,0x1c,0xcb,0xd4,0x62,0xfa,0x1c,0xae,0xe0
};

uint8_t encBuffer[128] = {0,};
uint8_t decBuffer[128] = {0,};

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

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_PKA_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  PKA_ModExpInTypeDef in;
  in.expSize = sizeof(publicExponent);
  in.OpSize = sizeof(modulus);
  in.pExp = publicExponent;
  in.pMod = modulus;
  in.pOp1 = plainText;

  //publicExponent + modulus = public key
  if(HAL_OK != HAL_PKA_ModExp(&hpka, &in, 1000))
  {
    Error_Handler();
  }

  HAL_PKA_ModExp_GetResult(&hpka, encBuffer);

  if(memcmp(encryptedText, encBuffer, sizeof(encryptedText)) != 0)
  {
    Error_Handler();
  }

  in.expSize = sizeof(privateExponent);
  in.OpSize = sizeof(modulus);
  in.pExp = privateExponent;
  in.pMod = modulus;
  in.pOp1 = encryptedText;

  //privateExponent + modulus = private key
  if(HAL_OK != HAL_PKA_ModExp(&hpka, &in, 1000))
  {
    Error_Handler();
  }

  HAL_PKA_ModExp_GetResult(&hpka, decBuffer);

  if(memcmp(plainText, decBuffer, sizeof(plainText)) != 0)
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4|RCC_CLOCKTYPE_HCLK2
                              |RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS;
  PeriphClkInitStruct.SmpsClockSelection = RCC_SMPSCLKSOURCE_HSI;
  PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLKDIV_RANGE0;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN Smps */

  /* USER CODE END Smps */
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
    HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
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
