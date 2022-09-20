/**
  ******************************************************************************
  * @file    sfu_fwimg_counter.c
  * @author  MCD Application Team
  * @brief   This file provides all the Non volatile firmware functions to manage
  *          Secure Counter mechanism.
  @verbatim
  ==============================================================================
                        ##### How to use this driver #####
  ==============================================================================
    [..]
      This driver provides functions to initialize Non Volatile Secure Counter Storage,
      to read and write Non volatile Monothonic Secure Counters.
      This is because Active Image Header is stored into Ext.Flash, so it would be
      possible for an attacker to replace header and Active Image with a previous valid one,
      in order to exploit weakness of previous valid Fw version.

  @endverbatim
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

#define SFU_FWIMG_COUNTER_C

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "app_sfu.h"
#include "sfu_fwimg_counter.h"
#include "sfu_low_level.h"
#include "sfu_low_level_flash.h"
#include "sfu_low_level_flash_int.h"
#include "sfu_fwimg_regions.h"

#include "sfu_trace.h"

/*
*   Introduction to Secure Counter for Anti-Rollback Mechanism
*   -----------------------------------------------------------
*
*   The Secure Counter is implemented on an area in secure internal Flash.
*   The area is accessed only by the secureboot and is hidden to user
*   application with HDP option bytes.
*
*   This area is written by payload of 8 bytes and the payload content is described as follow:
*
 *   byte 0                      byte 4        byte 6
*   +------+------+------+------+------+------+------+------+
*   | value                     | CRC         | ID          |
*   +------+------+------+------+------+------+------+------+
*
*   The bytes [0 .. 3] contains a value
*   The CRC is computed on bytes [0 .. 3] , the CRC Ip is used, The CRC16 polynomial is 0x8005U.
*   The bytes [6 .. 7] contains an identfication number.
*
*   Initialisation
*   --------------
*
*   Up to 3 counters (SLOT_ACTIVE_1_COUNTER, SLOT_ACTIVE_2_COUNTER, SLOT_ACTIVE_3_COUNTER), are initialized with 0.
*
*   +------+------+------+------+------+------+------+------+
*   |  0x0    0x0   0x0    0x0  | CRC         | 0x0   0x2   |
*   +------+------+------+------+------+------+------+------+ SECURE_COUNTER_REGION_ROM_START + 2 * NVCNT_OFFSET_SIZE
*   |  0x0    0x0   0x0    0x0  | CRC         | 0x0   0x1   |
*   +------+------+------+------+------+------+------+------+ SECURE_COUNTER_REGION_ROM_START + NVCNT_OFFSET_SIZE
*   |  0x0    0x0   0x0    0x0  | CRC         | 0x0   0x0   |
*   +------+------+------+------+------+------+------+------+ SECURE_COUNTER_REGION_ROM_START
*
*   WARNING :Initialization does not implement recovery in case of reset or powerdown.
*
*   Update of SLOT_ACTIVE_X_COUNTER
*   -------------------------------
*   During image installation, the counter is updated with the new version being
*   installed.
*   The new value to be written is check against last valid counter being written.
*   Counter Value smaller than last valid counter value being written are not written and an error is return.
*   The new value of counter is written after the last valid counter value being
*   written and so on till the Counter NVM area counter is fully written.
*
*
*   Start up check
*   --------------
*
*   At every reset, power up, the Counter NVM area is checked by secureboot :
*   Up to 3 counters (SLOT_ACTIVE_1_COUNTER, SLOT_ACTIVE_2_COUNTER, SLOT_ACTIVE_3_COUNTER) are read else an
*   error is reported.
*   For that complete Secure Counter area is read :
*   - to verify that area not written is contiguous see (1),
*   - to check the historic of counter value being written see (2).
*   If some inconsistency is detected an error is reported.
*
*   +-------------------+ +
*   |                   | |
*   |                   | |
*   |                   | | (1)contiguous area not written
*   |                   | |
*   |                   | |
*   |                   | v
*   +-------------------+
*   | last counter      | +
*   +-------------------+ |
*                         |
*   ....                  | (2)check that the counter value in history
*                         |  decreases till the value 0.
*                         |
*                         |
*                         |
*   +-------------------+ |
*   | counter  = 0      | V
*   +-------------------+    SECURE_COUNTER_REGION_ROM_START
*
*/


