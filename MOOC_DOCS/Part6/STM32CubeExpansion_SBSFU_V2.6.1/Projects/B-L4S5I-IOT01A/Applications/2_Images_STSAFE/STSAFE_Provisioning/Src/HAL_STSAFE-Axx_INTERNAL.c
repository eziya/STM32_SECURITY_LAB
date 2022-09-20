/**
  ******************************************************************************
  * @file    HAL_STSAFE-Axx_INTERNAL.c
  * @author  MCD Application team
  * @brief   Secure storage API, functions not exported to the outside world
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
#include "Commonappli.h"
#include "flash_if.h"
#include "HAL_STSAFE-Axx.h"


#pragma pack(push, 1)
#ifdef _MSC_VER
#pragma warning(disable : 4200)
#endif /* _MSC_VER */

/* Public function prototypes -----------------------------------------------*/

/*!
 * \brief Update data through zone partition
 *
 * \param *handle                : Handle for STSAFEA_A interface (must be created before any usage)
 * \param zone                   : Zone number
 * \param in_Data                : Data to copy inside zone
 * \param offset                 : Offset in selected zone
 * \return OK if no error
 */

uint8_t UpdateZone(void *handle, uint8_t zone, StSafeA_LVBuffer_t in_Data, uint16_t offset)
{
  uint8_t StatusCode = STSAFEA_OK;
  uint16_t Remaining_Data_To_Copy = 0;

  if (in_Data.Length <= MAX_SIZE_BUFFER_I2_FOR_DATAS)
  {

    STS_CHK(StatusCode, StSafeA_Update(handle, 0, 0, 0, 0, zone, offset, &in_Data, STSAFEA_MAC_HOST_CMAC));

#ifdef PRINTF_ON
    if (StatusCode != STSAFEA_OK)
    {
      printf("ERROR: StatusCode StSafeA_Update=0x%x \n\r", (uint8_t)StatusCode);
    }
    else
    {
      printf("StSafeA_Update STSAFEA_OK \n\r");
    }
#endif /* PRINTF */
  }
  else /* Size of data to be send exceed max size of STSAFE-Axx buffer */
  {
#ifdef PRINTF_ON
    printf("Size of data's to be send exceed max size of STSAFE-Axx buffer \n\r");
    printf("We have to split data's in blocks \n\r");
#endif /* PRINTF */

    uint16_t size = in_Data.Length;

    /* First Write of data*/

    in_Data.Length = MAX_SIZE_BUFFER_I2_FOR_DATAS;

    STS_CHK(StatusCode, StSafeA_Update(handle, 0, 0, 0, 0, zone, offset, &in_Data, STSAFEA_MAC_HOST_CMAC));

#ifdef PRINTF_ON

    if (StatusCode != STSAFEA_OK)
    {
      printf("ERROR: StatusCode StSafeA_Update=0x%x \n\r", (uint8_t)StatusCode);
    }
    else
    {
      printf("First StSafeA_Update STSAFEA_OK \n\r");
    }

    printf("Size of data to copy %d \n\r", size - MAX_SIZE_BUFFER_I2_FOR_DATAS);

#endif /* PRINTF */
    /* Management of 2nd block of data with max data of 1000 bytes*/
    /* Check if it's needed to split in 2 */

    Remaining_Data_To_Copy = size - MAX_SIZE_BUFFER_I2_FOR_DATAS;

    if (Remaining_Data_To_Copy <= MAX_SIZE_BUFFER_I2_FOR_DATAS)
    {
      STSAFEA_DYNAMIC_ALLOCATE_LV_BUFFER_VAR(StSafeA_LVBuffer_t, DataUpdate, Remaining_Data_To_Copy + 2);
      DataUpdate.Length = Remaining_Data_To_Copy;

      for (uint8_t i = 0; i < DataUpdate.Length; i++)
      {
        DataUpdate.Data[i] = in_Data.Data[i + MAX_SIZE_BUFFER_I2_FOR_DATAS];
      }

      /* 2nd Write of data*/

      STS_CHK(StatusCode, StSafeA_Update(handle, 0, 0, 0, 0, zone, MAX_SIZE_BUFFER_I2_FOR_DATAS + offset, &DataUpdate,
                                         STSAFEA_MAC_HOST_CMAC));

#ifdef PRINTF_ON

      if (StatusCode != STSAFEA_OK)
      {
        printf("ERROR: StatusCode StSafeA_Update=0x%x \n\r", (uint8_t)StatusCode);
      }
      else
      {
        printf("Second StSafeA_Update STSAFEA_OK \n\r");
      }

      printf("Size of data to copy %d \n\r", Remaining_Data_To_Copy);

#endif /* PRINTF */
      STSAFEA_DYNAMIC_FREE_LV_BUFFER_VAR(DataUpdate);
    }
    else /*  it's needed to split in 3 blocks*/
    {
#ifdef PRINTF_ON
      printf("%d remaining data's, it's needed to split in 2 more blocks \n\r", Remaining_Data_To_Copy);
#endif /* PRINTF */

      STSAFEA_DYNAMIC_ALLOCATE_LV_BUFFER_VAR(StSafeA_LVBuffer_t, DataUpdate, MAX_SIZE_BUFFER_I2_FOR_DATAS + 2);

      DataUpdate.Length = MAX_SIZE_BUFFER_I2_FOR_DATAS;

      for (uint8_t i = 0; i < DataUpdate.Length; i++)
      {
        DataUpdate.Data[i] = in_Data.Data[i + MAX_SIZE_BUFFER_I2_FOR_DATAS];
      }

      /* 2nd Write of data*/

      STS_CHK(StatusCode, StSafeA_Update(handle, 0, 0, 0, 0, zone, MAX_SIZE_BUFFER_I2_FOR_DATAS + offset, &DataUpdate,
                                         STSAFEA_MAC_HOST_CMAC));

      STSAFEA_DYNAMIC_FREE_LV_BUFFER_VAR(DataUpdate);

#ifdef PRINTF_ON
      if (StatusCode != STSAFEA_OK)
      {
        printf("ERROR: StatusCode StSafeA_Update=0x%x \n\r", (uint8_t)StatusCode);
      }
      else
      {
        printf("Second StSafeA_Update STSAFEA_OK \n\r");
      }
#endif /* PRINTF */
      Remaining_Data_To_Copy = size - 2 * MAX_SIZE_BUFFER_I2_FOR_DATAS;

#ifdef PRINTF_ON

      printf("Remaining Size of data to copy %d \n\r", Remaining_Data_To_Copy);

#endif /* PRINTF */
      STSAFEA_DYNAMIC_ALLOCATE_LV_BUFFER_VAR(StSafeA_LVBuffer_t, DataUpdate2, Remaining_Data_To_Copy + 2);
      DataUpdate2.Length = Remaining_Data_To_Copy;

      for (uint8_t i = 0; i < DataUpdate2.Length; i++)
      {
        DataUpdate2.Data[i] = in_Data.Data[i + 2 * MAX_SIZE_BUFFER_I2_FOR_DATAS];
      }

      /* third Write of data*/

      STS_CHK(StatusCode, StSafeA_Update(handle,
                                         0,
                                         0,
                                         0,
                                         0,
                                         zone,
                                         2 * MAX_SIZE_BUFFER_I2_FOR_DATAS + offset, &DataUpdate2,
                                         STSAFEA_MAC_HOST_CMAC
                                        ));

#ifdef PRINTF_ON

      if (StatusCode != STSAFEA_OK)
      {
        printf("ERROR: StatusCode StSafeA_Update=0x%x \n\r", (uint8_t)StatusCode);
      }
      else
      {
        printf("Third StSafeA_Update STSAFEA_OK \n\r");
      }
#endif /* PRINTF */
      STSAFEA_DYNAMIC_FREE_LV_BUFFER_VAR(DataUpdate2);
    }
  }
  return (StatusCode);
}

