/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ext_flash_driver.h
  * @author  MCD Application Team
  * @brief   Single core External Flash driver interface
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file in
  * the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef EXT_FLASH_DRIVER_H
#define EXT_FLASH_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

  /* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported constants --------------------------------------------------------*/
#define EXTERNAL_FLASH_ADDRESS  0x90000000U

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  EXT_FLASH_OPERATION_OK,
  EXT_FLASH_OPERATION_ERROR,
}ExtFlashOperationStatus_t;

/* Exported functions ------------------------------------------------------- */

  /**
   * @brief  Implements the external flash initialization
   *
   * @retval Status of the initialization
   */
  uint32_t EXT_FD_Init( void );

  /**
   * @brief  Implements the external flash algorithm to erase multiple sectors
   *         in external flash with CPU1
   *
   * @param  Address:       Flash address to be erased
   * @param  Length:        Number of bytes
   * @retval Status of the operation
   */
  uint32_t EXT_FD_EraseSectors(uint32_t Address, uint32_t Length);

  /**
   * @brief  Implements the external flash algorithm to write multiple 8bits data in flash with CPU1
   *         The user shall first make sure the location to be written has been first erased.
   *         Otherwise, the API will loop for ever as it will be not able to write in flash
   *         The only value that can be written even though the destination is not erased is 0.
   *
   * @param  DestAddress: Address of the flash to write the first data. It shall be 64bits aligned
   * @param  pSrcBuffer:  Address of the buffer holding the 64bits data to be written in flash
   * @param  NbrOfData:   Number of 8bits data to be written
   * @retval: EXT_FLASH_OPERATION_OK -> The data has been written
   *          EXT_FLASH_OPERATION_ERROR -> The data has not been written.
   */
  uint32_t EXT_FD_WriteData(uint32_t DestAddress, uint8_t * pSrcBuffer, uint32_t NbrOfData);


#ifdef __cplusplus
}
#endif

#endif /*EXT_FLASH_DRIVER_H */