/* Private typedef -----------------------------------------------------------*/
/* Secure Counter id type
   Support up to 3 active images */
typedef enum  {
    SLOT_ACTIVE_1_COUNTER = 1,
#if (SFU_NB_MAX_ACTIVE_IMAGE > 1U)
    SLOT_ACTIVE_2_COUNTER,
#endif  /* (NB_FW_IMAGES > 1) */
#if (SFU_NB_MAX_ACTIVE_IMAGE > 2U)
    SLOT_ACTIVE_3_COUNTER,
#endif  /* (NB_FW_IMAGES > 2) */
    NV_COUNTER_MAX
} NVCounterTypeDef;

/* Private defines -----------------------------------------------------------*/
#define NVCNT_ELEMENT_TYPE    uint64_t                      /*!< Complete element : data + CRC + ID  */
#define NVCNT_ELEMENT_SIZE    sizeof(NVCNT_ELEMENT_TYPE)
#define NVCNT_ID_TYPE         uint16_t                      /*!< Counter ID */
#define NVCNT_ID_SHIFT        0U                            /*!< Bits Shifting to get counter ID in element */
#define NVCNT_CRC_TYPE        uint16_t                      /*!< Crc */
#define NVCNT_CRC_SHIFT       16U                           /*!< Bits Shifting to get Crc in element */
#define NVCNT_CRC_DUMMY_VALUE (uint16_t)0xA5A5              /*!< Crc dummy value if CRC is not implemented */
#define NVCNT_DATA_TYPE       uint32_t                      /*!< Type of Data */
#define NVCNT_DATA_SHIFT      32U                           /*!< Bits Shifting to get Data value in element */
#define NVCNT_MASK_ID         (uint64_t)0x000000000000FFFFU
#define NVCNT_MASK_CRC        (uint64_t)0x00000000FFFF0000U
#define NVCNT_MASK_DATA       (uint64_t)0xFFFFFFFF00000000U
#define NVCNT_MASK_FULL       (uint64_t)0xFFFFFFFFFFFFFFFFU
#define NVCNT_PATTERN_ERASE   (uint64_t)0xFFFFFFFFFFFFFFFFU /*!< Flash value after erase */
#define NVCNT_MAX_WRITTEN_ELEMENTS (SECURE_COUNTER_REGION_ROM_SIZE / NVCNT_OFFSET_SIZE)
#define NVCNT_INIT_VALUE      SFU_FW_VERSION_START_NUM

#define CRC_POLYNOMIAL_LENGTH      LL_CRC_POLYLENGTH_16B         /*!< CRC polynomial length 16 bits */
#define CRC_POLYNOMIAL_VALUE       0x8005U                       /*!< Polynomial to use for CRC calculation */

#define NVCNT_OFFSET_SIZE          (NVCNT_ELEMENT_SIZE > sizeof(SFU_LL_FLASH_write_t) ?\
                                    NVCNT_ELEMENT_SIZE : sizeof(SFU_LL_FLASH_write_t))  /*! offset between two counters */

/* Private macros ------------------------------------------------------------*/
/*!< Get Crc value from element value */
#define NVCNT_CRC_VALUE(__ELEMENT__) (NVCNT_CRC_TYPE)(((__ELEMENT__) & NVCNT_MASK_CRC) >> NVCNT_CRC_SHIFT)

/*!< Get id value from element value */
#define NVCNT_ID_VALUE(__ELEMENT__) (NVCNT_ID_TYPE)((__ELEMENT__) & NVCNT_MASK_ID)

