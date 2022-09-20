/**
  ******************************************************************************
  * @file    fw_update_app.c
  * @author  MCD Application Team
  * @brief   Firmware Update module.
  *          This file provides set of firmware functions to manage Firmware
  *          Update functionalities.
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

#include "se_def.h"
#include "com.h"
#include "common.h"
#include "flash_if.h"
#include "stm32h7xx_hal.h"
#include "stm32h750b_discovery.h"
#include "fw_update_app.h"
#include "se_interface_application.h"
#include "sfu_fwimg_regions.h"
#include "sfu_standalone_loader.h"
#include "sfu_app_new_image.h"
#include "string.h"
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
#include "mapping_fwimg.h"
#include "mapping_sbsfu.h"
#elif defined (__ICCARM__) || defined(__GNUC__)
#include "mapping_export.h"
#endif /* __CC_ARM || __ARMCC_VERSION */
#include "ymodem.h"

/* Private defines -----------------------------------------------------------*/

#define EXTERNAL_LOADER                /* In this example, external loader is activated in the default configuration */
#define ALL_SLOTS 255U                   /*!< Validate all new firmware installed in a single request */
/* Global variables ----------------------------------------------------------*/

#if !defined(EXTERNAL_LOADER)
static uint32_t m_uFileSizeYmodem = 0U;  /*!< Ymodem File size*/
static uint32_t m_uNbrBlocksYmodem = 0U; /*!< Ymodem Number of blocks*/
static uint32_t m_uPacketsReceived = 0U; /*!< Ymodem packets received*/

#ifdef MINICOM_YMODEM
/*
 * With Minicom Ymodem, the data offset is not aligned because PACKET_DATA_INDEX is 7
 * So we cannot write in FLASH with pData as source, we need a copy in an aligned buffer
 */
/* With Minicom YMODEM each packet carries 128 bytes of data */
static uint8_t alignedBuffer[PACKET_SIZE] __attribute__((aligned(8)));
#endif /* MINICOM_YMODEM */
#endif /* !EXTERNAL_LOADER */

/* Private function prototypes -----------------------------------------------*/

#if !defined(EXTERNAL_LOADER)
static HAL_StatusTypeDef FW_UPDATE_DownloadNewFirmware(SFU_FwImageFlashTypeDef *pFwImageDwlArea);
#endif /* !EXTERNAL_LOADER */

/* Functions Definition ------------------------------------------------------*/

/**
  * @brief  Run FW Update process.
  * @param  None
  * @retval HAL Status.
  */
#if defined(EXTERNAL_LOADER)
void FW_UPDATE_Run(void)
{
  uint32_t source, source_end;
  typedef  void (*pFunction)(void);
  pFunction JumpToApplication;
  uint32_t *dest;

  printf("\r\n============== Move Loader to SRAM =======================\r\n");

  /* Disable Systick interrupt */
  SysTick->CTRL = 0;

  /* Copy App Loader code to SRAM */
  source = (uint32_t)APP_LOADER_REGION_ROM_START;
  source_end = (uint32_t)APP_LOADER_REGION_ROM_END;
  dest = (uint32_t *)APP_LOADER_REGION_RAM_CODE_START;
  while (source < source_end)
  {
      *dest = *(__IO uint32_t *)source;
      dest++;
      source += 4U;
  }

  /* Print Firmware Update welcome message */
  printf("\r\n================ New Fw Download =========================\r\n\n");

  /* Initialize user application's Stack Pointer & Jump to user application */
  JumpToApplication = (pFunction) (*(__IO uint32_t*) (APP_LOADER_REGION_RAM_CODE_START + 4));
  __set_MSP(*(__IO uint32_t*) APP_LOADER_REGION_RAM_CODE_START);
  JumpToApplication();

  /* We should never get here as execution is now on loader moved to SRAM */
  while(1)
  {
  }
}

