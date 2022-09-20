/**
  ******************************************************************************
  * @file    sfu_fwimg_no_swap.c
  * @author  MCD Application Team
  * @brief   This file provides set of firmware functions to manage the Firmware Images.
  *          This file contains the specific functionalities of the image handling for
  *          the installation process without swap area.
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

#define SFU_FWIMG_NO_SWAP_C

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "main.h"
#include "sfu_fsm_states.h" /* needed for sfu_error.h */
#include "sfu_error.h"
#include "sfu_low_level_flash_int.h"
#include "sfu_low_level_security.h"
#include "se_interface_bootloader.h"
#include "sfu_fwimg_regions.h"
#include "sfu_fwimg_services.h"
#include "sfu_fwimg_internal.h"
#include "sfu_trace.h"
#include "sfu_boot.h"

#if  (SECBOOT_LOADER == SECBOOT_USE_STANDALONE_LOADER)
/* Private defines -----------------------------------------------------------*/
#define FLASH_ERASE_SIZE FLASH_PAGE_SIZE

/* Private variables ---------------------------------------------------------*/
static  uint8_t decrypted_sector[FLASH_ERASE_SIZE] __attribute__((aligned(8)));
#endif /* (SECBOOT_LOADER == SECBOOT_USE_STANDALONE_LOADER)) */


/* Functions Definition : helper ---------------------------------------------*/

#if  (SECBOOT_LOADER == SECBOOT_USE_STANDALONE_LOADER)
/**
  * @brief  Decrypt Image "in place"
  * @param  SlotNumber index of the slot in the list
  * @param  pFwImageHeader pointer to fw header
  * @retval SFU_SUCCESS if successful, a SFU_ErrorStatus error otherwise.
  */