/*!< Get Data value from element value*/
#define NVCNT_DATA_VALUE(__ELEMENT__) (NVCNT_DATA_TYPE)(((__ELEMENT__)\
                                                         & NVCNT_MASK_DATA) >> NVCNT_DATA_SHIFT)

/*!< Get element value from id, data and crc values */
#define NVCNT_ELEMENT_VALUE(__ID__,__DATA__,__CRC__) (((NVCNT_ELEMENT_TYPE)(__DATA__) << NVCNT_DATA_SHIFT)\
                                                      | (__CRC__) << NVCNT_CRC_SHIFT | (__ID__))

/* Private variables ---------------------------------------------------------*/
static uint32_t NbWrittenElements = 0U;                  /*!< Nb of elements written in valid and active pages */
static uint32_t AddressNextWrite = 0U;                   /*!< Initialize write position just after page header */

/* Private function prototypes -----------------------------------------------*/
static SFU_ErrorStatus InitCounterArea(void);
static SFU_ErrorStatus SetCounter(NVCounterTypeDef CounterId, uint32_t Data);
static SFU_ErrorStatus ReadCounter(NVCounterTypeDef CounterId, NVCNT_DATA_TYPE *pData);
static SFU_ErrorStatus VerifyAreaFullyErased(uint32_t Address, uint32_t Size);
static SFU_ErrorStatus InitCounter(void);
static NVCounterTypeDef GetCounterFromImgId(uint32_t ImageId);
static inline SFU_ErrorStatus WriteElement(void *pDestination, NVCNT_ELEMENT_TYPE element);
static void ConfigureCrc(void);
static uint16_t CalculateCrc(NVCNT_DATA_TYPE Data, NVCNT_ID_TYPE CounterId);

/* Functions Definition ------------------------------------------------------*/
/**
  * @brief  Initialize nv counter storage area if not formatted.
  *         Check consistency of nv counter storage
  *         Initialize global variable for write access.
  * @param  none
  * @retval SFU_SUCCESS if successful, a SFU_ErrorStatus error otherwise.
  */
static SFU_ErrorStatus InitCounterArea(void)
{
  SFU_ErrorStatus e_ret_status = SFU_ERROR;
  uint32_t offset = 0U;
  NVCNT_ELEMENT_TYPE element = 0U;
  NVCNT_DATA_TYPE counter_value = 0U;
  NVCounterTypeDef counter_id = SLOT_ACTIVE_1_COUNTER;

  /*
   * Step 0: test if it is a full erased area
   * ========================================
   */
  e_ret_status = VerifyAreaFullyErased(SECURE_COUNTER_REGION_ROM_START, SECURE_COUNTER_REGION_ROM_SIZE);
  if (e_ret_status == SFU_SUCCESS)
  {
    /* area is fully erase :
       - no power down/reset is expected during this 1st initialization
       - incomplete initialization is detected in step 1, and SBSFU will not launch installed image */

    TRACE("\r\n\t  Initializing secure counter area : Power down/reset not supported...");

    /* initialize secure counters to INIT value */
    e_ret_status = InitCounter();
    if (e_ret_status != SFU_SUCCESS)
    {
#if defined(SFU_VERBOSE_DEBUG_MODE)
      TRACE("\r\n\t  Secure counters initialization failed");
#endif /* SFU_VERBOSE_DEBUG_MODE */
      return SFU_ERROR;
    }
    TRACE("\r\n\t  Secure counter area initialized : Power down/reset supported");
  }

  /*
   * Step 1: Checking Secure Counter consistency
   * ==========================================
   */
  while (counter_id < NV_COUNTER_MAX)
  {
    /* Checking Secure Counter consistency */
    e_ret_status = ReadCounter(counter_id, &counter_value);
    if (e_ret_status != SFU_SUCCESS)
    {
#if defined(SFU_VERBOSE_DEBUG_MODE)
      TRACE("\r\n\t  Secure counter %d not consistent", counter_id);
#endif /* SFU_VERBOSE_DEBUG_MODE */
      return SFU_ERROR;
    }
#if defined(SFU_VERBOSE_DEBUG_MODE)
    else
    {
      TRACE("\r\n\t  Consistent Secure Counter %d = 0x%x", counter_id, counter_value);
    }
#endif /* SFU_VERBOSE_DEBUG_MODE */
    counter_id++;
  }

  /*
   * Step 2: Initialize global variables
   * ===================================
   */
  /* Initialize global variables, with elements detected in active page */
  NbWrittenElements = 0U;
  AddressNextWrite = 0U;

  while (offset < SECURE_COUNTER_REGION_ROM_SIZE)
  {
    /* Read elements  */
    e_ret_status = SFU_LL_FLASH_Read((uint8_t *)&element, (uint8_t *)(SECURE_COUNTER_REGION_ROM_START + offset) , sizeof(element));

    if (e_ret_status != SFU_SUCCESS)
    {
      return SFU_ERROR;
    }
    /* written element ? */
    if (element != NVCNT_MASK_FULL)
    {
      /* Then increment NbWrittenElements and AddressNextWrite */
      NbWrittenElements++;
      AddressNextWrite += NVCNT_OFFSET_SIZE;
    }
    else /* no more written elements */
    {
      break;
    }
    offset += NVCNT_OFFSET_SIZE;
  }

  return e_ret_status;
}

