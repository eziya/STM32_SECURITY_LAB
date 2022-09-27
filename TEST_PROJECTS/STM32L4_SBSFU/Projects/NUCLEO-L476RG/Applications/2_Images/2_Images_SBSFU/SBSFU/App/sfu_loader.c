/**
  ******************************************************************************
  * @file    sfu_loader.c
  * @author  MCD Application Team
  * @brief   Secure Firmware Update LOADER module.
  *          This file provides set of firmware functions to manage SFU local
  *          loader functionalities.
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
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "sfu_loader.h"
#include "sfu_low_level_flash_int.h"
#include "sfu_low_level_security.h"
#include "sfu_com_loader.h"
#include "sfu_trace.h"
#include "se_interface_bootloader.h" /* for metadata authentication */
#include "sfu_fwimg_services.h"      /* for version checking & to check if a valid FW is installed (the local
                                        bootloader is a kind of "application" running in SB_SFU) */
#include "app_sfu.h"

#if (SECBOOT_LOADER == SECBOOT_USE_LOCAL_LOADER)

/* Private variables ---------------------------------------------------------*/
static uint32_t m_uDwlAreaAddress = 0U;                          /*!< Address of to write in download area */
static uint32_t m_uDwlAreaStart = 0U;                            /*!< Address of download area */
static uint32_t m_uDwlAreaSize = 0U;                             /*!< Size of download area */
static uint32_t m_uFileSizeYmodem = 0U;                          /*!< Ymodem file size being received */
static uint32_t m_uNbrBlocksYmodem = 0U;                         /*!< Number of blocks being received via Ymodem*/
static uint32_t m_uPacketsReceived = 0U;                         /*!< Number of packets received via Ymodem*/
static uint32_t m_uDwlSlot = SLOT_INACTIVE;                      /*!< Dwl slot identification : to be intialialized after header check */
static SFU_LOADER_StatusTypeDef m_LoaderStatus;                  /*!< Status of the loader */

/* Private function prototypes -----------------------------------------------*/
static SFU_ErrorStatus SFU_LOADER_VerifyFwHeader(uint8_t *pBuffer);

/* Functions Definition ------------------------------------------------------*/
/**
  * @brief  Initialize the SFU LOADER.
  * @param  None
  * @retval SFU_ErrorStatus SFU_SUCCESS if successful, SFU_ERROR otherwise.
  */
SFU_ErrorStatus SFU_LOADER_Init(void)
{
  /*
  * ADD SRC CODE HERE
  */

  /*
   * Sanity check to make sure that the local loader cannot read out of the buffer bounds
   * when doing a length alignment before writing in FLASH.
   */

  /* m_aPacketData contains SFU_COM_YMODEM_PACKET_1K_SIZE bytes of payload */
  if (0U != (uint32_t)(SFU_COM_YMODEM_PACKET_1K_SIZE % (uint32_t)sizeof(SFU_LL_FLASH_write_t)))
  {
    /* The packet buffer (payload part) must be a multiple of the FLASH write length  */
    TRACE("\r\n= [FWIMG] Packet Payload size (%d) is not matching the FLASH constraints",
          SFU_COM_YMODEM_PACKET_1K_SIZE);
    return SFU_ERROR;
  } /* else the FW Header Length is fine with regards to FLASH constraints */

  return SFU_SUCCESS;
}

/**
  * @brief  DeInitialize the SFU LOADER.
  * @param  None
  * @retval SFU_ErrorStatus SFU_SUCCESS if successful, SFU_ERROR otherwise.
  */
SFU_ErrorStatus SFU_LOADER_DeInit(void)
{
  /*
   * ADD SRC CODE HERE
   */
  return SFU_SUCCESS;
}