static SFU_ErrorStatus DecryptImageInPlace(uint32_t SlotNumber, SE_FwRawHeaderTypeDef *pFwImageHeader)
{
  SFU_ErrorStatus  e_ret_status = SFU_ERROR;
  SE_StatusTypeDef e_se_status;
  SE_ErrorStatus   se_ret_status = SE_ERROR;
  uint32_t number_chunk_per_sector = FLASH_ERASE_SIZE / SFU_IMG_CHUNK_SIZE;
  SFU_FLASH_StatusTypeDef flash_if_status;
  uint8_t fw_encrypted_chunk[SFU_IMG_CHUNK_SIZE] __attribute__((aligned(8)));
  uint32_t fw_dest_address_write = 0U;
  uint32_t fw_dest_erase_address = 0U;
  int32_t fw_decrypted_total_size = 0U;
  int32_t size;
  int32_t fw_decrypted_chunk_size;
  int32_t fw_tag_len = 0U;
  uint8_t fw_tag_output[SE_TAG_LEN];
  uint32_t pass_index = 0;
  uint32_t erase_index = 0U;
  uint8_t *pfw_source_address = 0U;
  uint32_t decrypted_sector_index = 0U;

  /* Verify header presence */
  if ((pFwImageHeader == NULL))
  {
    return e_ret_status;
  }

  /* Decryption process initialization*/
  se_ret_status = SE_Decrypt_Init(&e_se_status, pFwImageHeader, SE_FW_IMAGE_COMPLETE);
  if (se_ret_status == SE_SUCCESS)
  {
    e_ret_status = SFU_SUCCESS;
    size = SFU_IMG_CHUNK_SIZE;

    /*
    * Chunk Decryption loop
    * Each ((pass_index % number_chunk_per_sector we should) == 0)  :
    *   - erase the flash (FLASH_ERASE_SIZE)
    *   - write the decyrpted buffer
    * ==> decrypted buffer size == FLASH_ERASE_SIZE
     */

    while ((e_ret_status == SFU_SUCCESS) && (fw_decrypted_total_size < (pFwImageHeader->FwSize))
           && (se_ret_status == SE_SUCCESS))
    {
      /* pass_index == 0 : manage header
       =============================== */
      if (pass_index == 0)
      {
        /* By pass header ==> it will be written at the end of installation process */
        decrypted_sector_index += SFU_IMG_IMAGE_OFFSET;

        /* pointers initialization phase */
        pfw_source_address = (uint8_t *)(SlotStartAdd[SlotNumber] + SFU_IMG_IMAGE_OFFSET);
        fw_dest_erase_address = SlotStartAdd[SlotNumber];
        fw_dest_address_write = SlotStartAdd[SlotNumber];
        erase_index = number_chunk_per_sector;

        /*
         * Warning : SFU_IMG_CHUNK_SIZE should be equal to SFU_IMG_IMAGE_OFFSET and <= FLASH_ERASE_SIZE
         * ==> first chunk is the header - no decryption needed : pass_index = 1
         */
        pass_index = 1;

        /* specific case where SFU_IMG_CHUNK_SIZE == FLASH_ERASE_SIZE
         * bypass first SFU_IMG_CHUNK_SIZE which contains only header
         */
        if (number_chunk_per_sector == 1)
        {
          e_ret_status = SFU_LL_FLASH_Erase_Size(&flash_if_status, (uint8_t *)fw_dest_erase_address, FLASH_ERASE_SIZE);
          STATUS_FWIMG(e_ret_status == SFU_ERROR, SFU_IMG_FLASH_ERASE_FAILED);
          erase_index += number_chunk_per_sector;
          fw_dest_erase_address += FLASH_ERASE_SIZE;
          fw_dest_address_write += FLASH_ERASE_SIZE;
          decrypted_sector_index = 0;
        }
      }

      /* Set size of reading/decryption
       ============================== */
      fw_decrypted_chunk_size = SFU_IMG_CHUNK_SIZE;
      /* Last pass ? */
      if ((pFwImageHeader->FwSize - fw_decrypted_total_size) < fw_decrypted_chunk_size)
      {
        /* Default chunk size can be troncated at end of file */
        fw_decrypted_chunk_size = pFwImageHeader->FwSize - fw_decrypted_total_size;
      }
      else
      {
        /* nothing */
      }
      size = fw_decrypted_chunk_size;

      /* pass_index != 0 : read then decrypt phase
       ========================================= */
      e_ret_status = SFU_LL_FLASH_Read(fw_encrypted_chunk, pfw_source_address, size);
      if (e_ret_status == SFU_ERROR)
      {
        break;
      }
      if (size != 0)
      {
        se_ret_status = SE_Decrypt_Append(&e_se_status, (uint8_t *)fw_encrypted_chunk, size,
                                          (uint8_t *)(decrypted_sector + decrypted_sector_index),
                                          (int32_t *)&fw_decrypted_chunk_size);
      }
      else
      {
        /* specific case where last pass size is 0 */
        e_ret_status = SFU_SUCCESS;
        fw_decrypted_chunk_size = 0;
      }


      /*
      * Erasing and writing phase when :
      * ================================
      *   - number_chunk_per_sector reached
      *   - last past identified
      */
      if ((se_ret_status == SE_SUCCESS) && (e_ret_status == SFU_SUCCESS) && (fw_decrypted_chunk_size == size))
      {
        decrypted_sector_index += size;
        fw_decrypted_total_size += size;

        /* (number_chunk_per_sector reached) or (last past identified) */
        if (pass_index == erase_index - 1 || (fw_decrypted_total_size == pFwImageHeader->FwSize))
        {
          /* Erasing operation
             ================= */
          SFU_LL_SECU_IWDG_Refresh();
          e_ret_status = SFU_LL_FLASH_Erase_Size(&flash_if_status, (uint8_t *)fw_dest_erase_address, FLASH_ERASE_SIZE);
          STATUS_FWIMG(e_ret_status == SFU_ERROR, SFU_IMG_FLASH_ERASE_FAILED);
          erase_index += number_chunk_per_sector;
          fw_dest_erase_address += FLASH_ERASE_SIZE;

          /*
           * After erase, re-write the same sector with decrypted data
           * =========================================================
           * pass_index < number_chunk_per_sector ==> first sector. Header is included in the decrypted buffer.
           * It will be written in active slot at the end of installation process
           * otherwise : write decrypted buffer
           */
          if (pass_index < number_chunk_per_sector)
          {
            e_ret_status = SFU_LL_FLASH_Write(&flash_if_status,
                                              (uint8_t *)(fw_dest_address_write + SFU_IMG_IMAGE_OFFSET),
                                              (uint8_t *)(decrypted_sector + SFU_IMG_IMAGE_OFFSET),
                                              FLASH_ERASE_SIZE - SFU_IMG_IMAGE_OFFSET);

          }
          else
          {
            e_ret_status = SFU_LL_FLASH_Write(&flash_if_status, (uint8_t *)fw_dest_address_write,  decrypted_sector,
                                              FLASH_ERASE_SIZE);
          }
          STATUS_FWIMG(e_ret_status == SFU_ERROR, SFU_IMG_FLASH_WRITE_FAILED);

          if (e_ret_status == SFU_SUCCESS)
          {
            decrypted_sector_index = 0;
            memset(decrypted_sector, 0xff, sizeof(decrypted_sector));
            fw_dest_address_write  += (FLASH_ERASE_SIZE); /* Next sector */

          }
        }
        /* Update source pointer */
        pfw_source_address += size;
        pass_index += 1;
      }
    }
  }

#if (SFU_IMAGE_PROGRAMMING_TYPE == SFU_ENCRYPTED_IMAGE)
#if defined(SFU_VERBOSE_DEBUG_MODE)
  TRACE("\r\n\t  %d bytes of ciphertext decrypted.", fw_decrypted_total_size);
#endif /* SFU_VERBOSE_DEBUG_MODE */
#endif /* SFU_ENCRYPTED_IMAGE */

  if ((se_ret_status == SE_SUCCESS) && (e_ret_status == SFU_SUCCESS))
  {
    /* Do the Finalization, check the authentication TAG*/
    fw_tag_len = sizeof(fw_tag_output);
    se_ret_status = SE_Decrypt_Finish(&e_se_status, fw_tag_output, &fw_tag_len);
    if (se_ret_status != SE_SUCCESS)
    {
      e_ret_status = SFU_ERROR;
#if defined(SFU_VERBOSE_DEBUG_MODE)
      TRACE("\r\n\t  Decrypt fails at Finalization stage.");
#endif /* SFU_VERBOSE_DEBUG_MODE */
    }
  }
  else
  {
    e_ret_status = SFU_ERROR;
  }
  return e_ret_status;
}