/**
  * @brief  Writes/updates counter data in NV area.
  * @param  CounterId to be written
  * @param  Data 32bits data to be written
  * @retval SFU_SUCCESS if successful, a SFU_ErrorStatus error otherwise.
  */
static SFU_ErrorStatus SetCounter(NVCounterTypeDef CounterId, NVCNT_DATA_TYPE Data)
{
  SFU_ErrorStatus e_ret_status = SFU_ERROR;
  uint32_t crc = 0U;
  NVCNT_ELEMENT_TYPE element = 0U;
  NVCNT_DATA_TYPE current_value = 0U;

  /* check current value is not already in flash, and is consistent */
  e_ret_status = ReadCounter(CounterId, &current_value);
  if ((e_ret_status != SFU_SUCCESS) || (Data < current_value))
  {
    return SFU_ERROR;
  }

  /* check if same value is requested */
  if (current_value == Data)
  {
    /* nothing to do */
    return SFU_SUCCESS;
  }

  /* Check if pages are full, i.e. max number of written elements achieved */
  if (NbWrittenElements >= NVCNT_MAX_WRITTEN_ELEMENTS)
  {
    return SFU_ERROR;
  }

  /* Calculate crc of variable data and virtual address */
  crc = CalculateCrc(Data, CounterId);

  /*  build element  */
  element = NVCNT_ELEMENT_VALUE(CounterId, Data, crc);

  /* Program variable data + virtual address + crc */
  e_ret_status = WriteElement((void *)(SECURE_COUNTER_REGION_ROM_START + AddressNextWrite), element);

  /* Increment global variables relative to write operation done*/
  if (e_ret_status == SFU_SUCCESS)
  {
    AddressNextWrite += NVCNT_OFFSET_SIZE;
    NbWrittenElements++;
#if defined(SFU_VERBOSE_DEBUG_MODE)
    TRACE("\r\n\tCounter %d set to 0x%x", CounterId, Data);
#endif /* SFU_VERBOSE_DEBUG_MODE */
  }

  return e_ret_status;
}

/**
  * @brief  Read counter data from NV area.
  * @param  CounterId Counter Id to be read
  * @param  NVCNT_DATA_TYPE pointer to data to be filled
  * @retval SFU_SUCCESS if successful, a SFU_ErrorStatus error otherwise.
  */
