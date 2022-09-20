/**
  ******************************************************************************
  * @file    stsafe_service_interface.c
  * @author  SMD application teams
  * @brief   Service Interface file to support the hardware services required by the
  *          STSAFE-A Middleware and offered by the specific HW, Low Level library, or BSP
  *          selected by the user. E.g.:
  *           + IOs
  *           + Communication Bus (e.g. I2C)
  *           + Timing delay
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "stsafea_service.h"
#include "stsafea_interface_conf.h"
#include MCU_PLATFORM_INCLUDE

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define STSAFE_ONBOARD

#ifdef STSAFE_ONBOARD

/* I2c defines */ 

#define STSAFEA_VREG_PIN                          GPIO_PIN_7
#define STSAFEA_VREG_GPIO_PORT                    GPIOD
#define STSAFEA_VREG_GPIO_PORT_CLK_ENABLE         __HAL_RCC_GPIOD_CLK_ENABLE
#define STSAFEA_DEVICE_ADDRESS                    0x0020
#define I2C_ANALOG_FILTER_DELAY_DEFAULT        2U      /* ns */
#define BUS_I2C2_INSTANCE                       I2C2
#define BUS_I2C2_CLK_ENABLE()                   __HAL_RCC_I2C2_CLK_ENABLE()
#define BUS_I2C2_CLK_DISABLE()                  __HAL_RCC_I2C2_CLK_DISABLE()
#define BUS_I2C2_SDA_GPIO_PIN                   GPIO_PIN_11
#define BUS_I2C2_SCL_GPIO_PIN                   GPIO_PIN_10
#define BUS_I2C2_SDA_GPIO_PORT                  GPIOB
#define BUS_I2C2_SCL_GPIO_PORT                  GPIOB
#define BUS_I2C2_SDA_GPIO_AF                    GPIO_AF4_I2C2
#define BUS_I2C2_SCL_GPIO_AF                    GPIO_AF4_I2C2
#define BUS_I2C2_SDA_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOB_CLK_ENABLE()
#define BUS_I2C2_SCL_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOB_CLK_ENABLE()
#define BUS_I2C2_POLL_TIMEOUT                0x1000U
#define BUS_I2C2_TIMING                      0x00702991 /* Corresponding to frequency of I2C1 = 400 KHz*/

/* BSP Common Error codes */
   
#define BSP_ERROR_NONE                        0
#define BSP_ERROR_NO_INIT                    -1
#define BSP_ERROR_WRONG_PARAM                -2
#define BSP_ERROR_BUSY                       -3
#define BSP_ERROR_PERIPH_FAILURE             -4
#define BSP_ERROR_COMPONENT_FAILURE          -5
#define BSP_ERROR_UNKNOWN_FAILURE            -6
#define BSP_ERROR_UNKNOWN_COMPONENT          -7 
#define BSP_ERROR_BUS_FAILURE                -8 
#define BSP_ERROR_CLOCK_FAILURE              -9  
#define BSP_ERROR_MSP_FAILURE                -10  
#define BSP_ERROR_FEATURE_NOT_SUPPORTED      -11   
   
/* BSP BUS error codes */

#define BSP_ERROR_BUS_TRANSACTION_FAILURE    -100
#define BSP_ERROR_BUS_ARBITRATION_LOSS       -101
#define BSP_ERROR_BUS_ACKNOWLEDGE_FAILURE    -102
#define BSP_ERROR_BUS_PROTOCOL_FAILURE       -103
#define BSP_ERROR_BUS_MODE_FAULT             -104
#define BSP_ERROR_BUS_FRAME_ERROR            -105
#define BSP_ERROR_BUS_CRC_ERROR              -106
#define BSP_ERROR_BUS_DMA_FAILURE            -107

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef  hbus_i2c2;
/* Private function prototypes -----------------------------------------------*/
int32_t HW_IO_Init(void);
void    BSP_TimeDelay(uint32_t msDelay);
HAL_StatusTypeDef MX_I2C2_Init(I2C_HandleTypeDef *phi2c, uint32_t timing);
int32_t BSP_I2C2_Init(void);
int32_t BSP_I2C2_DeInit(void);
int32_t BSP_I2C2_Send(uint16_t DevAddr, uint8_t *pData, uint16_t Length);
int32_t BSP_I2C2_Recv(uint16_t DevAddr, uint8_t *pData, uint16_t Length);
int32_t CRC16X25_Init(void);
uint32_t CRC_Compute(uint8_t *pData1, uint16_t Length1, uint8_t *pData2, uint16_t Length2);
static void I2C2_MspInit(I2C_HandleTypeDef *hI2c);
static void I2C2_MspDeInit(I2C_HandleTypeDef *hI2c);