/**
  * @brief  Download a new User Fw.
  *         Writes firmware received via Ymodem in FLASH.
  * @param  peSFU_LOADER_Status: SFU LOADER Status.
  *         This parameter can be a value of @ref SFU_LOADER_Status_Structure_definition.
  * @param  p_DwlSlot identification of the downloaded area
  * @param  p_Size Size of the downloaded image.
  * @retval SFU_ErrorStatus SFU_SUCCESS if successful, SFU_ERROR otherwise.
  */
SFU_ErrorStatus SFU_LOADER_DownloadNewUserFw(SFU_LOADER_StatusTypeDef *peSFU_LOADER_Status, uint32_t *pDwlSlot,
                                             uint32_t *pSize)
{
  SFU_ErrorStatus e_ret_status = SFU_ERROR;
  SFU_COM_YMODEM_StatusTypeDef e_com_status = SFU_COM_YMODEM_ERROR;

  /* Check the pointers allocation */
  if ((peSFU_LOADER_Status == NULL) || (pSize == NULL) || (pDwlSlot == NULL))
  {
    return SFU_ERROR;
  }

  /* Refresh Watchdog */
  (void) SFU_LL_SECU_IWDG_Refresh();

  /* Transfer FW Image via YMODEM protocol */
  TRACE("\r\n\t  File> Transfer> YMODEM> Send ");

  /* Initialize global variables to be used during the YMODEM process */
  m_uDwlAreaAddress = 0U;
  m_uDwlAreaStart = 0U;
  m_uDwlAreaSize = 0U;
  m_uFileSizeYmodem = 0U;
  m_uNbrBlocksYmodem = 0U;
  m_uPacketsReceived = 0U;
  m_uDwlSlot = SLOT_DWL_1;
  m_LoaderStatus = SFU_LOADER_ERR_COM;

  /* Receive the FW in RAM and write it in the Flash*/
  // YModem 방식으로 UART 를 통해서 fw 를 다운로드 받는다.
  if (SFU_COM_YMODEM_Receive(&e_com_status, pSize) == SFU_SUCCESS)
  {
    if (*pSize > 0U)
    {
      /* File download correct*/
      m_LoaderStatus = SFU_LOADER_OK;
    }
  }
  else
  {
    /* Nothing to do : m_LoaderStatus updated during during YModem process */
  }

  /* Provide slot information to the caller
     m_uDwlSlot updated during VerifyFwHeader() except in case of bad formatted file
     ==> m_uDwlSlot stick at SLOT_DWL_1 value if the header signature is not verified */
  //다운로드가 완료되면 어느 다운로드 슬롯에 다운로드를 받았는지 반환한다.
  *pDwlSlot = m_uDwlSlot;


  *peSFU_LOADER_Status = m_LoaderStatus;
  if (m_LoaderStatus == SFU_LOADER_OK)
  {
    e_ret_status = SFU_SUCCESS;
  }
  return e_ret_status;
}

/**
  * @brief  Ymodem Header Packet Transfer completed callback.
  * @param  uFileSize: Dimension of the file that will be received.
  * @retval SFU_ErrorStatus SFU_SUCCESS if successful, SFU_ERROR otherwise.
  */
SFU_ErrorStatus SFU_COM_YMODEM_HeaderPktRxCpltCallback(uint32_t uFileSize)
{
  /*Reset of the ymodem variables */
  m_uFileSizeYmodem = 0U;
  m_uPacketsReceived = 0U;
  m_uNbrBlocksYmodem = 0U;

  /*Filesize information is stored*/
  m_uFileSizeYmodem = uFileSize;

  /*Compute the number of blocks */

  /* Teraterm sends 1kB YMODEM packets */
  //테라텀 1kB 패킷크기를 기준으로 몇 블록을 받아야 하는지 계산한다.
  m_uNbrBlocksYmodem = (m_uFileSizeYmodem + (SFU_COM_YMODEM_PACKET_1K_SIZE - 1U)) / SFU_COM_YMODEM_PACKET_1K_SIZE;

  /* NOTE : delay inserted for Ymodem protocol*/
  HAL_Delay(1000U);

  return SFU_SUCCESS;
}

