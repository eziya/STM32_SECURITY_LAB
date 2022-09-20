/**
  ******************************************************************************
  * @file    HAL_STSAFE-Axx.c
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "HAL_STSAFE-Axx_INTERNAL.h"
#include "HAL_STSAFE-Axx.h"
#include "Commonappli.h"
#include "flash_if.h"
#include "Commonappli.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/sha256.h"
#include "stsafea_conf.h"
#include "stsafea_core.h"
#include "flash_if.h"

#define STSAFE_A_HAL_VERSION_STRING    "HAL_STSAFE-Axx 3.0.3"  /*!< Full name and version as string */

#pragma pack(push, 1)
#ifdef _MSC_VER
#pragma warning(disable : 4200)
#endif /* End _MSC_VER*/

uint8_t Check_Host_Keys(void *handle, uint8_t perso_type, uint8_t *buf);

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

uint8_t Init_HAL(void *handle_se)

{
  uint8_t StatusCode = STSAFEA_OK;
  StSafeA_HostKeySlotBuffer_t HostKeySlot;

#ifdef PRINTF_ON
  printf("\r\nCheck if Pairing Host keys available \r\n");
#endif /* PRINTF */

  STS_CHK(StatusCode, StSafeA_HostKeySlotQuery(handle_se, &HostKeySlot, STSAFEA_MAC_NONE));

  if (HostKeySlot.HostKeyPresenceFlag == 0U)      /*  Not populated */
  {
#ifdef PRINTF_ON
    printf("\r\nHost key is not set, personalization of STSAFE-A has never been done before !  \r\n");
#endif /* PRINTF */
    return (STSAFEA_KEY_NOT_FOUND);
  }
#ifdef PRINTF_ON
  else
  {
    printf("\r\nHost Key is set : STSAFEA_OK \r\n");
  }
#endif /* PRINTF */

  if (StatusCode != STSAFEA_OK)
  {
#ifdef PRINTF_ON
    printf("\r\nERROR: StatusCode =0x%x \n\r", (uint8_t)StatusCode);
#endif /* PRINTF */
    return (NOK);
  }
  else
  {
    return (STSAFEA_OK);
  }

}

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

uint8_t Init_Perso(void *handle_se, uint8_t perso_type, uint8_t *buf)
{
  int8_t  StatusCode = STSAFEA_OK;

  printf("Check if Pairing Host keys available \r\n");

  STS_CHK(StatusCode, Check_Host_Keys(handle_se, perso_type, buf));

#ifdef PRINTF_ON
  if (StatusCode != STSAFEA_OK)
  {
    printf("ERROR: StatusCode check_host_keys=0x%x \n\r", (uint8_t)StatusCode);
  }
  else
  {
    printf("Check_host_keys STSAFEA_OK \n\r");
  }
#endif /* PRINTF_ON */

#ifdef PRINTF_ON
  printf("Set access condition to host for Zone 2 & 3 & 4\r\n");
#endif /* PRINTF */

  STSAFEA_DYNAMIC_ALLOCATE_LV_BUFFER_VAR(StSafeA_LVBuffer_t, DatatoStore, 1 + 2);
  DatatoStore.Length = 1 ;
  DatatoStore.Data[0] = 0;

  /* Set access condition STSAFEA_AC_HOSTSTfor zones 2 & 3 & 4 */
  for (uint8_t i = 2; i < 5; i++)
  {
    StSafeA_Update(handle_se, 0, 1, 1, STSAFEA_AC_HOST, i, DatatoStore.Length, &DatatoStore, STSAFEA_MAC_HOST_CMAC);
  }

  if (StatusCode != STSAFEA_OK)
  {
#ifdef PRINTF_ON
    printf("ERROR: StatusCode =0x%x \n\r", (uint8_t)StatusCode);
#endif /* PRINTF */
    return (NOK);
  }
  else
  {
    return (STSAFEA_OK);
  }
}

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