/*!
 * \brief Check host keys presence and if not existing enable it in STSAFE-Axx and store value in flash
 *
 * \param *handle                : Handle for STSAFEA_A interface (must be created before any usage)
 * \param perso_type             : type of perso as explained below
 *
 *    type=0: use default keys and meanwhile set buf = NULL
 *    type=1: use random generated keys and meanwhile set buf = NULL
 *    type=2: use the keys customers prefer and meanwhile set buf = preferred key.
 *
 * \param buf                   : Host Mac and Cipher key value
 * \return OK if no error
 */

uint8_t Check_Host_Keys(void *handle, uint8_t perso_type, uint8_t *buf)
{
  uint8_t StatusCode = 0;
  uint8_t Host_MAC_Cipher_Key[SIZE_HOST_MAC_CIPHER_KEY] =
  {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, /* Host MAC key */
    0x11, 0x11, 0x22, 0x22, 0x33, 0x33, 0x44, 0x44, 0x55, 0x55, 0x66, 0x66, 0x77, 0x77, 0x88, 0x88 /* Host cipher key */
  };
  uint32_t i;
  StSafeA_HostKeySlotBuffer_t HostKeySlot;

  switch (perso_type)
  {
    case 0 :
#ifdef PRINTF_ON
      printf("use default keys and meanwhile set buf = NULL\n");
#endif /* PRINTF */
      break;
    case 1 :
#ifdef PRINTF_ON
      printf("use random generated keys and meanwhile set buf = NULL\n");
#endif /* PRINTF */
      STS_CHK(StatusCode, GenerateUnsignedChallenge(handle,
                                                    sizeof(Host_MAC_Cipher_Key) / sizeof(Host_MAC_Cipher_Key[0]),
                                                    Host_MAC_Cipher_Key
                                                   ));
      break;
    case 2 :
#ifdef PRINTF_ON
      printf("use the keys customers prefer and meanwhile set buf = preferred key\n");
#endif /* PRINTF */
      memcpy(Host_MAC_Cipher_Key, buf, 32);

      break;
#ifdef PRINTF_ON
    default :
      printf("Invalid choice\n");
#endif /* PRINTF */
  }
  /* Check if host cipher key & host MAC key are populated */
  STS_CHK(StatusCode, StSafeA_HostKeySlotQuery(handle, &HostKeySlot, STSAFEA_MAC_NONE));

  if (HostKeySlot.HostKeyPresenceFlag == 0U)      /* Not populated */
    STS_CHK(StatusCode, StSafeA_PutAttribute(handle, STSAFEA_TAG_HOST_KEY_SLOT, Host_MAC_Cipher_Key,
                                             sizeof(Host_MAC_Cipher_Key) / sizeof(Host_MAC_Cipher_Key[0]),
                                             STSAFEA_MAC_NONE));

  /* Store in flash Host Mac & Data encryption key */
  /* Generating code for key storing as ARM instructions */

  /* Values of Host keys written in below code are loaded in STM32 register r0 to r7 */
  /* This is the function PCROP_Write_Mac_Keys_To_Registers called by secure bootloader */
  /* This function will be written in PCROP area */
  /* on then flushed in RAM with pointer given as argument */

  uint16_t PCROP_Code_buff[44];

  PCROP_Code_buff[0] =  0xb43e;
  PCROP_Code_buff[1] =  0xf240;
  PCROP_Code_buff[2] =  0x0000;
  PCROP_Code_buff[3] =  0xf2c0;
  PCROP_Code_buff[4] =  0x0000;
  PCROP_Code_buff[5] =  0xf240;
  PCROP_Code_buff[6] =  0x0000;
  PCROP_Code_buff[7] =  0xf2c0;
  PCROP_Code_buff[8] =  0x0000;
  PCROP_Code_buff[9] =  0xf240;
  PCROP_Code_buff[10] = 0x0000;
  PCROP_Code_buff[11] = 0xf2c0;
  PCROP_Code_buff[12] = 0x0000;
  PCROP_Code_buff[13] = 0xf240;
  PCROP_Code_buff[14] = 0x0000;
  PCROP_Code_buff[15] = 0xf2c0;
  PCROP_Code_buff[16] = 0x0000;

  PCROP_Code_buff[17] = 0xe880;
  PCROP_Code_buff[18] = 0x001e;

  PCROP_Code_buff[19] = 0xf100;
  PCROP_Code_buff[20] = 0x0010;

  PCROP_Code_buff[21] = 0xf240;
  PCROP_Code_buff[22] = 0x0000;
  PCROP_Code_buff[23] = 0xf2c0;
  PCROP_Code_buff[24] = 0x0000;
  PCROP_Code_buff[25] = 0xf240;
  PCROP_Code_buff[26] = 0x0000;
  PCROP_Code_buff[27] = 0xf2c0;
  PCROP_Code_buff[28] = 0x0000;
  PCROP_Code_buff[29] = 0xf240;
  PCROP_Code_buff[30] = 0x0000;
  PCROP_Code_buff[31] = 0xf2c0;
  PCROP_Code_buff[32] = 0x0000;
  PCROP_Code_buff[33] = 0xf240;
  PCROP_Code_buff[34] = 0x0000;
  PCROP_Code_buff[35] = 0xf2c0;
  PCROP_Code_buff[36] = 0x0000;
  PCROP_Code_buff[37] = 0xe880;
  PCROP_Code_buff[38] = 0x001e;
  PCROP_Code_buff[39] = 0xbc3e;
  PCROP_Code_buff[40] = 0x4770;
  PCROP_Code_buff[41] = 0x0000;
  PCROP_Code_buff[42] = 0x0000;
  PCROP_Code_buff[43] = 0x0000;

  for (i = 0; i < 8; i++)
  {
    unsigned char zz;
    unsigned char yy;

    zz = Host_MAC_Cipher_Key[(2 * i) + 1];
    yy = Host_MAC_Cipher_Key[2 * i];

    PCROP_Code_buff[1 + 2 * i] |= (((zz & 0x08) >> 1) << 8) | ((zz & 0xF0) >> 4);
    PCROP_Code_buff[1 + 2 * i + 1] |= ((((zz & 0x07) << 4) | (1 + i / 2)) << 8) | yy;

    zz = Host_MAC_Cipher_Key[(2 * i) + 1 + 16];
    yy = Host_MAC_Cipher_Key[2 * i + 16];
    PCROP_Code_buff[21 + 2 * i] |= (((zz & 0x08) >> 1) << 8) | ((zz & 0xF0) >> 4);
    PCROP_Code_buff[21 + 2 * i + 1] |= ((((zz & 0x07) << 4) | (1 + i / 2)) << 8) | yy;
  }

  /* Store Keys in clear inside STM32 flash*/
  STS_CHK(StatusCode, WriteUnsignedCharArrayToFlash((unsigned char *)Host_MAC_Cipher_Key, SIZE_HOST_MAC_CIPHER_KEY,
                                                    KEY_ADDR));
  /* Store executable code into STM32 flash */
  STS_CHK(StatusCode, WriteUnsignedCharArrayToFlash((unsigned char *)PCROP_Code_buff, sizeof(PCROP_Code_buff),
                                                    PCROP_ADDR));

  return StatusCode;
}