static SFU_ErrorStatus ReadCounter(NVCounterTypeDef CounterId, NVCNT_DATA_TYPE *pData)
{
  SFU_ErrorStatus e_ret_status = SFU_ERROR;
  NVCNT_ELEMENT_TYPE element = 0U;
  uint32_t offset = 0U;
  uint32_t crc = 0U;
  uint32_t found = 0U;
  NVCNT_DATA_TYPE previous_value = 0U;
  NVCNT_DATA_TYPE current_value = 0U;

  /* Set offset to last element position in the page */
  offset = SECURE_COUNTER_REGION_ROM_SIZE;

  /* Check each element starting from end */
  while (offset >= NVCNT_OFFSET_SIZE)
  {
    /* Next address location */
    offset -= NVCNT_OFFSET_SIZE;

    /* Read element */
    e_ret_status = SFU_LL_FLASH_Read((uint8_t *)&element, (uint8_t *)(SECURE_COUNTER_REGION_ROM_START + offset) , sizeof(element));

    if ((e_ret_status == SFU_SUCCESS) && (element != NVCNT_PATTERN_ERASE))
    {
      /* Check a zero is not written, zero is forbidden, zero can be used to clean a value */
      if ((NVCNT_DATA_VALUE(element) == 0U) || (NVCNT_ID_VALUE(element) == 0U))
      {
        return SFU_ERROR;
      }

      /* Check counter Id */
      if (NVCNT_ID_VALUE(element) == CounterId)
      {
        /* Calculate CRC based on counter data and counter id */
        crc = CalculateCrc(NVCNT_DATA_VALUE(element), NVCNT_ID_VALUE(element));

        /* if CRC verification pass, data is correct and is returned.
           else, data is corrupted and has to be skip */
        if (crc == NVCNT_CRC_VALUE(element))
        {
          /* First time the counter is found */
          if (found != 1)
          {
            *pData = NVCNT_DATA_VALUE(element);
            previous_value = NVCNT_DATA_VALUE(element);
            found = 1;
          }
          /* Already found, verify historic :
             check that the counter value decreases  */
          else
          {
            current_value = NVCNT_DATA_VALUE(element);
            if (current_value >= previous_value)
            {
              /* Error reported */
              return SFU_ERROR;
            }
            else
            {
              previous_value = current_value;
            }
          }
        }
      }
    }
  }

  /* Counter found with consistent data ? */
  if (found == 1)
  {
    e_ret_status = SFU_SUCCESS;;
  }
  else
  {
    e_ret_status = SFU_ERROR;
  }

  return e_ret_status;
}


/**
  * @brief  Verify if specified area is fully erased.
  * @param  Address: area address
  * @param  Size: area size
  * @retval SFU_SUCCESS if successful, a SFU_ErrorStatus error otherwise.
  */
static SFU_ErrorStatus VerifyAreaFullyErased(uint32_t Address, uint32_t Size)
{
  SFU_ErrorStatus e_ret_status = SFU_ERROR;
  uint32_t offset = 0U;
  NVCNT_ELEMENT_TYPE element;

  /* Check each element in the area */
  while (offset < Size)
  {
    e_ret_status = SFU_LL_FLASH_Read((uint8_t *)&element, (uint8_t *)(Address + offset), NVCNT_ELEMENT_SIZE);

    /* Element erased ? */
    if ((e_ret_status != SFU_SUCCESS) || (element != NVCNT_PATTERN_ERASE))
    {
      break;
    }
    /* Next element */
    offset = offset + NVCNT_OFFSET_SIZE;
  }

  /* Verify complete parsing */
  if (offset >= Size)
  {
    e_ret_status = SFU_SUCCESS;
  }

  return e_ret_status;
}


/**
  * @brief  This function configures CRC Instance.
  * @note   This function is used to :
  *         -1- Enable peripheral clock for CRC.
  *         -2- Configure CRC functional parameters.
  * @note   Peripheral configuration is minimal configuration from reset values.
  *         Thus, some useless LL unitary functions calls below are provided as
  *         commented examples - setting is default configuration from reset.
  * @param  None
  * @retval None
  */
