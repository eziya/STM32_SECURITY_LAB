/**
  ******************************************************************************
  * @file    loader_low_level.c
  * @author  MCD Application Team
  * @brief   Loader Low Level Interface module
  *          This file provides set of firmware functions to manage CRC/UART low level API
  *          interface.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "loader_low_level.h"
#if defined(__ICCARM__)
#include <LowLevelIOInterface.h>
#endif /* __ICCARM__ */

/* Private variables ---------------------------------------------------------*/
static UART_HandleTypeDef   UartHandle;
static CRC_HandleTypeDef    CrcHandle;

/* Private defines -----------------------------------------------------------*/
#if defined(__ICCARM__)
/* New definition from EWARM V9, compatible with EWARM8 */
int iar_fputc(int ch);
#define PUTCHAR_PROTOTYPE int iar_fputc(int ch)
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#elif defined(__GNUC__)
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#endif /* __ICCARM__ */

/* Private function prototypes -----------------------------------------------*/


/* Public Functions Definition ------------------------------------------------------*/

/**
  * @brief  Low level CRC Init.
  * @param  None
  * @retval HAL_OK if successful, HAL_ERROR otherwise.
  */
HAL_StatusTypeDef LOADER_LL_CRC_Init(void)
{
  HAL_StatusTypeDef ret_status;

  /* Configure the peripheral clock */
  __HAL_RCC_CRC_CLK_ENABLE();

  CrcHandle.Instance = CRC;
  /* The input data are not inverted */
  CrcHandle.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  /* The output data are not inverted */
  CrcHandle.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  /* The CRC-16-CCIT polynomial is used */
  CrcHandle.Init.DefaultPolynomialUse    = DEFAULT_POLYNOMIAL_DISABLE;
  CrcHandle.Init.GeneratingPolynomial    = 0x1021U;
  CrcHandle.Init.CRCLength               = CRC_POLYLENGTH_16B;
  /* The zero init value is used */
  CrcHandle.Init.DefaultInitValueUse     = DEFAULT_INIT_VALUE_DISABLE;
  CrcHandle.Init.InitValue               = 0U;
  /* The input data are 8-bit long */
  CrcHandle.InputDataFormat              = CRC_INPUTDATA_FORMAT_BYTES;
  ret_status = HAL_CRC_Init(&CrcHandle);

  return ret_status;
}

/**
  * @brief  Low level CRC Calculate.
  * @param  pBuffer: pointer to data buffer.
  * @param  BufferLength: buffer length in bytes.
  * @retval uint32_t CRC (returned value LSBs for CRC shorter than 32 bits)
  */
uint32_t LOADER_LL_CRC_Calculate(uint32_t pBuffer[], uint32_t BufferLength)
{
  return HAL_CRC_Calculate(&CrcHandle, pBuffer, BufferLength);
}

/**
  * @brief  Low level UART Init.
  * @param  None
  * @retval HAL_OK if successful, HAL_ERROR otherwise.
  */
HAL_StatusTypeDef LOADER_LL_UART_Init(void)
{
#if defined(__GNUC__)
  setvbuf(stdout, NULL, _IONBF, 0);
#endif /* __GNUC__ */

  /* USART resources configuration (Clock, GPIO pins and USART registers) ----*/
  /* USART configured as follow:
  - BaudRate = 115200 baud
  - Word Length = 8 Bits
  - One Stop Bit
  - No parity
  - Hardware flow control disabled (RTS and CTS signals)
  - Receive and transmit enabled
  */
  UartHandle.Instance = LOADER_UART;
  UartHandle.Init.BaudRate = 115200U;
  UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits = UART_STOPBITS_1;
  UartHandle.Init.Parity = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode = UART_MODE_RX | UART_MODE_TX;
  UartHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXOVERRUNDISABLE_INIT;
  UartHandle.AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;

  return HAL_UART_Init(&UartHandle);
}

/**
  * @brief  Low level UART DeInit.
  * @param  None
  * @retval HAL_OK.
  */
HAL_StatusTypeDef LOADER_LL_UART_DeInit(void)
{
  return HAL_OK;
}

/**
  * @brief  Low level UART Write data (send).
  * @param  pData: pointer to the 128bit data to write.
  * @param  DataLength: pointer to the 128bit data to write.
  * @param  Timeout: Timeout duration.
  * @retval HAL_OK if successful, HAL_ERROR otherwise.
  */