#else
void FW_UPDATE_Run(void)
{
  HAL_StatusTypeDef ret = HAL_ERROR;
  uint8_t  fw_header_dwl_slot[SE_FW_HEADER_TOT_LEN];
  SFU_FwImageFlashTypeDef fw_image_dwl_area;

  /* Print Firmware Update welcome message */
  printf("\r\n================ New Fw Download =========================\r\n\n");

  /* Get Info about the download area */
  SFU_APP_GetDownloadAreaInfo(SLOT_DWL_1, &fw_image_dwl_area);

  /* Download new firmware image*/
  ret = FW_UPDATE_DownloadNewFirmware(&fw_image_dwl_area);

  if (HAL_OK == ret)
  {
    /* Read header in dwl slot */
    (void)FLASH_If_Read(fw_header_dwl_slot, (void *) fw_image_dwl_area.DownloadAddr, SE_FW_HEADER_TOT_LEN);

    /* Ask for installation at next reset */
    (void)SFU_APP_InstallAtNextReset((uint8_t *) fw_header_dwl_slot);

    /* System Reboot*/
    printf("  -- Image correctly downloaded - reboot\r\n\n");
    HAL_Delay(1000U);
    NVIC_SystemReset();
  }

  if (ret != HAL_OK)
  {
    printf("  -- !!Operation failed!! \r\n\n");
  }
}
#endif /* EXTERNAL_LOADER */


#if defined(EXTERNAL_LOADER)
/**
  * @brief  Run Multiple FW Update process.
  * @param  None
  * @retval None.
  */
void FW_UPDATE_MULTIPLE_RunMenu(void)
{
  printf("  Feature not supported ! \r\n");
}

#else
/**
  * @brief  Print Multiple FW Update menu.
  * @param  None
  * @retval HAL Status.
  */
static void FW_UPDATE_MULTIPLE_PrintMenu(void)
{
  /* Print Multiple Download menu */
  printf("\r\n============= Multiple Download Menu ================\r\n\n");
  printf("  Download firmware in DWL_SLOT #1 ---------------------- 1\r\n\n");
  printf("  Download firmware in DWL_SLOT #2 ---------------------- 2\r\n\n");
  printf("  Download firmware in DWL_SLOT #3 ---------------------- 3\r\n\n");
  printf("  Start installation ------------------------------------ 4\r\n\n");
  printf("  Previous Menu ----------------------------------------- x\r\n\n");
  printf("  Selection :\r\n\n");
}

/**
  * @brief  Run Multiple FW Update process.
  * @param  None
  * @retval None.
  */
void FW_UPDATE_MULTIPLE_RunMenu(void)
{
  SFU_FwImageFlashTypeDef fw_image_dwl_area;
  uint8_t key = 0U;
  uint32_t exit = 0U;
  uint32_t i = 0U;
  uint32_t several_dwl_area = 1U;
  uint32_t slot_number = 0U;
  static  uint8_t fw_to_install = 0U;
  uint8_t  fw_header_dwl_slot[SE_FW_HEADER_TOT_LEN];

  /* Print Menu */
  FW_UPDATE_MULTIPLE_PrintMenu();

  /* Verify if at least 2 DWL areas exist */
  for (i = 1U; i < SFU_NB_MAX_DWL_AREA; i++)
  {
    if (SlotStartAdd[SLOT_DWL_1 + i] != 0U)
    {
      several_dwl_area++;
    }
  }

  if (several_dwl_area == 1)
  {
    printf("  -- !!Only 1 download area configured - feature not available!! \r\n\n");
    return;
  }

  while (exit == 0U)
  {
    key = 0U;
    slot_number = 0U;

    /* If the SecureBoot configured the IWDG, UserApp must reload IWDG counter with value defined in the
       reload register */
    WRITE_REG(IWDG1->KR, IWDG_KEY_RELOAD);

    /* Clean the input path */
    COM_Flush();

    /* Receive key */
    if (COM_Receive(&key, 1U, RX_TIMEOUT) == HAL_OK)
    {
      switch (key)
      {
        case '1' :
          slot_number = SLOT_DWL_1 + 0U;
          break;
        case '2' :
          slot_number = SLOT_DWL_1 + 1U;
          break;
        case '3' :
          slot_number = SLOT_DWL_1 + 2U;
          break;
        case '4' :
          /*
           * When IMAGE_STATE_HANDLING switch is not activated :
           *  - requesting installation means writing the header in the SWAP slot then reset.
           *  - else, reset is enough. Image state set as NEW in the header of dwl_slot will identify the dwl_slot
           *    to install.
           */
          /* Get Info about the download area */
          SFU_APP_GetDownloadAreaInfo(fw_to_install, &fw_image_dwl_area);

          /* Read header in dwl slot */
          FLASH_If_Read(fw_header_dwl_slot, (void *) fw_image_dwl_area.DownloadAddr, SE_FW_HEADER_TOT_LEN);

          /* Ask for installation at next reset */
          (void)SFU_APP_InstallAtNextReset((uint8_t *) fw_header_dwl_slot);

          /* System Reboot*/
          printf("  -- Installation requested - Reboot");
          HAL_Delay(1000U);
          NVIC_SystemReset();
          break;
        case 'x' :
          exit = 1U;
          break;

        default:
          printf("Invalid Number !\r");
          break;
      }

      if (slot_number != 0U)
      {
        /* Get Info about the download area */
        SFU_APP_GetDownloadAreaInfo(slot_number, &fw_image_dwl_area);

        if ((fw_image_dwl_area.DownloadAddr != 0U) && (fw_image_dwl_area.MaxSizeInBytes != 0U))
        {
          /* Download new firmware image*/
          if (FW_UPDATE_DownloadNewFirmware(&fw_image_dwl_area) == HAL_OK)
          {
            fw_to_install = slot_number;
          }
          else
          {
            printf("  -- !!Download failed!! \r\n\n");
            HAL_Delay(1000U);
          }
        }
        else
        {
          printf("  -- !!Download slot not configured!! \r\n\n");
          HAL_Delay(1000U);
        }
      }

      /* Print Menu */
      FW_UPDATE_MULTIPLE_PrintMenu();
    }
  }
}
#endif /* EXTERNAL_LOADER */