/**
  * @brief  Ymodem Data Packet Transfer completed callback.
  * @param  pData: Pointer to the buffer.
  * @param  uSize: Packet dimension.
  * @retval SFU_ErrorStatus SFU_SUCCESS if successful, SFU_ERROR otherwise.
  */

/* Teraterm variant of the YMODEM protocol: uses 1kB packets */
SFU_ErrorStatus SFU_COM_YMODEM_DataPktRxCpltCallback(uint8_t *pData, uint32_t uSize)
{
  /* The local loader must make a copy of the Firmware metadata,
   * because this memory area is not copied when calling the SE_Decrypt_Init() primitive.
   * Hence we must make sure this memory area still contains the FW header when SE_Decrypt_Finish() is called. */
  static uint8_t fw_header[SE_FW_HEADER_TOT_LEN] __attribute__((aligned(8)));
  /* Size of downloaded Image initialized with first packet (header) and checked along download process */
  static uint32_t m_uDwlImgSize = 0U;

  SFU_ErrorStatus e_ret_status = SFU_SUCCESS;
  SFU_FLASH_StatusTypeDef x_flash_info;
  uint32_t rx_size = uSize;
  uint8_t *p_data = pData;
  uint32_t uOldSize;
#if  !defined(SFU_NO_SWAP)
  uint32_t uLength;
#endif /* !SFU_NO_SWAP */

  /* Check the pointers allocation */
  if (p_data == NULL)
  {
    return SFU_ERROR;
  }

  /*Increase the number of received packets*/
  m_uPacketsReceived++;

  /* Last packet : size of data to write could be different than SFU_COM_YMODEM_PACKET_1K_SIZE */
  //최초 계산한 블럭의 개수와 수신 패킷의 개수가 일치하는 경우는 결국 마지막 패킷을 수신할 때를 의미한다.
  if (m_uPacketsReceived == m_uNbrBlocksYmodem)
  {
    /*Extracting actual payload from last packet*/
    //마지막 패킷의 실제 크기를 확인한다. (패킷을 꽉채워서 데이터가 들어왔는지 아닌지 여부에 따라서 실제 rx_size 크기 계산)
    if (0U == (m_uFileSizeYmodem % SFU_COM_YMODEM_PACKET_1K_SIZE))
    {
      /* The last packet must be fully considered */
      rx_size = SFU_COM_YMODEM_PACKET_1K_SIZE;
    }
    else
    {
      /* The last packet is not full, drop the extra bytes */
      rx_size = m_uFileSizeYmodem - ((uint32_t)(m_uFileSizeYmodem / SFU_COM_YMODEM_PACKET_1K_SIZE) *
                                     SFU_COM_YMODEM_PACKET_1K_SIZE);
    }
  }

  /* First packet : Contains the FW header (SE_FW_HEADER_TOT_LEN bytes length) which is not encrypted  */
  //처음 패킷을 수신한 경우에는 fw 헤더정보를 포함하고 있다. 따라서 해당 fw 정보를 읽어서 어떤 다운로드 슬롯에 다운로드 해야하는지 확인이 필요하다.
  if (m_uPacketsReceived == 1U)
  {

    (void) memcpy(fw_header, p_data, SE_FW_HEADER_TOT_LEN);

    /* Verify header */
    //실제 헤더 정보가 올바른지 검증한다. 이 때 다운로드 할 슬롯이 결정된다.
    e_ret_status = SFU_LOADER_VerifyFwHeader(p_data);
    if (e_ret_status == SFU_SUCCESS)
    {
      m_uDwlAreaAddress = m_uDwlAreaStart;
#if   defined(SFU_NO_SWAP)

      /* Downloaded Image size : Header size + Image size */
      m_uDwlImgSize = ((SE_FwRawHeaderTypeDef *)(uint32_t)fw_header)->FwSize + SFU_IMG_IMAGE_OFFSET;
#else
      /* Downloaded Image size : Header size + gap for image alignment to (UpdateFwOffset % sector size) +
         Partial Image size */
      //다운로드 이미지의 크기는 헤더 영역의 크기(SFU_IMG_IMAGE_OFFSET), offset 의 크기, fw 이미지의 크기의 합이다.
      m_uDwlImgSize = ((SE_FwRawHeaderTypeDef *)(uint32_t)fw_header)->PartialFwSize +
                      (((SE_FwRawHeaderTypeDef *)(uint32_t)fw_header)->PartialFwOffset % SLOT_SIZE(SLOT_SWAP)) +
                      SFU_IMG_IMAGE_OFFSET;
#endif /* (SFU_NO_SWAP) */
    }

    /* Clear Download application area (including TRAILERS area) */
    //다운로드 슬롯이 결정되었으므로 해당 다운로드 슬롯을 erase 한다.
    if (e_ret_status == SFU_SUCCESS)
    {
      if (SFU_LL_FLASH_Erase_Size(&x_flash_info, (uint8_t *) m_uDwlAreaAddress, SLOT_SIZE(m_uDwlSlot)) !=
          SFU_SUCCESS)
      {
        m_LoaderStatus = SFU_LOADER_ERR_FLASH;
        e_ret_status = SFU_ERROR;
      }
    }

  }

#if  !defined(SFU_NO_SWAP)
  if (e_ret_status == SFU_SUCCESS)
  {
    /* This packet : contains end of FW header */
    //현재 수신한 패킷이 헤더의 끝이라면
    if ((m_uDwlAreaAddress < (m_uDwlAreaStart + SFU_IMG_IMAGE_OFFSET)) &&
        ((m_uDwlAreaAddress + rx_size) >= (m_uDwlAreaStart + SFU_IMG_IMAGE_OFFSET)))
    {
      /* Write the FW header part (SFU_IMG_IMAGE_OFFSET % SFU_COM_YMODEM_PACKET_1K_SIZE bytes length) */
      //헤더의 크기인 SFU_IMG_IMAGE_OFFSET 는 기종마다 크기가 상이해서 align 을 확인한다.
      //L4 의 경우에는 512이기 때문에 uLength 는 512가 된다.
      uLength = SFU_IMG_IMAGE_OFFSET % SFU_COM_YMODEM_PACKET_1K_SIZE;
      if (uLength == 0U)
      {
        uLength = SFU_COM_YMODEM_PACKET_1K_SIZE;
      }
      if (SFU_LL_FLASH_Write(&x_flash_info, (uint8_t *)m_uDwlAreaAddress, p_data, uLength) == SFU_SUCCESS)
      {
        /* Shift the DWL area pointer, to align image with (PartialFwOffset % sector size) in DWL area */
        m_uDwlAreaAddress += uLength + (((SE_FwRawHeaderTypeDef *)(uint32_t)fw_header)->PartialFwOffset %
                                        SLOT_SIZE(SLOT_SWAP));

        /* Update remaining packet size to write */
        rx_size -= uLength;

        /* Update p_data pointer to received packet data */
        p_data += uLength;
      }
      else
      {
        m_LoaderStatus = SFU_LOADER_ERR_FLASH;
        e_ret_status = SFU_ERROR;
      }
    }
  }
#endif /* !SFU_NO_SWAP */

  if (e_ret_status == SFU_SUCCESS)
  {
    /* Check size to avoid writing beyond DWL image size */
    //현재 수신한 크기가 다운로드 이미지 크기를 넘어서지 않는지 확인한다.
    if ((m_uDwlAreaAddress + rx_size) > (m_uDwlAreaStart + m_uDwlImgSize))
    {
      m_LoaderStatus = SFU_LOADER_ERR_FW_LENGTH;
      e_ret_status = SFU_ERROR;
    }
  }

  if (e_ret_status == SFU_SUCCESS)
  {
    /* Set dimension to the appropriate length for FLASH programming.
     * Example: 64-bit length for L4.
     */
    //수신한 크기가 flash 쓰기 단위에 align 되어 있는지 확인하고 align 되어 있지 않다면 나머지 부분은 0xFF 로 채워서 writing 한다.
    if ((rx_size % (uint32_t)sizeof(SFU_LL_FLASH_write_t)) != 0U)
    {
      /* By construction, the length of the buffer (fw_decrypted_chunk or p_data) must be a multiple of
         sizeof(SFU_IMG_write_t) to avoid reading out of the buffer */
      uOldSize = rx_size;
      rx_size = rx_size + ((uint32_t)sizeof(SFU_LL_FLASH_write_t) - (rx_size % (uint32_t)sizeof(SFU_LL_FLASH_write_t)));
      while (uOldSize < rx_size)
      {
        p_data[uOldSize] = 0xFF;
        uOldSize++;
      }
    }
  }

  if (e_ret_status == SFU_SUCCESS)
  {
    /* Check size to avoid writing beyond DWL area */
    //다운로드 슬롯의 영역을 벗어나지 않는지 확인한다.
    if ((m_uDwlAreaAddress + rx_size) > (m_uDwlAreaStart + m_uDwlAreaSize))
    {
      m_LoaderStatus = SFU_LOADER_ERR_FW_LENGTH;
      e_ret_status = SFU_ERROR;
    }
  }

  /* Write Data in Flash */
  //수신한 패킷을 flash 에 writing 한다.
  if (e_ret_status == SFU_SUCCESS)
  {
    if (SFU_LL_FLASH_Write(&x_flash_info, (uint8_t *)m_uDwlAreaAddress, p_data, rx_size) == SFU_SUCCESS)
    {
      m_uDwlAreaAddress += (rx_size);
    }
    else
    {
      m_LoaderStatus = SFU_LOADER_ERR_FLASH;
      e_ret_status = SFU_ERROR;
    }
  }


  /* Last packet : reset m_uPacketsReceived */
  //모든 패킷을 수신한 경우에는 변수를 초기화 한다.
  if (m_uPacketsReceived == m_uNbrBlocksYmodem)
  {
    m_uPacketsReceived = 0U;
  }

  /* Reset data counters in case of error */
  if (e_ret_status == SFU_ERROR)
  {
    /*Reset of the ymodem variables */
    m_uFileSizeYmodem = 0U;
    m_uPacketsReceived = 0U;
    m_uNbrBlocksYmodem = 0U;
  }

  return e_ret_status;
}