/* Private function prototypes -----------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/*!
 * \brief Write unsigned char array in STM32 flash at chosen address
 *
 * \param in : *MyArray : Array to write
 * \param in : SizeMyArray : zize of array
 * \param in : Addr : Address to write in STM32 flash
 * \return OK if no error
 */



uint8_t WriteUnsignedCharArrayToFlash(unsigned char *MyArray, uint8_t SizeMyArray, uint32_t Addr)
{
  uint8_t StatusCode = 0;
  static FLASH_EraseInitTypeDef EraseInitStruct; 
  uint32_t PAGEError = 0;
  uint32_t FirstPage=0;
  
  HAL_FLASH_Unlock();
  
  /* Clear OPTVERR bit set on virgin samples */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR); 
  
  FirstPage = GetPage(Addr);
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.Page=FirstPage;
  EraseInitStruct.NbPages=1;      
  EraseInitStruct.Banks=GetBank(Addr);
  
  /* Below configuration is OK only if User configuration Option bytes DBANK = 0 (Dual bank is disabled) */ 

  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) == HAL_OK)
  {    
    /* Programming flash */
    for (uint8_t i = 0; i < SizeMyArray ; i += 8)
    {
      StatusCode |= HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Addr + i, *(uint64_t *)(MyArray + i));
    }
  }
  else
  {
    StatusCode=HAL_ERROR;    
  }

  HAL_FLASH_Lock();

  return (StatusCode);
}

/*!
 * \brief challenge inside a Bytes Array
 *
 * \param in : *handle_se : Handle for STSAFE-Axx
 * \param in : size      : size of challenge to be generated
 * \param in : out_Data  : challenge
 * \return OK if no error
 */

uint8_t GenerateUnsignedChallenge(void *handle, uint32_t size, uint8_t *buf)
{
  uint8_t StatusCode = 0;

  STSAFEA_DYNAMIC_ALLOCATE_LV_BUFFER_VAR(StSafeA_LVBuffer_t, K, size + 2);

  if (buf)
  {

    STS_CHK(StatusCode, StSafeA_GenerateRandom(handle,
                                               STSAFEA_EPHEMERAL_RND,
                                               size / sizeof(uint8_t),
                                               &K,
                                               STSAFEA_MAC_NONE
                                              ));
    if (!StatusCode)
    {
      memcpy(buf, K.Data, size * sizeof(uint8_t));
    }
  }

  return (StatusCode);
}

#if defined (__ARMCC_VERSION)
#pragma pack(pop)
#endif /* __ARMCC_VERSION */