/**
  * @brief  Install the new version
  * @param  SlotNumber index of the slot in the list
  * @param  pFwImageHeader pointer to fw header
  * @retval SFU_SUCCESS if successful,SFU_ERROR error otherwise.
  */
static SFU_ErrorStatus InstallNewVersion(uint32_t SlotNumber, SE_FwRawHeaderTypeDef *pFwImageHeader)
{
  SFU_ErrorStatus e_ret_status = SFU_ERROR;
  SE_StatusTypeDef e_se_status;
  SFU_FLASH_StatusTypeDef flash_if_status;

  /*
   * Control if there is no additional code beyond the firmware image (malicious SW)
   */
  e_ret_status = VerifySlot((uint8_t *) SlotStartAdd[SlotNumber], SLOT_SIZE(SlotNumber), pFwImageHeader->FwSize);
  if (e_ret_status != SFU_SUCCESS)
  {
    SFU_EXCPT_SetError(SFU_EXCPT_ADDITIONAL_CODE_ERR);
    return e_ret_status;
  }

  /*
   * Decrypt "in place" the FW image
   */
  e_ret_status = DecryptImageInPlace(SlotNumber, pFwImageHeader);
  if (e_ret_status != SFU_SUCCESS)
  {
    SFU_EXCPT_SetError(SFU_EXCPT_DECRYPT_ERR);
    return e_ret_status;
  }

  /*
   * Verify signature after decrypt
   */
  e_ret_status = VerifyFwSignature(&e_se_status, SlotNumber, pFwImageHeader, SE_FW_IMAGE_COMPLETE);
  if (e_ret_status != SFU_SUCCESS)
  {
    SFU_EXCPT_SetError(SFU_EXCPT_SIGNATURE_ERR);
    return e_ret_status;
  }

  /*
   * Erase header not contiguous with DWL area in this configuration
   */
  e_ret_status = SFU_LL_FLASH_Erase_Size(&flash_if_status, (uint8_t *)SlotHeaderAdd[SlotNumber], SFU_IMG_IMAGE_OFFSET);
  STATUS_FWIMG(e_ret_status == SFU_ERROR, SFU_IMG_FLASH_ERASE_FAILED);
  if (e_ret_status != SFU_SUCCESS)
  {
    SFU_EXCPT_SetError(SFU_EXCPT_HEADER_ERASING_ERR);
    return e_ret_status;
  }

  /*
   * validate immediately the new active FW
   */
  e_ret_status = SFU_IMG_Validation(SlotNumber, pFwImageHeader);
  if (e_ret_status != SFU_SUCCESS)
  {
    SFU_EXCPT_SetError(SFU_EXCPT_HEADER_VALIDATION_ERR);
    return e_ret_status;
  }

  /* Return the result of this preparation */
  return (e_ret_status);
}