/**
  * @brief  Verifies the Raw Fw Header received. It checks if the header is
  *         authentic and if the fields are ok with the device (e.g. size and version).
  * @note   Please note that when the new image is installed, this metadata is checked
  *         by @ref SFU_IMG_CheckCandidateVersion.
  * @param  peSFU_LOADER_Status: SFU LOADER Status.
  *         This parameter can be a value of @ref SFU_LOADER_Status_Structure_definition.
  * @param  pBuffer: pointer to header Buffer.
  * @retval SFU_ErrorStatus SFU_SUCCESS if successful, SFU_ERROR otherwise.
  */

/* anti-rollback mechanism is tested twice to avoid basic hardware attack
   ==> compiler optimization is disabled for this reason */
#if defined(__ICCARM__)
#pragma optimize=none
#elif defined(__CC_ARM)
#pragma O0
#elif defined (__ARMCC_VERSION)
__attribute__((optnone))
#elif defined ( __GNUC__ )
__attribute__((optimize("O0")))
#endif /* __ICCARM__ */

static SFU_ErrorStatus SFU_LOADER_VerifyFwHeader(uint8_t *pBuffer)
{
  SFU_ErrorStatus e_ret_status = SFU_ERROR;
  SE_StatusTypeDef              e_se_status;
  SE_FwRawHeaderTypeDef         *p_x_fw_raw_header;
  SFU_FwImageFlashTypeDef       fw_image_flash_data;
  uint16_t                      cur_ver;
  uint16_t                      verif_ver;
  uint32_t                      active_slot;

  /*Parse the received buffer*/
  p_x_fw_raw_header = (SE_FwRawHeaderTypeDef *)(uint32_t)pBuffer;

  /*Check if the received header packet is authentic*/
  //헤더가 올바르게 signing 되었는지 검증한다.
  if (SE_VerifyHeaderSignature(&e_se_status, p_x_fw_raw_header) != SE_ERROR)
  {
    //헤더가 올바른 경우 헤더의 magic tag 를 확인하여 다운로드 슬롯을 확인한다.
    active_slot = SFU_IMG_GetFwImageSlot(p_x_fw_raw_header);
    /*
     * 2 slots per image configuration. Identify the download area.
     *   It will be based on SFU magic :
     *   - same number as active_slot if configured.
     *   - else : SLOT_DWL_1
     *    As example : Header magic is SFU2, active slot is SLOT_ACTIVE_2 and dwl slot will be SLOT_DWL2.
     */
    m_uDwlSlot = active_slot - SLOT_ACTIVE_1 + SLOT_DWL_1;
    if (SLOT_SIZE(m_uDwlSlot) == 1U)
    {
      m_uDwlSlot = SLOT_DWL_1;
    }
    SFU_IMG_GetDownloadAreaInfo(m_uDwlSlot, &fw_image_flash_data);

    /* Assign the download flash address to be used during the YMODEM process */
    //다운로드 슬롯정보를 바탕으로 위치와 크기정보를 확인한다.
    m_uDwlAreaStart = fw_image_flash_data.DownloadAddr;
    m_uDwlAreaSize = fw_image_flash_data.MaxSizeInBytes;

    /*
     * Check if the version is allowed
     * ==> SFU_IMG_GetActiveFwVersion() returns 0 if the header of active firmware is not valid
     */
    //현재 액티브 슬롯의 버전을 확인하여 설치가 가능한 버전인지 확인한다.
    cur_ver = SFU_IMG_GetActiveFwVersion(active_slot);
    if (SFU_IMG_CheckFwVersion(active_slot, cur_ver, p_x_fw_raw_header->FwVersion) == SFU_SUCCESS)
    {
#if  defined(SFU_NO_SWAP)
      /*
       * Check length : Let's make sure the Firmware image can be written in the destination slot.
       */
      if (p_x_fw_raw_header->FwSize > (SLOT_SIZE(m_uDwlSlot) - SFU_IMG_IMAGE_OFFSET))
#else
      /*
       * Check length :
       *      We do not check the length versus the trailer constraint because this check is already implemented in
       *      FirmwareToInstall().
       *      If the firmware is too big to have some room left for the trailer info then the installation will not be
       *      triggered.
       *      The interest is to avoid duplicating the checks (checking just in time).
       *      This is also because in the case of a download handled by the UserApp we cannot rely on the checks done
       *      by the UserApp before installing a FW.
       *      The drawback is that the firmware can be downloaded in dwl slot though it is too big.
       *
       *      Nevertheless, we can still detect if the FW is too big to be downloaded (cannot be written in dwl slot).
       *      This will avoid download issues (but the installation itself can still be rejected) or overflows.
       *      The dwl slot must contain the HEADER and also the binary FW (encrypted).
       *      But there is an offset of FW_OFFSET_IMAGE bytes to respect.
       */
        //이미지의 크기가 다운로드 슬롯 크기보다 큰 경우에는 크기 오류를 반환
      if ((p_x_fw_raw_header->PartialFwSize + (p_x_fw_raw_header->PartialFwOffset % SLOT_SIZE(SLOT_SWAP))) >
          (SLOT_SIZE(m_uDwlSlot) - SFU_IMG_IMAGE_OFFSET))
#endif /* SFU_NO_SWAP */
      {
        /* The firmware cannot be written in dwl slot */
        m_LoaderStatus = SFU_LOADER_ERR_FW_LENGTH;
      }
      else
      {
        /* Installation authorized */
        e_ret_status = SFU_SUCCESS;
      }
    }
    else
    {
      /* The installation is forbidden */
      //버전이 올바르지 않은 경우 버전 오류 반환
      TRACE("\r\n          Anti-rollback: candidate version(%d) rejected | current version(%d) , min.version(%d) !",
            p_x_fw_raw_header->FwVersion, cur_ver, SFU_FW_VERSION_START_NUM);
      m_LoaderStatus = SFU_LOADER_ERR_FW_VERSION;
    }


    /*
     * Control of version and length is done twice to avoid basic hardware attack
     * This more important for the 1 image configuration as the installation is done during the download procedure
     * For the 2 images configuration anti-rollback will be checked again during installation process
     */
    //정확하게 basic hardware attack 이 무엇인지는 모르겠으나 앞서 진행한 검증을 반복한다.
    verif_ver = SFU_IMG_GetActiveFwVersion(active_slot);
    if (SFU_IMG_CheckFwVersion(active_slot, verif_ver, p_x_fw_raw_header->FwVersion) == SFU_SUCCESS)
    {
#if  defined(SFU_NO_SWAP)
      /*
       * Check length : Let's make sure the Firmware image can be written in the destination slot.
       */
      if (p_x_fw_raw_header->FwSize > (SLOT_SIZE(m_uDwlSlot) - SFU_IMG_IMAGE_OFFSET))
#else
      /*
       * Check length :
       *      We do not check the length versus the trailer constraint because this check is already implemented in
       *      FirmwareToInstall().
       *      If the firmware is too big to have some room left for the trailer info then the installation will not be
       *      triggered.
       *      The interest is to avoid duplicating the checks (checking just in time).
       *      This is also because in the case of a download handled by the UserApp we cannot rely on the checks done
       *      by the UserApp before installing a FW.
       *      The drawback is that the firmware can be downloaded in dwl slot though it is too big.
       *
       *      Nevertheless, we can still detect if the FW is too big to be downloaded (cannot be written in dwl slot).
       *      This will avoid download issues (but the installation itself can still be rejected) or overflows.
       *      The dwl slot must contain the HEADER and also the binary FW (encrypted).
       *      But there is an offset of FW_OFFSET_IMAGE bytes to respect.
       */
      if ((p_x_fw_raw_header->PartialFwSize + (p_x_fw_raw_header->PartialFwOffset % SLOT_SIZE(SLOT_SWAP))) >
          (SLOT_SIZE(m_uDwlSlot) - SFU_IMG_IMAGE_OFFSET))
#endif /* SFU_NO_SWAP */
      {
        /* The firmware cannot be written in dwl slot */
        m_LoaderStatus = SFU_LOADER_ERR_FW_LENGTH;
      }
      else
      {
        /* Installation authorized */
        e_ret_status = SFU_SUCCESS;
      }
    }
    else
    {
      /* The installation is forbidden */
      TRACE("\r\n          Anti-rollback: candidate version(%d) rejected | current version(%d) , min.version(%d) !",
            p_x_fw_raw_header->FwVersion, verif_ver, SFU_FW_VERSION_START_NUM);
      m_LoaderStatus = SFU_LOADER_ERR_FW_VERSION;
    }
  }
  else
  {
    /* Header authentication failure */
    m_LoaderStatus = SFU_LOADER_ERR_AUTH_FAILED;
  }

  return e_ret_status;
}

#endif /* (SECBOOT_LOADER == SECBOOT_USE_LOCAL_LOADER) */