#ifdef HAL_CRC_MODULE_ENABLED
CRC_HandleTypeDef hbus_crc;
static __IO uint32_t CRC_CR; /*!< CRC Control register */
#endif /* HAL_CRC_MODULE_ENABLED */

/* Private functions ---------------------------------------------------------*/


/**
  * @brief  Configure STSAFE IO and Bus operation functions to be implemented at User level
  * @param  Ctx the STSAFE IO context
  * @retval 0 in case of success, an error code otherwise
  */
int8_t StSafeA_HW_Probe(void *pCtx)
{
  STSAFEA_HW_t *HwCtx = (STSAFEA_HW_t *)pCtx;


  HwCtx->IOInit     = HW_IO_Init;
  HwCtx->BusInit    = BSP_I2C2_Init;
  HwCtx->BusDeInit  = BSP_I2C2_DeInit;
  HwCtx->BusSend    = BSP_I2C2_Send;
  HwCtx->BusRecv    = BSP_I2C2_Recv;
  HwCtx->CrcInit    = CRC16X25_Init;
  HwCtx->CrcCompute = CRC_Compute;
  HwCtx->TimeDelay  = HAL_Delay;
  HwCtx->DevAddr    = STSAFEA_DEVICE_ADDRESS;

  return STSAFEA_BUS_OK;
}


/**
  * @brief  Additional IO pins configuration needed for STSAFE (VREG pin, etc.)
  * @param  none
  * @retval 0 in case of success, an error code otherwise
  */
int32_t HW_IO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  STSAFEA_VREG_GPIO_PORT_CLK_ENABLE();

  /* Configure GPIO pin : RST Pin */
  GPIO_InitStruct.Pin = STSAFEA_VREG_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(STSAFEA_VREG_GPIO_PORT, &GPIO_InitStruct);

  /* Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(STSAFEA_VREG_GPIO_PORT, STSAFEA_VREG_PIN, GPIO_PIN_SET);
  HAL_Delay(50);

  return BSP_ERROR_NONE;
}


/**
  * @brief  This function provides a delay (in milliseconds)
  * @param  none
  * @retval 0 in case of success, an error code otherwise
  */
void BSP_TimeDelay(uint32_t msDelay)
{
  /* Could be redirected to a Task Delay or to a different custom implementation */
  HAL_Delay(msDelay);
}

/**
  * @brief  MX I2C2 Inititialization.
  * @param  phi2c : I2C handle.
  * @param  timing : I2C timings as described in the I2C peripheral V2 and V3.
  * @retval Prescaler divider
  */
__weak HAL_StatusTypeDef MX_I2C2_Init(I2C_HandleTypeDef *phi2c, uint32_t timing)
{
  HAL_StatusTypeDef ret = HAL_ERROR;

  phi2c->Init.Timing           = timing;
  phi2c->Init.OwnAddress1      = 0;
  phi2c->Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
  phi2c->Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
  phi2c->Init.OwnAddress2      = 0;
  phi2c->Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
  phi2c->Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;

  if (HAL_I2C_Init(phi2c) == HAL_OK)
  {
    if (HAL_I2CEx_ConfigAnalogFilter(phi2c, I2C_ANALOGFILTER_DISABLE) == HAL_OK)
    {
      if (HAL_I2CEx_ConfigDigitalFilter(phi2c, I2C_ANALOG_FILTER_DELAY_DEFAULT) == HAL_OK)
      {
        ret = HAL_OK;
      }
    }
  }

  return ret;
}

/**
  * @brief  Initializes I2C HAL.
  * @retval BSP status
  */