/**
  * @brief  Check that there is an Image to Install
  * @param  DwlSlot identification of the downloaded area
  * @param  pFwImageHeader pointer to fw header to be populated
  * @retval SFU_SUCCESS if Image can be installed, a SFU_ERROR  otherwise.
  */
static SFU_ErrorStatus FirmwareToInstall(uint32_t DwlSlot, SE_FwRawHeaderTypeDef *pFwImageHeader)
{
  SFU_ErrorStatus e_ret_status = SFU_ERROR;

  /*
   * Verify FW Image Header tag in dwl slot :
   *   Cannot call CheckAndGetFWHeader(DwlSlot, pFwImageHeader) which control the header
   *   ==> in non contiguous configuration the header is loacted outside the DWL area
   */
  e_ret_status = SFU_LL_FLASH_Read((uint8_t *) pFwImageHeader, (uint8_t *) SlotStartAdd[DwlSlot],
                                   sizeof(SE_FwRawHeaderTypeDef));
  if (e_ret_status == SFU_SUCCESS)
  {
    /* Verify signature */
    e_ret_status = VerifyHeaderSignature(pFwImageHeader);
  }

  return e_ret_status;
}

#endif /*(GENERATOR_1_FW_IMAGE) && (SECBOOT_LOADER == SECBOOT_USE_STANDALONE_LOADER) */


/* Functions Definition : services ------------------------------------------- */


#if  (SECBOOT_LOADER == SECBOOT_USE_STANDALONE_LOADER)
/**
  * @brief Checks if there is a pending firmware installation.
  *        2 situations can occur:
  *        A. Pending firmware installation: a firmware is ready to be installed.
  *        B. No firmware installation pending
  * @note  The anti-rollback check is not taken into account at this stage.
  * @note  This function populates the FWIMG module variables: fw_image_header_to_test
  * @param  pDwlSlotToInstall identification of the downloaded slot to install
  * @param  pActiveSlotToResume identification of the active slot to resume
  * @retval SFU_IMG_ImgInstallStateTypeDef Pending Installation status (pending install, no pending action)
  */