#if !defined(EXTERNAL_LOADER)
/**
  * @brief Download a new Firmware from the host.
  * @retval HAL status
  */
static HAL_StatusTypeDef FW_UPDATE_DownloadNewFirmware(SFU_FwImageFlashTypeDef *pFwImageDwlArea)
{
  HAL_StatusTypeDef ret = HAL_ERROR;
  COM_StatusTypeDef e_result;
  uint32_t u_fw_size;
  YMODEM_CallbacksTypeDef ymodemCb = {Ymodem_HeaderPktRxCpltCallback, Ymodem_DataPktRxCpltCallback};

  printf("  -- Send Firmware \r\n\n");
  /* Refresh Watchdog */
  WRITE_REG(IWDG1->KR, IWDG_KEY_RELOAD);

  /* Clear download area */
  printf("  -- -- Erasing download area ...\r\n\n");
  if ((ret = FLASH_If_Erase_Size((void *)(pFwImageDwlArea->DownloadAddr), pFwImageDwlArea->MaxSizeInBytes)) == HAL_OK)
  {
    /* Download binary */
    printf("  -- -- File> Transfer> YMODEM> Send ");

    /*Init of Ymodem*/
    Ymodem_Init();

    /*Receive through Ymodem*/
    e_result = Ymodem_Receive(&u_fw_size, pFwImageDwlArea->DownloadAddr, &ymodemCb);
    printf("\r\n\n");

    if ((e_result == COM_OK))
    {
      printf("  -- -- Programming Completed Successfully!\r\n\n");
      printf("  -- -- Bytes: %d\r\n\n", u_fw_size);
      ret = HAL_OK;
    }
    else if (e_result == COM_ABORT)
    {
      printf("  -- -- !!Aborted by user!!\r\n\n");
      ret = HAL_ERROR;
    }
    else
    {
      printf("  -- -- !!Error during file download!!\r\n\n");
      ret = HAL_ERROR;
    }
  }
  return ret;
}
#endif /* !(EXTERNAL_LOADER) */

/**
  * @brief  Run validation of a FW image menu.
  * @param  None
  * @retval None.
  */
void FW_VALIDATE_RunMenu(void)
{
  printf("  Feature not supported ! \r\n");
}




#if !defined(EXTERNAL_LOADER)
/**
  * @brief  Ymodem Header Packet Transfer completed callback.
  * @param  uFileSize Dimension of the file that will be received (Bytes).
  * @retval None
  */
HAL_StatusTypeDef Ymodem_HeaderPktRxCpltCallback(uint32_t uFileSize)
{
  /*Reset of the ymodem variables */
  m_uFileSizeYmodem = 0U;
  m_uPacketsReceived = 0U;
  m_uNbrBlocksYmodem = 0U;

  /*Filesize information is stored*/
  m_uFileSizeYmodem = uFileSize;

  /*Compute the number of blocks */
#ifndef MINICOM_YMODEM
  /* Teraterm sends 1kB YMODEM packets */
  m_uNbrBlocksYmodem = (m_uFileSizeYmodem + (PACKET_1K_SIZE - 1U)) / PACKET_1K_SIZE;
#else
  /* Minicom sends 128 bytes YMODEM packets */
  m_uNbrBlocksYmodem = (m_uFileSizeYmodem + (PACKET_SIZE - 1U)) / PACKET_SIZE;
#endif /* MINICOM_YMODEM */

  /* NOTE : delay inserted for Ymodem protocol*/
  HAL_Delay(1000U);

  return HAL_OK;
}