int32_t BSP_I2C2_Init(void)
{
  int32_t ret = BSP_ERROR_BUS_FAILURE;

  hbus_i2c2.Instance  = I2C2;

  if (HAL_I2C_GetState(&hbus_i2c2) == HAL_I2C_STATE_RESET)
  {
    /* Init the I2C Msp */
    I2C2_MspInit(&hbus_i2c2);

    /* Init the I2C */
    if (MX_I2C2_Init(&hbus_i2c2, BUS_I2C2_TIMING) == HAL_OK)
    {
      if (HAL_I2CEx_ConfigAnalogFilter(&hbus_i2c2, I2C_ANALOGFILTER_ENABLE) == HAL_OK)
      {
        ret = BSP_ERROR_NONE;
      }
    }
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  return ret;
}

/**
  * @brief  DeInitializes I2C HAL.
  * @retval BSP status
  */
int32_t BSP_I2C2_DeInit(void)
{
  int32_t ret  = BSP_ERROR_BUS_FAILURE;

  /* DeInit the I2C */
  I2C2_MspDeInit(&hbus_i2c2);

  /* DeInit the I2C */
  if (HAL_I2C_DeInit(&hbus_i2c2) == HAL_OK)
  {
    ret = BSP_ERROR_NONE;
  }

  return ret;
}

/**
  * @brief  Send data through BUS.
  * @param  DevAddr Device address on Bus.
  * @param  pData  Pointer to data buffer to write
  * @param  Length Data Length
  * @retval BSP status
  */
int32_t BSP_I2C2_Send(uint16_t DevAddr, uint8_t *pData, uint16_t Length)
{
  int32_t ret;
  uint32_t hal_error;

  if (HAL_I2C_Master_Transmit(&hbus_i2c2, 
                              DevAddr, 
                              pData, 
                              Length, 
                              BUS_I2C2_POLL_TIMEOUT) == HAL_OK)
  {
    ret = BSP_ERROR_NONE;
  }
  else
  {
    hal_error = HAL_I2C_GetError(&hbus_i2c2);
    if( hal_error == HAL_I2C_ERROR_AF)
    {
      return BSP_ERROR_BUS_ACKNOWLEDGE_FAILURE;
    }
    else
    {
      ret =  BSP_ERROR_PERIPH_FAILURE;
    }
  }
  return ret;
}

/**
  * @brief  Send data through BUS
  * @param  DevAddr Device address on Bus.
  * @param  pData  Pointer to data buffer to read
  * @param  Length Data Length
  * @retval BSP status
  */

int32_t  BSP_I2C2_Recv(uint16_t DevAddr, uint8_t *pData, uint16_t Length)
{
  int32_t ret;
  uint32_t hal_error;

  if (HAL_I2C_Master_Receive(&hbus_i2c2, 
                              DevAddr, 
                              pData, 
                              Length, 
                              BUS_I2C2_POLL_TIMEOUT) == HAL_OK)
  {
    ret = BSP_ERROR_NONE;
  }
  else
  {
    hal_error = HAL_I2C_GetError(&hbus_i2c2);
    
    if( hal_error == HAL_I2C_ERROR_AF)
    {
      return STSAFEA_BUS_NACK;
    }
    else
    {
      ret =  BSP_ERROR_PERIPH_FAILURE;
    }
  }
  return ret;
}


/**
  * @brief  Initializes I2C MSP.
  * @param  hI2c  I2C handler
  * @retval None
  */
static void I2C2_MspInit(I2C_HandleTypeDef *hI2c)
{
  GPIO_InitTypeDef  gpio_init;

  /* Enable I2C clock */
  BUS_I2C2_CLK_ENABLE();

  /* Enable GPIO clock */
  BUS_I2C2_SDA_GPIO_CLK_ENABLE();
  BUS_I2C2_SCL_GPIO_CLK_ENABLE();

  /* Configure I2C SDA Line */
  gpio_init.Pin = BUS_I2C2_SDA_GPIO_PIN;
  gpio_init.Mode = GPIO_MODE_AF_OD;
  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init.Alternate = BUS_I2C2_SDA_GPIO_AF;
  HAL_GPIO_Init(BUS_I2C2_SDA_GPIO_PORT, &gpio_init);

  /* Configure I2C SCL Line */
  gpio_init.Pin = BUS_I2C2_SCL_GPIO_PIN;
  gpio_init.Mode = GPIO_MODE_AF_OD;
  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init.Alternate = BUS_I2C2_SCL_GPIO_AF;
  HAL_GPIO_Init(BUS_I2C2_SCL_GPIO_PORT, &gpio_init);

}

/**
  * @brief  DeInitializes I2C MSP.
  * @param  hI2c  I2C handler
  * @retval None
  */
static void I2C2_MspDeInit(I2C_HandleTypeDef *hI2c)
{
  /* Disable I2C clock */
  __HAL_RCC_I2C2_CLK_DISABLE();

  /* DeInitialize peripheral GPIOs */
  HAL_GPIO_DeInit(BUS_I2C2_SDA_GPIO_PORT, BUS_I2C2_SDA_GPIO_PIN);
  HAL_GPIO_DeInit(BUS_I2C2_SCL_GPIO_PORT, BUS_I2C2_SCL_GPIO_PIN);

}

#else /* STSAFE_ONBOARD*/ 

//

/* I2c defines */ 

#define STSAFEA_VREG_PIN                          GPIO_PIN_14
#define STSAFEA_VREG_GPIO_PORT                    GPIOB
#define STSAFEA_VREG_GPIO_PORT_CLK_ENABLE         __HAL_RCC_GPIOB_CLK_ENABLE
#define STSAFEA_DEVICE_ADDRESS                    0x0020
#define I2C_ANALOG_FILTER_DELAY_DEFAULT        2U      /* ns */
#define BUS_I2C1_INSTANCE                       I2C1
#define BUS_I2C1_CLK_ENABLE()                   __HAL_RCC_I2C1_CLK_ENABLE()
#define BUS_I2C1_CLK_DISABLE()                  __HAL_RCC_I2C1_CLK_DISABLE()
#define BUS_I2C1_SDA_GPIO_PIN                   GPIO_PIN_8
#define BUS_I2C1_SCL_GPIO_PIN                   GPIO_PIN_9
#define BUS_I2C1_SDA_GPIO_PORT                  GPIOB
#define BUS_I2C1_SCL_GPIO_PORT                  GPIOB
#define BUS_I2C1_SDA_GPIO_AF                    GPIO_AF4_I2C1
#define BUS_I2C1_SCL_GPIO_AF                    GPIO_AF4_I2C1
#define BUS_I2C1_SDA_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOA_CLK_ENABLE()
#define BUS_I2C1_SCL_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOA_CLK_ENABLE()
#define BUS_I2C1_POLL_TIMEOUT                0x1000U
#define BUS_I2C1_TIMING                      0x00702991 /* Corresponding to frequency of I2C1 = 400 KHz*/


/* BSP Common Error codes */
   
#define BSP_ERROR_NONE                        0
#define BSP_ERROR_NO_INIT                    -1
#define BSP_ERROR_WRONG_PARAM                -2
#define BSP_ERROR_BUSY                       -3
#define BSP_ERROR_PERIPH_FAILURE             -4
#define BSP_ERROR_COMPONENT_FAILURE          -5
#define BSP_ERROR_UNKNOWN_FAILURE            -6
#define BSP_ERROR_UNKNOWN_COMPONENT          -7 
#define BSP_ERROR_BUS_FAILURE                -8 
#define BSP_ERROR_CLOCK_FAILURE              -9  
#define BSP_ERROR_MSP_FAILURE                -10  
#define BSP_ERROR_FEATURE_NOT_SUPPORTED      -11   
   
/* BSP BUS error codes */

#define BSP_ERROR_BUS_TRANSACTION_FAILURE    -100
#define BSP_ERROR_BUS_ARBITRATION_LOSS       -101
#define BSP_ERROR_BUS_ACKNOWLEDGE_FAILURE    -102
#define BSP_ERROR_BUS_PROTOCOL_FAILURE       -103
#define BSP_ERROR_BUS_MODE_FAULT             -104
#define BSP_ERROR_BUS_FRAME_ERROR            -105
#define BSP_ERROR_BUS_CRC_ERROR              -106
#define BSP_ERROR_BUS_DMA_FAILURE            -107

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef  hbus_i2c1;
/* Private function prototypes -----------------------------------------------*/
int32_t HW_IO_Init(void);
void    BSP_TimeDelay(uint32_t msDelay);
HAL_StatusTypeDef MX_I2C1_Init(I2C_HandleTypeDef *phi2c, uint32_t timing);
int32_t BSP_I2C1_Init(void);
int32_t BSP_I2C1_DeInit(void);
int32_t BSP_I2C1_Send(uint16_t DevAddr, uint8_t *pData, uint16_t Length);
int32_t BSP_I2C1_Recv(uint16_t DevAddr, uint8_t *pData, uint16_t Length);
int32_t CRC16X25_Init(void);
uint32_t CRC_Compute(uint8_t *pData1, uint16_t Length1, uint8_t *pData2, uint16_t Length2);
static void I2C1_MspInit(I2C_HandleTypeDef *hI2c);
static void I2C1_MspDeInit(I2C_HandleTypeDef *hI2c);

#ifdef HAL_CRC_MODULE_ENABLED
CRC_HandleTypeDef hbus_crc;
static __IO uint32_t CRC_CR; /*!< CRC Control register */
#endif /* HAL_CRC_MODULE_ENABLED */

/* Private functions ---------------------------------------------------------*/


/**
  * @brief  Configure STSAFE IO and Bus operation functions to be implemented at User level
  * @param  Ctx the STSAFE IO context
  * @retval 0 in case of success, an error code otherwise
  */
int8_t StSafeA_HW_Probe(void *pCtx)
{
  STSAFEA_HW_t *HwCtx = (STSAFEA_HW_t *)pCtx;


  HwCtx->IOInit     = HW_IO_Init;
  HwCtx->BusInit    = BSP_I2C1_Init;
  HwCtx->BusDeInit  = BSP_I2C1_DeInit;
  HwCtx->BusSend    = BSP_I2C1_Send;
  HwCtx->BusRecv    = BSP_I2C1_Recv;
  HwCtx->CrcInit    = CRC16X25_Init;
  HwCtx->CrcCompute = CRC_Compute;
  HwCtx->TimeDelay  = HAL_Delay;
  HwCtx->DevAddr    = STSAFEA_DEVICE_ADDRESS;

  return STSAFEA_BUS_OK;
}


/**
  * @brief  Additional IO pins configuration needed for STSAFE (VREG pin, etc.)
  * @param  none
  * @retval 0 in case of success, an error code otherwise
  */
int32_t HW_IO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  STSAFEA_VREG_GPIO_PORT_CLK_ENABLE();

  /* Configure GPIO pin : RST Pin */
  GPIO_InitStruct.Pin = STSAFEA_VREG_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(STSAFEA_VREG_GPIO_PORT, &GPIO_InitStruct);

  /* Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(STSAFEA_VREG_GPIO_PORT, STSAFEA_VREG_PIN, GPIO_PIN_SET);
  HAL_Delay(50);

  return BSP_ERROR_NONE;
}


/**
  * @brief  This function provides a delay (in milliseconds)
  * @param  none
  * @retval 0 in case of success, an error code otherwise
  */
void BSP_TimeDelay(uint32_t msDelay)
{
  /* Could be redirected to a Task Delay or to a different custom implementation */
  HAL_Delay(msDelay);
}

/**
  * @brief  MX I2C1 Inititialization.
  * @param  phi2c : I2C handle.
  * @param  timing : I2C timings as described in the I2C peripheral V2 and V3.
  * @retval Prescaler divider
  */
__weak HAL_StatusTypeDef MX_I2C1_Init(I2C_HandleTypeDef *phi2c, uint32_t timing)
{
  HAL_StatusTypeDef ret = HAL_ERROR;

  phi2c->Init.Timing           = timing;
  phi2c->Init.OwnAddress1      = 0;
  phi2c->Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
  phi2c->Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
  phi2c->Init.OwnAddress2      = 0;
  phi2c->Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
  phi2c->Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;

  if (HAL_I2C_Init(phi2c) == HAL_OK)
  {
    if (HAL_I2CEx_ConfigAnalogFilter(phi2c, I2C_ANALOGFILTER_DISABLE) == HAL_OK)
    {
      if (HAL_I2CEx_ConfigDigitalFilter(phi2c, I2C_ANALOG_FILTER_DELAY_DEFAULT) == HAL_OK)
      {
        ret = HAL_OK;
      }
    }
  }

  return ret;
}

/**
  * @brief  Initializes I2C HAL.
  * @retval BSP status
  */
int32_t BSP_I2C1_Init(void)
{
  int32_t ret = BSP_ERROR_BUS_FAILURE;

  hbus_i2c1.Instance  = I2C1;

  if (HAL_I2C_GetState(&hbus_i2c1) == HAL_I2C_STATE_RESET)
  {
    /* Init the I2C Msp */
    I2C1_MspInit(&hbus_i2c1);

    /* Init the I2C */
    if (MX_I2C1_Init(&hbus_i2c1, BUS_I2C1_TIMING) == HAL_OK)
    {
      if (HAL_I2CEx_ConfigAnalogFilter(&hbus_i2c1, I2C_ANALOGFILTER_ENABLE) == HAL_OK)
      {
        ret = BSP_ERROR_NONE;
      }
    }
  }
  else
  {
    ret = BSP_ERROR_NONE;
  }

  return ret;
}

/**
  * @brief  DeInitializes I2C HAL.
  * @retval BSP status
  */
int32_t BSP_I2C1_DeInit(void)
{
  int32_t ret  = BSP_ERROR_BUS_FAILURE;

  /* DeInit the I2C */
  I2C1_MspDeInit(&hbus_i2c1);

  /* DeInit the I2C */
  if (HAL_I2C_DeInit(&hbus_i2c1) == HAL_OK)
  {
    ret = BSP_ERROR_NONE;
  }

  return ret;
}

/**
  * @brief  Send data through BUS.
  * @param  DevAddr Device address on Bus.
  * @param  pData  Pointer to data buffer to write
  * @param  Length Data Length
  * @retval BSP status
  */
int32_t BSP_I2C1_Send(uint16_t DevAddr, uint8_t *pData, uint16_t Length)
{
  int32_t ret;
  uint32_t hal_error;

  if (HAL_I2C_Master_Transmit(&hbus_i2c1, 
                              DevAddr, 
                              pData, 
                              Length, 
                              BUS_I2C1_POLL_TIMEOUT) == HAL_OK)
  {
    ret = BSP_ERROR_NONE;
  }
  else
  {
    hal_error = HAL_I2C_GetError(&hbus_i2c1);
    if( hal_error == HAL_I2C_ERROR_AF)
    {
      return BSP_ERROR_BUS_ACKNOWLEDGE_FAILURE;
    }
    else
    {
      ret =  BSP_ERROR_PERIPH_FAILURE;
    }
  }
  return ret;
}

/**
  * @brief  Send data through BUS
  * @param  DevAddr Device address on Bus.
  * @param  pData  Pointer to data buffer to read
  * @param  Length Data Length
  * @retval BSP status
  */

int32_t  BSP_I2C1_Recv(uint16_t DevAddr, uint8_t *pData, uint16_t Length)
{
  int32_t ret;
  uint32_t hal_error;

  if (HAL_I2C_Master_Receive(&hbus_i2c1, 
                              DevAddr, 
                              pData, 
                              Length, 
                              BUS_I2C1_POLL_TIMEOUT) == HAL_OK)
  {
    ret = BSP_ERROR_NONE;
  }
  else
  {
    hal_error = HAL_I2C_GetError(&hbus_i2c1);
    
    if( hal_error == HAL_I2C_ERROR_AF)
    {
      return STSAFEA_BUS_NACK;
    }
    else
    {
      ret =  BSP_ERROR_PERIPH_FAILURE;
    }
  }
  return ret;
}


/**
  * @brief  Initializes I2C MSP.
  * @param  hI2c  I2C handler
  * @retval None
  */
static void I2C1_MspInit(I2C_HandleTypeDef *hI2c)
{
  GPIO_InitTypeDef  gpio_init;

  /* Enable I2C clock */
  BUS_I2C1_CLK_ENABLE();

  /* Enable GPIO clock */
  BUS_I2C1_SDA_GPIO_CLK_ENABLE();
  BUS_I2C1_SCL_GPIO_CLK_ENABLE();

  /* Configure I2C SDA Line */
  gpio_init.Pin = BUS_I2C1_SDA_GPIO_PIN;
  gpio_init.Mode = GPIO_MODE_AF_OD;
  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init.Alternate = BUS_I2C1_SDA_GPIO_AF;
  HAL_GPIO_Init(BUS_I2C1_SDA_GPIO_PORT, &gpio_init);

  /* Configure I2C SCL Line */
  gpio_init.Pin = BUS_I2C1_SCL_GPIO_PIN;
  gpio_init.Mode = GPIO_MODE_AF_OD;
  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init.Alternate = BUS_I2C1_SCL_GPIO_AF;
  HAL_GPIO_Init(BUS_I2C1_SCL_GPIO_PORT, &gpio_init);

}

/**
  * @brief  DeInitializes I2C MSP.
  * @param  hI2c  I2C handler
  * @retval None
  */
static void I2C1_MspDeInit(I2C_HandleTypeDef *hI2c)
{
  /* Disable I2C clock */
  __HAL_RCC_I2C1_CLK_DISABLE();

  /* DeInitialize peripheral GPIOs */
  HAL_GPIO_DeInit(BUS_I2C1_SDA_GPIO_PORT, BUS_I2C1_SDA_GPIO_PIN);
  HAL_GPIO_DeInit(BUS_I2C1_SCL_GPIO_PORT, BUS_I2C1_SCL_GPIO_PIN);

}

//

#endif /* End */ 

#ifdef HAL_CRC_MODULE_ENABLED

/**
  * @brief   CRC16X25_Init
  *          Initializes CRC X25.
  * @retval  BSP status
  */
int32_t CRC16X25_Init(void)

{
  /* Enable CRC clock */
  __CRC_CLK_ENABLE();

  int32_t ret = BSP_ERROR_NONE;
  hbus_crc.Instance = CRC;
  hbus_crc.Init.DefaultPolynomialUse    = DEFAULT_POLYNOMIAL_DISABLE;
  hbus_crc.Init.DefaultInitValueUse     = DEFAULT_INIT_VALUE_DISABLE;
  hbus_crc.Init.GeneratingPolynomial    = 0x00001021U;
  hbus_crc.Init.CRCLength               = CRC_POLYLENGTH_16B;
  hbus_crc.Init.InitValue               = 0xFFFFFFFFU;
  hbus_crc.Init.InputDataInversionMode  = CRC_INPUTDATA_INVERSION_BYTE;
  hbus_crc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_ENABLE;
  hbus_crc.InputDataFormat              = CRC_INPUTDATA_FORMAT_BYTES;
  switch (HAL_CRC_Init(&hbus_crc))
	{
    case HAL_ERROR:
    case HAL_TIMEOUT:
      ret = BSP_ERROR_PERIPH_FAILURE;
      break;
    case HAL_BUSY:
      ret = BSP_ERROR_BUSY;
      break;
		default:
      break;
	}

  CRC_CR = READ_REG(hbus_crc.Instance->CR);

  return ret;
}
#endif /* HAL_CRC_MODULE_ENABLED */

#ifdef HAL_CRC_MODULE_ENABLED

/**
  * @brief   Compute CRC
  *          Computes the CRC using software solution.
  *          CRC is computed firsly using 1st data starting with initialization value.
  *          CRC is computed secondly using 2nd data starting with the previously computed CRC.
  *
  * @param   pData1  : Pointer to 1st input data buffer.
  * @param   Length1 : Size of 1st input data buffer.
  * @param   pData2  : Pointer to 2nd input data buffer.
  * @param   Length2 : Size of 2nd input data buffer.
  * @retval  uint32_t CRC (returned value LSBs for CRC)
  */

uint32_t CRC_Compute(uint8_t *pData1, uint16_t Length1, uint8_t *pData2, uint16_t Length2)
{
  uint32_t crc = 0;

  WRITE_REG(hbus_crc.Instance->CR, CRC_CR);

  if (pData1 != NULL)
  {
    crc = HAL_CRC_Calculate(&hbus_crc, (uint32_t *)pData1, (uint32_t)Length1);
  }

  if (pData2 != NULL)
  {
    crc = ~HAL_CRC_Accumulate(&hbus_crc, (uint32_t *)pData2, (uint32_t)Length2);
  }

  crc = SWAP2BYTES(crc);

  return crc;
}
#endif
