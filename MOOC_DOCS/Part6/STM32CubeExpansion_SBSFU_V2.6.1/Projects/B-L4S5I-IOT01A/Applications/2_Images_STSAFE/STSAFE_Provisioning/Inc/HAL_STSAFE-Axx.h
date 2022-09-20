/**
  ******************************************************************************
  * @file    HAL_STSAFE-Axx.h
  * @author  MCD Application team
  * @brief   Secure storage API
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

#ifndef HAL_STSAFEA100_H
#define HAL_STSAFEA100_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Exported functions ------------------------------------------------------- */

/*!
 * \brief Initialize the library
 *
 * This command must be executed one time before using other HAL functions
 * It set communication with STSAFE-Axx, verify if STSAFE-Axx is personalized or not
 *
  * \param in/out : handle_se : Handle for STSAFE-A1xx
 * \return 0 if no error else 1
 *
 */

uint8_t Init_HAL(void *handle_se);

/*!
 * \brief Run personalization of chip
 *
 * This command must be executed one time in the life of the product
 * and does personalization of STSAFE-Axx
 *
 * \param in : handle_se : Handle for STSAFE-Axx
 * \param in : perso_type define which personalization to be ran :
 *
 *  perso_type=0: use default keys and meanwhile set buf = NULL
 *  perso_type=1: use random generated keys and meanwhile set buf = NULL
 *  perso_type=2: use the keys customers prefer and meanwhile set buf = preferred key.
 *
 * \param in : buff : Buffer which contains host keys Only used if perso_type=2
 *
 * \return 0 if no error else 1
 *
 */

uint8_t Init_Perso(void *handle_se, uint8_t perso_type, uint8_t *buf);

/*!
 * \brief Store data in STSAFE-Axx
 *
 * This command allow to store unencrypted data inside zone

 * \param in : handle_se : Handle for STSAFE-Axx
 * \param in : zone : zone number
 * \param in : size : Size of data to store
 * \param in : in_Data : data to store
 * \param in : in_Data : offset
 *
 * \return 0 if no error else 1
 *
 */

uint8_t HAL_Store_Data_Zone(void *handle_se, uint8_t zone, uint16_t size, uint8_t *in_Data, uint16_t offset);

/*!
 * \brief Get data inside zone
 *
 * This command allow to get data encrypted inside STSAFE-Axx
 * Max data that could be retrieved  is 800 bytes
 *
 * \param in : handle_se : Handle for STSAFE-Axx
 * \param in : zone      : zone number
 * \param in : size      : Size of data read from zone
 * \param out: buf       : Data read from zone
 * \param in : in_Data   : offset
 * \return 0 if no error else 1
 */

uint8_t HAL_Get_Data_Zone(void *handle_se, uint8_t zone, uint16_t size, uint8_t *buf, uint16_t offset);

/*!
 * \brief Erase data in STSAFE-Axx zone
 *
 * This command set to 0x0 STSAFE-Axx zone
 *
 * Example : for size=5, it set first 5 bytes of zone to 0x0

 * \param in : handle_se : Handle for STSAFE-Axx
 * \param in : zone      : zone number
 * \param in : size      : size of data to be erased
 * \param in : in_Data   : offset
*/

uint8_t HAL_Erase_Data_Zone(void *handle_se, uint8_t zone, uint16_t size, uint16_t offset);

/*!
 * \brief Get Library version
 *
 * This command must be executed one time in the life of the product

 * \param out :  Library version
 *
 * \return 0 if no error else 1
 *
 */

void HAL_Version(char *string);

#ifdef __cplusplus
}

#endif /* __cplusplus */

#endif /* HAL_STSAFEA100_H */