uint8_t HAL_Store_Data_Zone(void *handle_se, uint8_t zone, uint16_t size, uint8_t *in_Data, uint16_t offset)
{
  uint32_t  StatusCode = STSAFEA_OK;

  if (size == 0)
  {
#ifdef PRINTF_ON
    printf("Size of data's must not equals zero \n\r");
#endif /* PRINTF */
    return (STSAFEA_VALUE_OUT_OF_RANGE);
  }

  STSAFEA_DYNAMIC_ALLOCATE_LV_BUFFER_VAR(StSafeA_LVBuffer_t, DatatoStore, size + 2);

  DatatoStore.Length = size ;
  memcpy(DatatoStore.Data, in_Data, size);

  if (!StatusCode)
  {
    STS_CHK(StatusCode, UpdateZone(handle_se, zone, DatatoStore, offset));

#ifdef PRINTF_ON
    if (StatusCode != STSAFEA_OK)
    {
      printf("ERROR: StatusCode UpdateZone=0x%x \n\r", (uint8_t)StatusCode);
    }
    else
    {
      printf("UpdateZone STSAFEA_OK \n\r");
    }
#endif /* PRINTF */
  }

  STSAFEA_DYNAMIC_FREE_LV_BUFFER_VAR(DatatoStore);

  if (StatusCode != STSAFEA_OK)
  {
#ifdef PRINTF_ON
    printf("ERROR: StatusCode =0x%x \n\r", (uint8_t)StatusCode);
#endif /* PRINTF */
    return (NOK);
  }
  else
  {
    return (STSAFEA_OK);
  }
}

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

uint8_t HAL_Get_Data_Zone(void *handle_se, uint8_t zone, uint16_t size, uint8_t *buf, uint16_t offset)

{
  uint32_t StatusCode = STSAFEA_OK;

  if (size == 0 || size >=MAX_SIZE_ZONE_STSAFE_A)
  {
#ifdef PRINTF_ON
    printf("Size of data's must be in between 1 to size of zone \n\r");
#endif /* PRINTF */
    return (STSAFEA_VALUE_OUT_OF_RANGE);
  }

  StSafeA_LVBuffer_t sts_read;
  uint8_t data_sts_read [size];
  sts_read.Length = size;
  sts_read.Data = data_sts_read;

  if (size <= MAX_SIZE_BUFFER_I2_FOR_DATAS)
  {

    STS_CHK(StatusCode, StSafeA_Read(
              handle_se, /* handle */
              0 /*in_change_ac_indicator*/,
              0 /*in_new_read_ac_right*/,
              STSAFEA_AC_ALWAYS /*in_new_read_ac*/,
              zone,
              offset /*in_offset*/,
              size/*in_length*/,
              size/*in_length*/,
              &sts_read /*out_read*/,
              STSAFEA_MAC_HOST_CMAC)
           );

    if (StatusCode != STSAFEA_OK)
    {
#ifdef PRINTF_ON
      printf("ERROR: StatusCode Read Zone=0x%x \n\r", (uint8_t)StatusCode);
#endif /* PRINTF */
    }
    else
    {

#ifdef PRINTF_ON
      printf("Read Zone STSAFEA_OK \n\r");
#endif /* PRINTF */
      memcpy(buf, sts_read.Data, sts_read.Length);
    }
  }
  else /* length exceed MAX_SIZE_BUFFER_I2_FOR_DATAS */
  {
#ifdef PRINTF_ON
    printf("length exceed MAX_SIZE_BUFFER_I2_FOR_DATAS it's needed to read by blocks\n\r");
#endif /* PRINTF */


    STS_CHK(StatusCode, StSafeA_Read(
              handle_se, /* handle */
              0 /*in_change_ac_indicator*/,
              0 /*in_new_read_ac_right*/,
              STSAFEA_AC_ALWAYS /*in_new_read_ac*/,
              zone,
              offset /*in_offset*/,
              MAX_SIZE_BUFFER_I2_FOR_DATAS,/*in_length*/
              MAX_SIZE_BUFFER_I2_FOR_DATAS,/*in_length*/
              &sts_read /*out_read*/,
              STSAFEA_MAC_HOST_CMAC)
           );

    memcpy(buf, sts_read.Data, sts_read.Length);
    uint16_t Remaining_Data_To_Read = size - MAX_SIZE_BUFFER_I2_FOR_DATAS;

    if (Remaining_Data_To_Read <= MAX_SIZE_BUFFER_I2_FOR_DATAS) /* 2 read is enough to read data's*/
    {

      STS_CHK(StatusCode, StSafeA_Read(
                handle_se, /* handle */
                0 /*in_change_ac_indicator*/,
                0 /*in_new_read_ac_right*/,
                STSAFEA_AC_ALWAYS /*in_new_read_ac*/,
                zone,
                offset + MAX_SIZE_BUFFER_I2_FOR_DATAS /*in_offset*/,
                Remaining_Data_To_Read,/*in_length*/
                Remaining_Data_To_Read,/*in_length*/
                &sts_read /*out_read*/,
                STSAFEA_MAC_HOST_CMAC)
             );

      for (uint16_t i = MAX_SIZE_BUFFER_I2_FOR_DATAS; i < size; i++)
      {
        buf[i] = sts_read.Data[i - MAX_SIZE_BUFFER_I2_FOR_DATAS];
      }

    }
    else /* 3 read is enough to read data's*/
    {

      STS_CHK(StatusCode, StSafeA_Read(
                handle_se, /* handle */
                0 /*in_change_ac_indicator*/,
                0 /*in_new_read_ac_right*/,
                STSAFEA_AC_ALWAYS /*in_new_read_ac*/,
                zone,
                offset + MAX_SIZE_BUFFER_I2_FOR_DATAS /*in_offset*/,
                MAX_SIZE_BUFFER_I2_FOR_DATAS,/*in_length*/
                MAX_SIZE_BUFFER_I2_FOR_DATAS,/*in_length*/
                &sts_read /*out_read*/,
                STSAFEA_MAC_HOST_CMAC)
             );

      for (uint16_t i = MAX_SIZE_BUFFER_I2_FOR_DATAS; i < 2 * MAX_SIZE_BUFFER_I2_FOR_DATAS; i++)
      {
        buf[i] = sts_read.Data[i - MAX_SIZE_BUFFER_I2_FOR_DATAS];
      }

      Remaining_Data_To_Read = size - 2 * MAX_SIZE_BUFFER_I2_FOR_DATAS;

      STS_CHK(StatusCode, StSafeA_Read(
                handle_se, /* handle */
                0 /*in_change_ac_indicator*/,
                0 /*in_new_read_ac_right*/,
                STSAFEA_AC_ALWAYS /*in_new_read_ac*/,
                zone,
                offset + 2 * MAX_SIZE_BUFFER_I2_FOR_DATAS /*in_offset*/,
                Remaining_Data_To_Read,/*in_length*/
                Remaining_Data_To_Read,/*in_length*/
                &sts_read /*out_read*/,
                STSAFEA_MAC_HOST_CMAC)
             );


      for (uint8_t i = 0; i < Remaining_Data_To_Read; i++)
      {
        buf[i + 2 * MAX_SIZE_BUFFER_I2_FOR_DATAS] = sts_read.Data[i];
      }
    }
  }

  if (StatusCode != STSAFEA_OK)
  {
#ifdef PRINTF_ON
    printf("ERROR: StatusCode =0x%x \n\r", (uint8_t)StatusCode);
#endif /* PRINTF */
    return (NOK);
  }
  else
  {
    return (STSAFEA_OK);
  }
}