static void ConfigureCrc(void)
{
    /* STM32H750 Internal Flash already supports ECC.
     Up to the customer to add CRC if he wants to add additional robustness */
   SFU_LL_CRC_Config(SFU_CRC_CONFIG_16BIT);
}

/**
  * @brief  This function performs CRC calculation on Data and Virtual Address.
  * @param  Data value, part of CRC calculation.
  * @param  CounterId Counter Id, part of CRC calculation.
  * @retval 16-bit CRC value computed on Data and CounterId.
  */
static uint16_t CalculateCrc(NVCNT_DATA_TYPE Data, NVCNT_ID_TYPE CounterId)
{
  uint8_t buffer[NVCNT_ELEMENT_SIZE] __attribute__((aligned(4)));

  ConfigureCrc();

  memcpy(&buffer[0], &Data, sizeof(NVCNT_DATA_TYPE));
  memcpy(&buffer[sizeof(NVCNT_DATA_TYPE)], &CounterId, sizeof(NVCNT_ID_TYPE));

  return (uint16_t) SFU_LL_CRC_Calculate((uint32_t *)(uint32_t)&buffer[0], sizeof(NVCNT_DATA_TYPE) + sizeof(NVCNT_ID_TYPE));
}

/**
  * @brief  Write the element in Flash according to SFU_LL_FLASH_write_t
  * @param  pDestination: address pointer if flash to write the secure counter element .
  * @param  Element: secure counter element to write in flash.
  * @retval SFU_SUCCESS if successful, a SFU_ErrorStatus error otherwise.
  */
static SFU_ErrorStatus WriteElement(void *pDestination, NVCNT_ELEMENT_TYPE Element)
{
  uint32_t i = 0U;
  uint8_t buffer[NVCNT_OFFSET_SIZE];
  SFU_FLASH_StatusTypeDef flash_if_info;

  /* Set dimension to the appropriate length for FLASH programming.
   * Example: 64-bit length for L4.
   */
  memcpy(&buffer[0], (void*) &Element, NVCNT_ELEMENT_SIZE);
  if ((NVCNT_ELEMENT_SIZE % (uint32_t)sizeof(SFU_LL_FLASH_write_t)) != 0U)
  {
    i = NVCNT_ELEMENT_SIZE;
    while (i < NVCNT_OFFSET_SIZE)
    {
       buffer[i] = 0xFFU;
       i++;
    }
  }

  return SFU_LL_FLASH_Write(&flash_if_info, (void *)pDestination, (void *)&buffer[0], sizeof(buffer));
}

/**
  * @brief  Init counter data in NV area.
  * @param  None
  * @retval SFU_SUCCESS if successful, a SFU_ErrorStatus error otherwise.
  */
static SFU_ErrorStatus InitCounter(void)
{
  uint32_t crc = 0U;
  SFU_ErrorStatus e_ret_status = SFU_ERROR;
  NVCNT_ELEMENT_TYPE element = 0U;
  NVCounterTypeDef counter_id = SLOT_ACTIVE_1_COUNTER;
  const uint32_t data = NVCNT_INIT_VALUE;

  /* initialize each counter ID */
  while (counter_id < NV_COUNTER_MAX)
  {
    /* Check if area is full, i.e. max number of written elements achieved */
    if (NbWrittenElements >= NVCNT_MAX_WRITTEN_ELEMENTS)
    {
      return SFU_ERROR;
    }

    /* Calculate crc of variable data and virtual address */
    crc = CalculateCrc(data, counter_id);

    /*  build element  */
    element = NVCNT_ELEMENT_VALUE(counter_id, data, crc);

    /* Program variable data + virtual address + crc */
    e_ret_status = WriteElement((void *)(SECURE_COUNTER_REGION_ROM_START + AddressNextWrite), element);

    /* If program operation was failed, a Flash error code is returned */
    if (e_ret_status != SFU_SUCCESS)
    {
      return SFU_ERROR;
    }

    /* Increment global variables relative to write operation done*/
    AddressNextWrite += NVCNT_OFFSET_SIZE;
    NbWrittenElements++;
    counter_id++;
  }

  /* Verify all counter initialized */
  if (counter_id >= NV_COUNTER_MAX)
  {
    e_ret_status = SFU_SUCCESS;
  }

  return e_ret_status;
}