HAL_StatusTypeDef LOADER_LL_UART_Transmit(uint8_t *pData, uint16_t DataLength, uint32_t Timeout)
{
  /* Check the pointers allocation */
  if (pData == NULL)
  {
    return HAL_ERROR;
  }

  return HAL_UART_Transmit(&UartHandle, (uint8_t *)pData, DataLength, Timeout);
}

/**
  * @brief  Low level UART Read data (receive).
  * @param  pData: pointer to the 128bit data where to store the received data.
  * @param  DataLength: the length of the data to be read in bytes.
  * @param  Timeout: Timeout duration.
  * @retval HAL_OK if successful, HAL_ERROR otherwise.
  */
HAL_StatusTypeDef LOADER_LL_UART_Receive(uint8_t *pData, uint16_t DataLength, uint32_t Timeout)
{
  /* Check the pointers allocation */
  if (pData == NULL)
  {
    return HAL_ERROR;
  }

  return HAL_UART_Receive(&UartHandle, (uint8_t *)pData, DataLength, Timeout);
}

/**
  * @brief  Low level UART Flush.
  * @param  None.
  * @retval HAL_OK.
  */
HAL_StatusTypeDef LOADER_LL_UART_Flush(void)
{
  /* Clean the input path */
  __HAL_UART_FLUSH_DRREGISTER(&UartHandle);

  return HAL_OK;
}

/**
  * @brief  Retargets the C library __write function to the IAR function iar_fputc.
  * @param  file: file descriptor.
  * @param  ptr: pointer to the buffer where the data is stored.
  * @param  len: length of the data to write in bytes.
  * @retval length of the written data in bytes.
  */
#if defined(__ICCARM__)
size_t __write(int file, unsigned char const *ptr, size_t len)
{
  size_t idx;
  unsigned char const *pdata = ptr;

  for (idx = 0; idx < len; idx++)
  {
    iar_fputc((int)*pdata);
    pdata++;
  }
  return len;
}
#endif /* __ICCARM__ */

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval ch
  */
PUTCHAR_PROTOTYPE
{
  (void) HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1U, 0xFFFFU);

  return ch;
}


/* MSP Functions Definition ------------------------------------------------------*/
/**
  * @brief UART MSP Initialization
  *        This function configures the hardware resources used in LOADER application.
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if (huart->Instance == LOADER_UART)
  {
    /* Peripheral Clock Enable */
    LOADER_UART_CLK_ENABLE();

    /* GPIO Ports Clock Enable */
    LOADER_UART_TX_GPIO_CLK_ENABLE();
    LOADER_UART_RX_GPIO_CLK_ENABLE();

    /*Configure GPIO pins : LOADER_UART_TX_Pin  */
    GPIO_InitStruct.Pin = LOADER_UART_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = LOADER_UART_TX_AF;
    HAL_GPIO_Init(LOADER_UART_TX_GPIO_PORT, &GPIO_InitStruct);

    /*Configure GPIO pins : LOADER_UART_RX_Pin  */
    GPIO_InitStruct.Pin = LOADER_UART_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = LOADER_UART_RX_AF;
    HAL_GPIO_Init(LOADER_UART_RX_GPIO_PORT, &GPIO_InitStruct);

  }

}

/**
  * @brief UART MSP De-Initialization
  *        This function frees the hardware resources used in LOADER application:
  *          - Disable the Peripheral's clock
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{

  if (huart->Instance == LOADER_UART)
  {
    /* Peripheral clock disable */
    LOADER_UART_CLK_DISABLE();

    /* GPIO DeInit*/
    HAL_GPIO_DeInit(LOADER_UART_TX_GPIO_PORT, LOADER_UART_TX_PIN);
    HAL_GPIO_DeInit(LOADER_UART_RX_GPIO_PORT, LOADER_UART_RX_PIN);

  }
}


/* Callback Functions Definition ------------------------------------------------------*/
/**
  * @brief  Implement the Cube_Hal Callback generated on the Tamper IRQ.
  * @param  None
  * @retval None
  */
void CALLBACK_Antitamper(RTC_HandleTypeDef *hrtc)
{
  UNUSED(hrtc);

  /* Reset to restart SBSFU */
  NVIC_SystemReset();
}


/* Static Functions Definition ------------------------------------------------------*/