/*!
 * \brief Erase data in STSAFE-Axx zone
 *
 * This command set to 0x0 STSAFE-Axx zone
 *
 * Example : for size=5, it set first 5 bytes of zone to 0x0

 * \param in : *handle_se : Handle for STSAFE-Axx
 * \param in : zone      : zone number
 * \param in : size      : size of data to be erased
 * \param in : in_Data   : offset
*/

uint8_t HAL_Erase_Data_Zone(void *handle_se, uint8_t zone, uint16_t size, uint16_t offset)
{
  uint32_t StatusCode = STSAFEA_OK;

  if (size == 0)
  {
#ifdef PRINTF_ON
    printf("Size of data's must be in between 1 to size of zone \n\r");
#endif /* PRINTF */
    return (STSAFEA_INCONSISTENT_COMMAND_DATA);
  }

  STSAFEA_DYNAMIC_ALLOCATE_LV_BUFFER_VAR(StSafeA_LVBuffer_t, DatatoStore, size + 2);

  DatatoStore.Length = size ;

  memset(DatatoStore.Data, 0x0, size);

  if (!StatusCode)
  {
    STS_CHK(StatusCode, UpdateZone(handle_se, zone, DatatoStore, offset));

#ifdef PRINTF_ON
    if (StatusCode != STSAFEA_OK)
    {
      printf("ERROR: StatusCode UpdateZone=0x%x \n\r", (uint8_t)StatusCode);
    }
    else
    {
      printf("UpdateZone STSAFEA_OK \n\r");
    }
#endif /* PRINTF */

  }

  STSAFEA_DYNAMIC_FREE_LV_BUFFER_VAR(DatatoStore);

  if (StatusCode != STSAFEA_OK)
  {
#ifdef PRINTF_ON
    printf("ERROR: StatusCode =0x%x \n\r", (uint8_t)StatusCode);
#endif /* PRINTF */
    return (NOK);
  }
  else
  {
    return (STSAFEA_OK);
  }
}

/*!
 * \brief Get Library version
 *
 * This command must be executed one time in the life of the product
 * \param out :  Library version
 *
 * \return 0 if no error else 1
 *
 */

void HAL_Version(char *string)
{
  if (string != NULL)
  {
    memcpy(string, STSAFE_A_HAL_VERSION_STRING, sizeof(STSAFE_A_HAL_VERSION_STRING));
  }
}

#if defined (__ARMCC_VERSION)
#pragma pack(pop)
#endif /* __ARMCC_VERSION */