/**
  * @brief  Return the counter id from the ImageId.
  * @param  ImageId: must be 0 for the active image counter.
  *         Additional id can be supported extending tfm_nv_counter_t.
  * @note   None
  * @retval The secure counter id corresponding to the ImageId.
  */
static NVCounterTypeDef GetCounterFromImgId(uint32_t ImageId)
{
  uint32_t nv_counter = NV_COUNTER_MAX;

  /* Avoid integer overflow */
  if (ImageId < SLOT_ACTIVE_1)
  {
    return NV_COUNTER_MAX;
  }

  nv_counter = SLOT_ACTIVE_1_COUNTER + ImageId - SLOT_ACTIVE_1;

  /* Check the existence of the enumerated counter value */
  if (nv_counter >= NV_COUNTER_MAX)
  {
    return NV_COUNTER_MAX;
  }

  return (NVCounterTypeDef)nv_counter;
}

/**
  * @brief  Initialize the secure counter and the related area in flash.
  * @param  none
  * @note   None
  * @retval SFU_SUCCESS if successful, a SFU_ErrorStatus error otherwise.
  */
SFU_ErrorStatus SFU_IMG_SecureCounterInit()
{
  SFU_ErrorStatus e_ret_status = SFU_ERROR;

  e_ret_status = InitCounterArea();

  return e_ret_status;
}

/**
  * @brief  Read the last stored secure counter.
  * @param  ImageId: must be 0 for the active image counter.
  *         Additional id support can be added on tfm_nv_counter_t.
  *         Single active image id (0) is currently supported.
  * @param  pImgSecureCounter: pointer to the counter variable to be filled
  * @note   None
  * @retval SFU_SUCCESS if successful, a SFU_ErrorStatus error otherwise.
  */
SFU_ErrorStatus SFU_IMG_SecureCounterRead(uint32_t ImageId, uint32_t *pImgSecureCounter)
{
  NVCounterTypeDef nv_counter;
  SFU_ErrorStatus e_ret_status = SFU_ERROR;

  /* Check if it's a null-pointer. */
  if (pImgSecureCounter == 0U)
  {
    return SFU_ERROR;
  }

  /* Retrieve counter Id from Image Id */
  nv_counter = GetCounterFromImgId(ImageId);
  if (nv_counter == NV_COUNTER_MAX)
  {
    return SFU_ERROR;
  }

  /* Read counter Id */
  e_ret_status = ReadCounter(nv_counter, pImgSecureCounter);

  return e_ret_status;
}

/**
  * @brief  Write a new secure counter.
  * @param  ImageId: must be 0 for active image counter
  * @param  ImgSecureCounter: version counter value to be written
  * @note   None
  * @retval SFU_SUCCESS if successful, a SFU_ErrorStatus error otherwise.
  */
SFU_ErrorStatus SFU_IMG_SecureCounterWrite(uint32_t ImageId, uint32_t ImgSecureCounter)
{
  NVCounterTypeDef nv_counter;
  SFU_ErrorStatus e_ret_status = SFU_ERROR;

  /* Retrieve counter Id from Image Id */
  nv_counter = GetCounterFromImgId(ImageId);
  if (nv_counter == NV_COUNTER_MAX)
  {
    return SFU_ERROR;
  }

  /* Write counter Id */
  e_ret_status = SetCounter(nv_counter, ImgSecureCounter);

  return e_ret_status;
}


#undef SFU_FWIMG_COUNTER_C