/**
  * @brief  Ymodem Data Packet Transfer completed callback.
  * @param  pData Pointer to the buffer.
  * @param  uSize Packet dimension (Bytes).
  * @retval None
  */
#ifndef MINICOM_YMODEM
/* Teraterm YMODEM */
HAL_StatusTypeDef Ymodem_DataPktRxCpltCallback(uint8_t *pData, uint32_t uFlashDestination, uint32_t uSize)
{
  /* Start address of downloaded area : initialized with first packet (header) */
  static uint32_t m_uDwlImgStart = 0U;

  /* End address of downloaded Image : initialized with first packet (header) and checked along download process */
  static uint32_t m_uDwlImgEnd = 0U;

  /* Current destination address for data packet : initialized with first packet (header), incremented at each
     flash write */
  static uint32_t m_uDwlImgCurrent = 0U;

  HAL_StatusTypeDef e_ret_status = HAL_OK;
  uint32_t uLength;
  uint32_t uOldSize;
#if !defined(SFU_NO_SWAP)
  SE_FwRawHeaderTypeDef fw_header_dwl;
#endif /* (SFU_NO_SWAP) */

  m_uPacketsReceived++;

  /*Increase the number of received packets*/
  if (m_uPacketsReceived == m_uNbrBlocksYmodem) /*Last Packet*/
  {
    /*Extracting actual payload from last packet*/
    if (0 == (m_uFileSizeYmodem % PACKET_1K_SIZE))
    {
      /* The last packet must be fully considered */
      uSize = PACKET_1K_SIZE;
    }
    else
    {
      /* The last packet is not full, drop the extra bytes */
      uSize = m_uFileSizeYmodem - ((uint32_t)(m_uFileSizeYmodem / PACKET_1K_SIZE) * PACKET_1K_SIZE);
    }
  }

  /* First packet : Contains header information: PartialFwSize and PartialFwOffset information */
  if (m_uPacketsReceived == 1)
  {
    /* Start of Download area */
    m_uDwlImgStart = uFlashDestination;

    /* Initialize Current destination address for data packet */
    m_uDwlImgCurrent = uFlashDestination;

    /* End of Image to be downloaded */
#if defined(SFU_NO_SWAP)
    m_uDwlImgEnd = uFlashDestination + ((SE_FwRawHeaderTypeDef *)pData)->FwSize + SFU_IMG_IMAGE_OFFSET;
#else
    m_uDwlImgEnd = uFlashDestination + ((SE_FwRawHeaderTypeDef *)pData)->PartialFwSize
                   + (((SE_FwRawHeaderTypeDef *)pData)->PartialFwOffset % SLOT_SIZE(SLOT_SWAP))
                   + SFU_IMG_IMAGE_OFFSET;
#endif /* SFU_NO_SWAP */
  }

  /* This packet : contains end of FW header */
  if ((m_uDwlImgCurrent < (m_uDwlImgStart + SFU_IMG_IMAGE_OFFSET)) &&
      ((m_uDwlImgCurrent + uSize) >= (m_uDwlImgStart + SFU_IMG_IMAGE_OFFSET)))
  {
    /* Write the FW header part (SFU_IMG_IMAGE_OFFSET % PACKET_1K_SIZE bytes length) */
    uLength = SFU_IMG_IMAGE_OFFSET % PACKET_1K_SIZE;
    if (uLength == 0)
    {
      uLength = PACKET_1K_SIZE;
    }
    if (FLASH_If_Write((void *)m_uDwlImgCurrent, pData, uLength) == HAL_OK)
    {
#if defined(SFU_NO_SWAP)
      m_uDwlImgCurrent += uLength;
#else
      /*
       * Read header from dwl area : in some configuration header can be transmitted with 2 YMODEM packets
       * ==> pData contains only the last part of the header and cannot be used to retrieve PartialFwOffset
       */
      e_ret_status = FLASH_If_Read((uint8_t *)&fw_header_dwl, (void *) m_uDwlImgStart, SE_FW_HEADER_TOT_LEN);

      /* Shift the DWL area pointer, to align image with (PartialFwOffset % sector size) in DWL area */
      m_uDwlImgCurrent += uLength + fw_header_dwl.PartialFwOffset % SLOT_SIZE(SLOT_SWAP);
#endif /* SFU_NO_SWAP */

      /* Update remaining packet size to write */
      uSize -= uLength;

      /* Update pData pointer to received packet data */
      pData += uLength;
    }
    else
    {
      e_ret_status = HAL_ERROR;
    }
  }

  /* Skip data write if all has been already written as part of the header */
  if (uSize != 0U)
  {
    /*Adjust dimension to unitary flash programming length */
    if (uSize % FLASH_IF_MIN_WRITE_LEN != 0U)
    {
      uOldSize = uSize;
      uSize += (FLASH_IF_MIN_WRITE_LEN - (uSize % FLASH_IF_MIN_WRITE_LEN));
      m_uDwlImgEnd += (FLASH_IF_MIN_WRITE_LEN - (uSize % FLASH_IF_MIN_WRITE_LEN));
      while (uOldSize < uSize)
      {
        pData[uOldSize] = 0xFF;
        uOldSize++;
      }
    }

    /* Write Data in Flash - size has to be 256-bit aligned */

    /* Write in flash only if not beyond allowed area */
    if (((m_uDwlImgCurrent + uSize) <= m_uDwlImgEnd) && (e_ret_status == HAL_OK))
    {
      if (FLASH_If_Write((void *)m_uDwlImgCurrent, pData, uSize) == HAL_OK)
      {
        e_ret_status = HAL_OK;

        /* Move Current destination address for next packet */
        m_uDwlImgCurrent += uSize;
      }
      else
      {
        e_ret_status = HAL_ERROR;
      }
    }
    else
    {
      e_ret_status = HAL_ERROR;
    }
  }

  /* Last packet : reset m_uPacketsReceived */
  if (m_uPacketsReceived == m_uNbrBlocksYmodem)
  {
    m_uPacketsReceived = 0U;
  }

  /* Reset data counters in case of error */
  if (e_ret_status == HAL_ERROR)
  {

    /*Reset of the ymodem variables */
    m_uFileSizeYmodem = 0U;
    m_uPacketsReceived = 0U;
    m_uNbrBlocksYmodem = 0U;
  }
  return e_ret_status;
}
#else
/* Minicom YMODEM */
HAL_StatusTypeDef Ymodem_DataPktRxCpltCallback(uint8_t *pData, uint32_t uFlashDestination, uint32_t uSize)
{
  /* Start address of downloaded area : initialized with first packet (header) */
  static uint32_t m_uDwlImgStart = 0U;

  /* End address of downloaded Image : initialized with first packet (header) and checked along download process */
  static uint32_t m_uDwlImgEnd = 0U;

  /* Current destination address for data packet : initialized with first packet (header), incremented at each
     flash write */
  static uint32_t m_uDwlImgCurrent = 0U;

  HAL_StatusTypeDef e_ret_status = HAL_OK;
  uint32_t uLength;
  uint32_t uOldSize;
  SE_FwRawHeaderTypeDef fw_header_dwl;

  m_uPacketsReceived++;

  /*Increase the number of received packets*/
  if (m_uPacketsReceived == m_uNbrBlocksYmodem) /*Last Packet*/
  {
    /*Extracting actual payload from last packet*/
    if (0 == (m_uFileSizeYmodem % PACKET_SIZE))
    {
      /* The last packet must be fully considered */
      uSize = PACKET_SIZE;
    }
    else
    {
      /* The last packet is not full, drop the extra bytes */
      uSize = m_uFileSizeYmodem - ((uint32_t)(m_uFileSizeYmodem / PACKET_SIZE) * PACKET_SIZE);
    }
  }

  /* First packet : Contains header information: PartialFwSize and PartialFwOffset information */
  if (m_uPacketsReceived == 1)
  {
    /* Start of Download area */
    m_uDwlImgStart = uFlashDestination;

    /* Initialize Current destination address for data packet */
    m_uDwlImgCurrent = uFlashDestination;

    /* End of Image to be downloaded */
#if defined(SFU_NO_SWAP)
    m_uDwlImgEnd = uFlashDestination + ((SE_FwRawHeaderTypeDef *)pData)->FwSize + SFU_IMG_IMAGE_OFFSET;
#else
    m_uDwlImgEnd = uFlashDestination + ((SE_FwRawHeaderTypeDef *)pData)->PartialFwSize
                   + (((SE_FwRawHeaderTypeDef *)pData)->PartialFwOffset % SLOT_SIZE(SLOT_SWAP))
                   + SFU_IMG_IMAGE_OFFSET;
#endif /* SFU_NO_SWAP */

  }

  /* This packet : contains end of FW header */
  if ((m_uDwlImgCurrent < (m_uDwlImgStart + SFU_IMG_IMAGE_OFFSET)) &&
      ((m_uDwlImgCurrent + uSize) >= (m_uDwlImgStart + SFU_IMG_IMAGE_OFFSET)))
  {
    /* Write the FW header part (SFU_IMG_IMAGE_OFFSET % PACKET_1K_SIZE bytes length) */
    uLength = SFU_IMG_IMAGE_OFFSET % PACKET_SIZE;
    if (uLength == 0)
    {
      uLength = PACKET_SIZE;
    }

    memcpy(alignedBuffer, pData, uLength);

    if (FLASH_If_Write((void *)m_uDwlImgCurrent, alignedBuffer, uLength) == HAL_OK)
    {
#if defined(SFU_NO_SWAP)
      m_uDwlImgCurrent += uLength;
#else
      /*
       * Read header from dwl area : in some configuration header can be transmitted with 2 YMODEM packets
       * ==> pData contains only the last part of the header and cannot be used to retrieve PartialFwOffset
       */
      e_ret_status = FLASH_If_Read((uint8_t *)&fw_header_dwl, (void *) m_uDwlImgStart, SE_FW_HEADER_TOT_LEN);
      /* Shift the DWL area pointer, to align image with (PartialFwOffset % sector size) in DWL area */
      m_uDwlImgCurrent += uLength + fw_header_dwl.PartialFwOffset % SLOT_SIZE(SLOT_SWAP);
#endif /* SFU_NO_SWAP */

      /* Update remaining packet size to write */
      uSize -= uLength;

      /* Update pData pointer to received packet data */
      pData += uLength;
    }
    else
    {
      e_ret_status = HAL_ERROR;
    }
  }
  /* Skip data write if all has been already written as part of the header */
  if (uSize != 0U)
  {
    /*Adjust dimension to unitary flash programming length */
    if (uSize % FLASH_IF_MIN_WRITE_LEN != 0U)
    {
      uOldSize = uSize;
      uSize += (FLASH_IF_MIN_WRITE_LEN - (uSize % FLASH_IF_MIN_WRITE_LEN));
      m_uDwlImgEnd += (FLASH_IF_MIN_WRITE_LEN - (uSize % FLASH_IF_MIN_WRITE_LEN));
      while (uOldSize < uSize)
      {
        pData[uOldSize] = 0xFF;
        uOldSize++;
      }
    }

    /* Write Data in Flash - size has to be 256-bit aligned */


    /* Write in flash only if not beyond allowed area */
    if (((m_uDwlImgCurrent + uSize) <= m_uDwlImgEnd) && (e_ret_status == HAL_OK))
    {
      memcpy(alignedBuffer, pData, uSize);

      if (FLASH_If_Write((void *)m_uDwlImgCurrent, alignedBuffer, uSize) == HAL_OK)
      {
        e_ret_status = HAL_OK;

        /* Move Current destination address for next packet */
        m_uDwlImgCurrent += uSize;
      }
      else
      {
        e_ret_status = HAL_ERROR;
      }
    }
    else
    {
      e_ret_status = HAL_ERROR;
    }
  }

  /* Last packet : reset m_uPacketsReceived */
  if (m_uPacketsReceived == m_uNbrBlocksYmodem)
  {
    m_uPacketsReceived = 0U;
  }

  /* Reset data counters in case of error */
  if (e_ret_status == HAL_ERROR)
  {

    /*Reset of the ymodem variables */
    m_uFileSizeYmodem = 0U;
    m_uPacketsReceived = 0U;
    m_uNbrBlocksYmodem = 0U;
  }
  return e_ret_status;
}
#endif /* MINICOM_YMODEM */
#endif /* !(EXTERNAL_LOADER) */
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