SFU_IMG_ImgInstallStateTypeDef SFU_IMG_CheckPendingInstallation(uint32_t *pDwlSlotToInstall)
{
  SFU_IMG_ImgInstallStateTypeDef e_ret_state = SFU_IMG_NO_FWUPDATE;
  uint32_t i;

  *pDwlSlotToInstall = SLOT_INACTIVE;

  for (i = 0U; i < SFU_NB_MAX_ACTIVE_IMAGE; i++)
  {
    /* Slot configured ? */
    if (SlotStartAdd[SLOT_ACTIVE_1 + i] != 0U)
    {
      if (SFU_SUCCESS == FirmwareToInstall(SLOT_ACTIVE_1 + i, &fw_image_header_to_test))
      {
        /*
         * A Firmware is available for installation:
         * fw_image_header_to_test has been populated
         * Stop at first occurrence : next ones if any will be installed after reset
         */
        e_ret_state = SFU_IMG_FWIMAGE_TO_INSTALL;
        *pDwlSlotToInstall = SLOT_ACTIVE_1 + i;
        break;
      }
    }
  }

  return e_ret_state;
}

/**
  * @brief Installs a new firmware, performs the post-installation checks and sets the metadata to tag this firmware as
  *        valid if the checks are successful.
  *        Cryptographic operations are used (if enabled): the firmware is decrypted and its authenticity is checked
  *        afterwards if the cryptographic scheme allows this (signature check for instance).
  *        The detailed errors are memorized in bootinfo area to be processed as critical errors if needed.
  *        This function modifies the FLASH content.
  *        If this procedure is interrupted (e.g.: switch off) installation will be restarted from begin at next reboot.
  * @param  DwlSlot identification of the downloaded area
  * @retval SFU_ErrorStatus SFU_SUCCESS if successful, SFU_ERROR otherwise.
  */
SFU_ErrorStatus SFU_IMG_TriggerImageInstallation(uint32_t DwlSlot)
{
  SFU_ErrorStatus e_ret_status = SFU_ERROR;

  /*
   * Installing the new firmware image consists in decrypting the candidate firmware image.
   * fw_image_header_to_test is already populated after SFU_IMG_CheckPendingInstallation() call.
   */
  e_ret_status = InstallNewVersion(DwlSlot, &fw_image_header_to_test);

  if (SFU_SUCCESS != e_ret_status)
  {
    /* Erase downloaded FW in case of installation  error */
    (void) SFU_IMG_EraseDownloadedImg(DwlSlot);
  }

#if defined(SFU_VERBOSE_DEBUG_MODE)
  if (e_ret_status == SFU_SUCCESS)
  {
    TRACE("\r\n\t  Installation procedure completed.");
  }
  else
  {
    TRACE("\r\n\t  Installation procedure cannot be finalized!");
  }
#endif /* SFU_VERBOSE_DEBUG_MODE */

  /* return the installation result */
  return (e_ret_status);
}

#endif /* (SECBOOT_LOADER == SECBOOT_USE_STANDALONE_LOADER) */

#if  SECBOOT_LOADER == SECBOOT_USE_STANDALONE_LOADER
/**
  * @brief  Erase downloaded firmware in case of decryption/authentication/integrity error
  * @param  DwlSlot identification of the downloaded area
  * @retval SFU_SUCCESS if successful, a SFU_ErrorStatus error otherwise.
  */
SFU_ErrorStatus SFU_IMG_EraseDownloadedImg(uint32_t DwlSlot)
{
  SFU_FLASH_StatusTypeDef flash_if_status;
  SFU_ErrorStatus e_ret_status_dwl = SFU_ERROR;

  e_ret_status_dwl = SFU_LL_FLASH_Erase_Size(&flash_if_status, (uint8_t *) SlotStartAdd[DwlSlot],
                                             SLOT_SIZE(DwlSlot));
  STATUS_FWIMG(e_ret_status_dwl == SFU_ERROR, SFU_IMG_FLASH_ERASE_FAILED);

  return ((e_ret_status_dwl != SFU_SUCCESS) ?  SFU_ERROR : SFU_SUCCESS);
}

#endif /* (SECBOOT_LOADER == SECBOOT_USE_STANDALONE_LOADER) || (SFU_NO_SWAP) */
