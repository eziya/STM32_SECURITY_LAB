/**
  ******************************************************************************
  * @file    sfu_fwimg_swap.c
  * @author  MCD Application Team
  * @brief   This file provides set of firmware functions to manage the Firmware Images.
  *          This file contains the specific functionalities of the image handling for
  *          the installation process with swap area.
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

#define SFU_FWIMG_SWAP_C

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "main.h"
#include "sfu_fsm_states.h" /* needed for sfu_error.h */
#include "sfu_error.h"
#include "sfu_low_level_flash_int.h"
#include "sfu_low_level_security.h"
#include "se_interface_bootloader.h"
#include "sfu_fwimg_regions.h"
#include "sfu_fwimg_services.h" /* to have definitions like SFU_IMG_InitStatusTypeDef
                                   (required by sfu_fwimg_internal.h) */
#include "sfu_fwimg_internal.h"
#include "sfu_trace.h"
#include "sfu_boot.h"

#if  !defined(SFU_NO_SWAP)

/* Private macros ------------------------------------------------------------*/
#define TRAILER_INDEX(A)  (SLOT_SIZE(A) / SLOT_SIZE(SLOT_SWAP))
/*  position of image begin on 1st block */
#define TRAILER_HEADER   (SE_FW_HEADER_TOT_LEN + SE_FW_HEADER_TOT_LEN + MAGIC_LENGTH + MAGIC_LENGTH)

/*                                         DWL_SLOT_REGION_SIZE                                             */
/* <----------------------------------------------------------------------------------------------------->  */
/* |HEADER_TOT_LEN|HEADER_TOT_LEN|MAGIC_LENGTH|MAGIC_LENGTH|N*sizeof(FLASH_write_t)|N*sizeof(FLASH_write_t) */
/* | header 1     | header 2     |SWAP magic  |CLEAN pat.  | N*CPY_TO_ACTIVE_SLOT  | N*CPY_TO_DWL_SLOT      */
/*                                                                                                          */
/* Please note that the size of the trailer area (N*CPY_TO_SLOTx) depends directly on SFU_LL_FLASH_write_t type, */
/* so it can differ from one platform to another (this is FLASH-dependent)                                       */
#define TRAILER_SIZE(A) ((sizeof(SFU_LL_FLASH_write_t)*(TRAILER_INDEX(A)))\
                         + (sizeof(SFU_LL_FLASH_write_t)*(TRAILER_INDEX(A))) + (uint32_t)(TRAILER_HEADER))
#define TRAILER_BEGIN(A)  (( uint8_t *)(SlotStartAdd[A]\
                                        + SLOT_SIZE(A) - TRAILER_SIZE(A)))
#define TRAILER_CPY_TO_DWL_SLOT(A,i) ((void*)((uint32_t)TRAILER_BEGIN(A)\
                                              + (sizeof(SFU_LL_FLASH_write_t)*(TRAILER_INDEX(A))) \
                                              + (uint32_t)TRAILER_HEADER+((i)*sizeof(SFU_LL_FLASH_write_t))))
#define TRAILER_CPY_TO_ACTIVE_SLOT(A,i) ((void*)((uint32_t)TRAILER_BEGIN(A)\
                                                 + (uint32_t)TRAILER_HEADER+((i)*sizeof(SFU_LL_FLASH_write_t))))

#define TRAILER_HDR_VALID(A) ((uint8_t *)(TRAILER_BEGIN(A)))
#define TRAILER_HDR_TEST(A)  ((uint8_t *)(TRAILER_BEGIN(A) + SE_FW_HEADER_TOT_LEN))
#define TRAILER_SWAP_ADDR(A) ((uint8_t *)(TRAILER_BEGIN(A) + SE_FW_HEADER_TOT_LEN + SE_FW_HEADER_TOT_LEN))
#define TRAILER_CLEAN_ADDR(A)((uint8_t *)(TRAILER_BEGIN(A)\
                                          + SE_FW_HEADER_TOT_LEN + SE_FW_HEADER_TOT_LEN + MAGIC_LENGTH))

#define CHUNK_1_ADDR(A,B,C) ((uint8_t *)(SlotStartAdd[A]\
                                         +(SLOT_SIZE(SLOT_SWAP)*(B))+(SFU_IMG_CHUNK_SIZE*(C))))
#define CHUNK_0_ADDR(A,B,C) ((uint8_t *)(SlotStartAdd[A]\
                                         +(SLOT_SIZE(SLOT_SWAP)*(B))+(SFU_IMG_CHUNK_SIZE*(C))))

#define CHUNK_0_ADDR_HEADER(A,B,C) ((((B)==0) && ((C)==0))?\
                                    (uint8_t *)(SlotHeaderAdd[(A)]) : \
                                    (uint8_t *)(SlotStartAdd[(A)]\
                                                +(SLOT_SIZE(SLOT_SWAP)*(B))+(SFU_IMG_CHUNK_SIZE*(C))))

#define CHUNK_0_ADDR_MODIFIED(A,B,C) ((((B)==0) && ((C)==0))?\
                                      ((uint8_t*)(SlotStartAdd[(A)] + SFU_IMG_IMAGE_OFFSET)) : \
                                      ((uint8_t*)(SlotStartAdd[(A)] + (SLOT_SIZE(SLOT_SWAP)*(B)) \
                                                  + (SFU_IMG_CHUNK_SIZE*(C)))))
#define CHUNK_SWAP_ADDR(B) ((uint8_t *)(SlotStartAdd[SLOT_SWAP]+(SFU_IMG_CHUNK_SIZE*(B))))

/* Private variables ----------------------------------------------------------*/
/* Trailer pattern : sizeof of write access type */
static const int8_t SWAPPED[sizeof(SFU_LL_FLASH_write_t)] __attribute__((aligned(8))) = {0, 0, 0, 0, 0, 0, 0, 0};
static const int8_t NOT_SWAPPED[sizeof(SFU_LL_FLASH_write_t)]  __attribute__((aligned(8))) = {-1, -1, -1, -1, -1, -1,
                                                                                              -1, -1
                                                                                             };

/* Functions Definition : helper ---------------------------------------------*/

/**
  * @brief  Check the magic from trailer or counter
  * @param  DwlSlot identification of the downloaded area
  * @retval SFU_ErrorStatus SFU_SUCCESS if successful, SFU_ERROR otherwise.
  */
static SFU_ErrorStatus CheckTrailerMagic(uint32_t DwlSlot)
{
  uint8_t  magic[MAGIC_LENGTH];
  uint8_t  clean[MAGIC_LENGTH];
  uint8_t  signature_valid[MAGIC_LENGTH / 2];
  uint8_t  signature_test[MAGIC_LENGTH / 2];
  uint8_t  erased_flash_pattern[MAGIC_LENGTH];

  /* Pattern initialization */
  (void) memset(erased_flash_pattern, 0xFF, MAGIC_LENGTH);

  /* Read trailer fields : TRAILER_HDR_VALID, TRAILER_HDR_TEST, MAGIC, CLEAN */
  //다운로드 슬롯 트레일러의 valid 헤더의 signature 에서 1/2을 읽어온다.
  if (SFU_LL_FLASH_Read(signature_valid,
                        (uint8_t *)((uint32_t)TRAILER_HDR_VALID(DwlSlot) + SE_FW_AUTH_LEN + (MAGIC_LENGTH / 2U)),
                        sizeof(signature_valid)) != SFU_SUCCESS)
  {
    return SFU_ERROR;
  }
  //다운로드 슬롯 트레일러의 test 헤더의 signature 에서 1/2을 읽어온다.
  if (SFU_LL_FLASH_Read(signature_test,
                        (uint8_t *)((uint32_t) TRAILER_HDR_TEST(DwlSlot) + SE_FW_AUTH_LEN + (MAGIC_LENGTH / 2U)),
                        sizeof(signature_test)) != SFU_SUCCESS)
  {
    return SFU_ERROR;
  }
  //다운로드 슬롯 트레일러의 SWAP 위치에 값을 읽어본다.
  if (SFU_LL_FLASH_Read(magic, TRAILER_SWAP_ADDR(DwlSlot), MAGIC_LENGTH) != SFU_SUCCESS)
  {
    return SFU_ERROR;
  }
  //다운로드 슬롯 트레일러의 CLEAN 위치의 값을 읽어본다.
  if (SFU_LL_FLASH_Read(clean, TRAILER_CLEAN_ADDR(DwlSlot), MAGIC_LENGTH) != SFU_SUCCESS)
  {
    return SFU_ERROR;
  }

  /*
   * Check magic validity :
   *  - last 16 bytes of validated header
   *  - last 16 bytes of test header
   *  - significant values (not an erased flash pattern)
   *
   *  Check clean tag not set :
   *  - should be erased flash pattern but not clean-up paatern (0x55 0x55 ...)
   */
  //magic 값이 valid signature 와 test signature 가 merge 된 값과 동일한지 확인한다.
  //clean 값이 0xFF 인지 확인한다. (clean 값은 설치가 완료되면 0x55로 써진다)
  if ((memcmp(magic, signature_valid, sizeof(signature_valid)) != 0)
      || (memcmp(&magic[MAGIC_LENGTH / 2U], signature_test, sizeof(signature_test)) != 0)
      || (memcmp(magic, erased_flash_pattern, MAGIC_LENGTH) == 0)
      || (memcmp(clean, erased_flash_pattern, MAGIC_LENGTH) != 0))
  {
    return SFU_ERROR;
  }
  return SFU_SUCCESS;
}

/**
  * @brief  Write the minimum value possible on the flash
  * @param  pAddr: pointer to address to write .
  * @param  pValue: pointer to the value to write
  * @retval SFU_ErrorStatus SFU_SUCCESS if successful, SFU_ERROR otherwise.
  *
  * @note This function should be FLASH dependent.
  *       We abstract this dependency thanks to the type SFU_LL_FLASH_write_t.
  *       See @ref SFU_LL_FLASH_write_t
  */
static SFU_ErrorStatus AtomicWrite(uint8_t *pAddr, SFU_LL_FLASH_write_t *pValue)
{
  SFU_FLASH_StatusTypeDef flash_if_info;

  return SFU_LL_FLASH_Write(&flash_if_info, pAddr, (uint8_t *)pValue, sizeof(SFU_LL_FLASH_write_t));
}

/**
  * @brief  Clean Magic value
  * @param  DwlSlot identification of the downloaded area
  * @retval SFU_ErrorStatus SFU_SUCCESS if successful, SFU_ERROR otherwise.
  */
static SFU_ErrorStatus CleanMagicValue(uint32_t DwlSlot)
{
  SFU_FLASH_StatusTypeDef flash_if_info;
  uint8_t clean_tag[MAGIC_LENGTH];

  /* set the clean tag into trailer */
  (void) memset(clean_tag, 0x55, MAGIC_LENGTH);
  return SFU_LL_FLASH_Write(&flash_if_info, TRAILER_CLEAN_ADDR(DwlSlot), clean_tag, MAGIC_LENGTH);
}

/**
  * @brief  Write Trailer Headers : TEST + VALID + SWAP
  * @param  DwlSlot identification of the downloaded area
  * @param  pValidHeader: pointer in ram to header of valid fw to backup
  * @param  pTestHeader: pointer in ram to header of fw to test
  * @retval SFU_ErrorStatus SFU_SUCCESS if successful, SFU_ERROR otherwise.
  */
static SFU_ErrorStatus WriteTrailerHeader(uint32_t DwlSlot, SE_FwRawHeaderTypeDef *pValidHeader,
                                          SE_FwRawHeaderTypeDef *pTestHeader)
{
  /* everything is in place , just compute from present data and write it */
  SFU_ErrorStatus e_ret_status = SFU_ERROR;
  SFU_FLASH_StatusTypeDef flash_if_info;
  uint8_t  magic[MAGIC_LENGTH];
  uint32_t buffer;

  /* Write Headers VALID + TEST */
  //트레일러 영역에 테스트 헤더를 기록한다.
  e_ret_status = SFU_LL_FLASH_Write(&flash_if_info, TRAILER_HDR_TEST(DwlSlot), (uint8_t *) pTestHeader,
                                    SE_FW_HEADER_TOT_LEN);
  if (e_ret_status == SFU_SUCCESS)
  {
    //트레일러 영역에 valid 헤더를 기록한다.
    e_ret_status = SFU_LL_FLASH_Write(&flash_if_info, TRAILER_HDR_VALID(DwlSlot), (uint8_t *) pValidHeader,
                                      SE_FW_HEADER_TOT_LEN);
  }

  /* Write Magic */
  //test 헤더와 valid 헤더를 반반씩 섞어서 트레일러 SWAP 영역에 기록한다.
  if (e_ret_status == SFU_SUCCESS)
  {
    buffer = (uint32_t) pValidHeader;
    (void) memcpy(&magic[0U], (uint8_t *)(buffer + SE_FW_AUTH_LEN + (MAGIC_LENGTH / 2U)), MAGIC_LENGTH / 2U);
    buffer = (uint32_t) pTestHeader;
    (void) memcpy(&magic[MAGIC_LENGTH / 2U], (uint8_t *)(buffer + SE_FW_AUTH_LEN + (MAGIC_LENGTH / 2U)),
                  MAGIC_LENGTH / 2U);
    e_ret_status = SFU_LL_FLASH_Write(&flash_if_info, TRAILER_SWAP_ADDR(DwlSlot), magic, MAGIC_LENGTH);
  }

  return e_ret_status;
}

/**
  * @brief  Erase the size of the swap area in a given slot sector.
  * @note   The erasure occurs at @: @slot + index*swap_area_size
  * @param  SlotNumber index of the slot in the list
  * @param  Index This is the number of "swap size" we jump from the slot start
  * @retval SFU_ SUCCESS if valid, a SFU_ErrorStatus error otherwise.
  */
static SFU_ErrorStatus EraseSlotIndex(uint32_t SlotNumber, uint32_t index)
{
  SFU_FLASH_StatusTypeDef flash_if_status;
  SFU_ErrorStatus e_ret_status = SFU_ERROR;
  uint32_t buffer;

  buffer = SlotStartAdd[SlotNumber];
  buffer = buffer + (SLOT_SIZE(SLOT_SWAP) * index);

  /* If this is an active slot and the index is 0 then header is concerned */
  if ((SlotNumber >= SLOT_ACTIVE_1) && (SlotNumber <= (SLOT_ACTIVE_1 + SFU_NB_MAX_ACTIVE_IMAGE)) && (index == 0U))
  {

    /*
     *  As the header may not be contiguous with FW, this action is split in 2 actions :
     *    - HEADER : executed in secured environment
     *    - remaining part : done by SBSFU
     */
    e_ret_status = SFU_LL_FLASH_Erase_Size(&flash_if_status, (uint8_t *) SlotHeaderAdd[SlotNumber],
                                           SFU_IMG_IMAGE_OFFSET) ;
    if (e_ret_status == SFU_SUCCESS)
    {
      e_ret_status = SFU_LL_FLASH_Erase_Size(&flash_if_status, (uint8_t *) SlotStartAdd[SlotNumber] +
                                             SFU_IMG_IMAGE_OFFSET, SLOT_SIZE(SLOT_SWAP) - SFU_IMG_IMAGE_OFFSET);
    }
  }
  else
  {
    e_ret_status = SFU_LL_FLASH_Erase_Size(&flash_if_status, (uint8_t *)buffer, SLOT_SIZE(SLOT_SWAP)) ;
  }

  STATUS_FWIMG(e_ret_status == SFU_ERROR, SFU_IMG_FLASH_ERASE_FAILED)
  return e_ret_status;
}

/**
  * @brief  Verify image signature of binary after decryption
  * @param  DwlSlot identification of the downloaded area
  * @param  pSeStatus pointer giving the SE status result
  * @param  pFwImageHeader pointer to fw header
  * @retval SFU_SUCCESS if successful, a SFU_ErrorStatus error otherwise.
  */
static SFU_ErrorStatus VerifyFwSignatureAfterDecrypt(SE_StatusTypeDef *pSeStatus, uint32_t DwlSlot,
                                                     SE_FwRawHeaderTypeDef *pFwImageHeader)
{
  SE_Ex_PayloadDescTypeDef payload_desc;
  uint32_t fw_size;
  uint32_t fw_offset;

  /*
   * The values below are not necessarily matching the way the firmware
   * has been spread in FLASH but this is adjusted later on in the this function.
   */
  payload_desc.pPayload[0] = SlotStartAdd[SLOT_SWAP];
  payload_desc.PayloadSize[0] = SLOT_SIZE(SLOT_SWAP);
  payload_desc.pPayload[1] = SlotStartAdd[DwlSlot];
  payload_desc.PayloadSize[1] = SLOT_SIZE(DwlSlot);

  fw_size = pFwImageHeader->PartialFwSize;
  fw_offset = (SFU_IMG_IMAGE_OFFSET + (pFwImageHeader->PartialFwOffset % SLOT_SIZE(SLOT_SWAP))) %
              SLOT_SIZE(SLOT_SWAP);

  /*
   * Adjusting the description of the way the Firmware is written in FLASH.
   */
  payload_desc.pPayload[0] = payload_desc.pPayload[0] + fw_offset;

  /* The first part contains the execution offset so the payload size must be adjusted accordingly */
  payload_desc.PayloadSize[0] = payload_desc.PayloadSize[0] - fw_offset;

  if (fw_size <= payload_desc.PayloadSize[0])
  {
    /* The firmware is written fully in a contiguous manner */
    payload_desc.PayloadSize[0] = fw_size;
    payload_desc.PayloadSize[1] = 0U;
    payload_desc.pPayload[1] = 0U;
  }
  else
  {
    /*
     * The firmware is too big to be contained in the first payload slot.
     * So, the firmware is split in 2 non-contiguous parts
     */

    if ((payload_desc.pPayload[1] == 0U)
        || (payload_desc.PayloadSize[1] < (fw_size - payload_desc.PayloadSize[0])))
    {
      return SFU_ERROR;
    }

    /* The second part contains the end of the firmware so the size is the total size - size already stored in the
       first area */
    payload_desc.PayloadSize[1] = fw_size - payload_desc.PayloadSize[0];

  }

  /* Signature Verification */
  return VerifyFwSignatureScatter(pSeStatus, DwlSlot, pFwImageHeader, &payload_desc, SE_FW_IMAGE_PARTIAL);
}


/**
  * @brief  Swap active slot with decrypted FW to install
  *         With the 2 images implementation, installing a new Firmware Image means swapping active slot and dwl slot.
  *         To perform this swap, the image to be installed is split in blocks of the swap size:
  *         SLOT_SIZE(DwlSlot) / SLOT_SIZE(SLOT_SWAP) blocks to be swapped .
  *         Each of these blocks is swapped using smaller chunks of SFU_IMG_CHUNK_SIZE size.
  *         The swap starts from the tail of the image and ends with the beginning of the image ("swap from tail to
  *         head").
  * @param  ActiveSlot identification of the active image
  * @param  DwlSlot identification of the downloaded area
  * @param  pFwImageHeader pointer in ram to the header to install
  * @retval SFU_SUCCESS if successful, a SFU_ErrorStatus error otherwise.
  */
#if defined(__ICCARM__)
#pragma inline=never
#endif
static SFU_ErrorStatus SwapFirmwareImages(uint32_t ActiveSlot, uint32_t DwlSlot, SE_FwRawHeaderTypeDef *pFwImageHeader)
{
  SFU_ErrorStatus e_ret_status = SFU_ERROR;
  SFU_FLASH_StatusTypeDef flash_if_status;
  SFU_LL_FLASH_write_t trailer;
  int32_t index_active_slot;
  int32_t index_dwl_slot_write;
  int32_t index_dwl_slot_read;
  int32_t chunk;
  uint8_t buffer[SFU_IMG_CHUNK_SIZE] __attribute__((aligned(8)));
  int32_t index_active_slot_partial_begin;
  int32_t index_active_slot_partial_end;
  int32_t index_active_slot_final_end;
  int32_t index_dwl_slot_partial_end;
  int32_t index_active_slot_empty_begin;
  /* number_of_index_active_slot is the number of blocks in active slot (block of SLOT_SIZE(SLOT_SWAP) bytes) */
  uint32_t number_of_index_active_slot = SLOT_SIZE(ActiveSlot) / SLOT_SIZE(SLOT_SWAP);
  /* number_of_index_dwl_slot is the number of blocks in dwl slot (block of SLOT_SIZE(SLOT_SWAP) bytes) */
  uint32_t number_of_index_dwl_slot = SLOT_SIZE(DwlSlot) / SLOT_SIZE(SLOT_SWAP);
  /* number_of_chunk is the number of chunks used to swap 1 block (moving a block of SLOT_SIZE(SLOT_SWAP) bytes
     split in number_of_chunk chunks of SFU_IMG_CHUNK_SIZE bytes) */
  uint32_t number_of_chunk = SLOT_SIZE(SLOT_SWAP) / SFU_IMG_CHUNK_SIZE;
  uint32_t write_len;
  uint32_t offset_block_partial_begin;
  uint32_t offset_block_partial_end;
  uint32_t offset_block_final_end;

  TRACE("\r\n\t  Image preparation done.\r\n\t  Swapping the firmware images");

  /* index_active_slot_partial_begin is the index of first block (of SLOT_SIZE(SLOT_SWAP) bytes) in active slot
     impacted by partial image */
  index_active_slot_partial_begin = (SFU_IMG_IMAGE_OFFSET + pFwImageHeader->PartialFwOffset) /
                                    SLOT_SIZE(SLOT_SWAP);

  /* offset_block_partial_begin is the offset of first byte of partial image inside index_active_slot_partial_begin
     block */
  offset_block_partial_begin = (SFU_IMG_IMAGE_OFFSET + pFwImageHeader->PartialFwOffset) %
                               SLOT_SIZE(SLOT_SWAP);

  /* In case of COMPLETE update, the full slots are swapped
     Assumption : DwlSlot is at least greater than ActiveSlot */
  if ((pFwImageHeader->FwSize == pFwImageHeader->PartialFwSize) && (SLOT_SIZE(DwlSlot) >= SLOT_SIZE(ActiveSlot)))
  {
    /* index_active_slot_partial_end is the index of block (of SLOT_SIZE(SLOT_SWAP) bytes) of first byte following
       partial image in active slot */
    index_active_slot_partial_end = (SLOT_SIZE(ActiveSlot) - TRAILER_SIZE(DwlSlot)) / SLOT_SIZE(SLOT_SWAP);

    /* offset_block_partial_end is the offset of first byte following partial image, inside
       index_active_slot_partial_end block */
    offset_block_partial_end = (SLOT_SIZE(ActiveSlot) - TRAILER_SIZE(DwlSlot)) % SLOT_SIZE(SLOT_SWAP);

    /* index_dwl_slot_partial_end is the index of block (of SLOT_SIZE(SLOT_SWAP) bytes) of first byte following
       partial image in dwl slot or swap area */
    index_dwl_slot_partial_end = ((SLOT_SIZE(ActiveSlot) - TRAILER_SIZE(DwlSlot)) / SLOT_SIZE(SLOT_SWAP)) - 1U;

    /* index_active_slot_final_end is the index of block (of SLOT_SIZE(SLOT_SWAP) bytes) of first byte following final
       image in active slot */
    index_active_slot_final_end = (SLOT_SIZE(ActiveSlot) - TRAILER_SIZE(DwlSlot)) / SLOT_SIZE(SLOT_SWAP);

    /* offset_block_final_end is the offset of first byte following final image, inside index_active_slot_final_end
       block */
    offset_block_final_end = (SLOT_SIZE(ActiveSlot) - TRAILER_SIZE(DwlSlot)) % SLOT_SIZE(SLOT_SWAP);

    /* set index_dwl_slot_write: starting from the end */
    index_dwl_slot_write = (SLOT_SIZE(ActiveSlot) - TRAILER_SIZE(DwlSlot)) / SLOT_SIZE(SLOT_SWAP);

  }
  else
  {
    /* index_active_slot_partial_end is the index of block (of SLOT_SIZE(SLOT_SWAP) bytes) of first byte following
       partial image in active slot */
    index_active_slot_partial_end = (SFU_IMG_IMAGE_OFFSET + pFwImageHeader->PartialFwOffset +
                                     pFwImageHeader->PartialFwSize) / SLOT_SIZE(SLOT_SWAP);

    /* offset_block_partial_end is the offset of first byte following partial image, inside
       index_active_slot_partial_end block */
    offset_block_partial_end = (SFU_IMG_IMAGE_OFFSET + pFwImageHeader->PartialFwOffset +
                                pFwImageHeader->PartialFwSize) % SLOT_SIZE(SLOT_SWAP);

    /* index_dwl_slot_partial_end is the index of block (of SLOT_SIZE(SLOT_SWAP) bytes) of first byte following
       partial image in dwl slot or swap area */
    index_dwl_slot_partial_end = (((SFU_IMG_IMAGE_OFFSET + (pFwImageHeader->PartialFwOffset %
                                                            SLOT_SIZE(SLOT_SWAP))) % SLOT_SIZE(SLOT_SWAP) +
                                   pFwImageHeader->PartialFwSize) / SLOT_SIZE(SLOT_SWAP)) - 1U;

    /* index_active_slot_final_end is the index of block (of SLOT_SIZE(SLOT_SWAP) bytes) of first byte following
       final image in active slot */
    index_active_slot_final_end = ((SFU_IMG_IMAGE_OFFSET + pFwImageHeader->FwSize) / SLOT_SIZE(SLOT_SWAP));

    /* offset_block_final_end is the offset of first byte following final image, inside index_active_slot_final_end
       block */
    offset_block_final_end = (SFU_IMG_IMAGE_OFFSET + pFwImageHeader->FwSize) % SLOT_SIZE(SLOT_SWAP);

    /* set index_dwl_slot_write: starting from the end, block index in dwl slot or swap area to receive block from
       active slot */
    if (index_active_slot_partial_end == (number_of_index_active_slot - 1))
    {
      /* last block of dwl slot can only receive last block of active slot, due to trailer presence */
      index_dwl_slot_write = number_of_index_dwl_slot - 1;
    }
    else
    {
      index_dwl_slot_write = number_of_index_dwl_slot - 2;
    }
  }

  /* set index_active_slot: starting from the end, block index from active slot to copy in dwl slot */
  index_active_slot = index_active_slot_partial_end;

  /* set index_dwl_slot_read: starting from the end, block index in dwl slot or swap area to copy updated image to
     active slot */
  index_dwl_slot_read = index_dwl_slot_partial_end;

  /* Adjust indexes in case offset is 0: in this case, first block to swap is the previous one */
  if (offset_block_partial_end == 0)
  {
    index_active_slot--;
    index_dwl_slot_read--;
  }

  /*
   * dwl slot read index should be lower than dwl slot write index.
   * If not the case, the dwl slot is not big enough to manage this partial firmware image
   * (depends on offset and size).
   */
  if (index_dwl_slot_read >= index_dwl_slot_write)
  {
    return SFU_ERROR;
  }

  /* Swap one block at each loop, until all targeted blocks of active slot have been copied
   * Concerned blocks are:
   * 1- block impacted by partial image
   * 2- first block containing the header
   */
  while (index_active_slot >= 0)
  {
    /* Reload watchdoag every SWAP size */
    SFU_LL_SECU_IWDG_Refresh();

    TRACE(".");

    if ((index_dwl_slot_write < -1) || (index_dwl_slot_read < -1))
    {
      return SFU_ERROR;
    }

    /* If CPY_TO_DWL_SLOT(i) is still virgin, then swap the block from active slot to dwl slot */
    e_ret_status = SFU_LL_FLASH_Read((uint8_t *) &trailer,
                                     TRAILER_CPY_TO_DWL_SLOT(DwlSlot, TRAILER_INDEX(DwlSlot) - 1 - index_active_slot),
                                     sizeof(trailer));
    if ((e_ret_status == SFU_SUCCESS) && (memcmp(&trailer, NOT_SWAPPED, sizeof(trailer)) == 0))
    {
      /* Erase destination block in dwl slot or swap, if not the trailer block */
      if (index_dwl_slot_write != (number_of_index_dwl_slot - 1))
      {
        if (index_dwl_slot_write == -1)
        {
          /* Erase the swap */
          e_ret_status = SFU_LL_FLASH_Erase_Size(&flash_if_status, (uint8_t *) SlotStartAdd[SLOT_SWAP],
                                                 SLOT_SIZE(SLOT_SWAP));
          STATUS_FWIMG(e_ret_status == SFU_ERROR, SFU_IMG_FLASH_ERASE_FAILED)
        }
        else /* index_dwl_slot_write >= 0 */
        {
          /* erase the size of "swap area" at @: Dwl slot + index_dwl_slot_write*swap_area_size */
          e_ret_status = EraseSlotIndex(DwlSlot, index_dwl_slot_write);
        }
        if (e_ret_status !=  SFU_SUCCESS)
        {
          return SFU_ERROR;
        }
      }

      /* Copy the block from active slot to dwl slot or swap (using "number_of_chunk" chunks) */
      for (chunk = (number_of_chunk - 1); chunk >= 0 ; chunk--)
      {
        /* ignore return value,  no double ecc error is expected, area already read before */
        (void)SFU_LL_FLASH_Read(buffer, CHUNK_0_ADDR_HEADER(ActiveSlot, index_active_slot, chunk), sizeof(buffer));
        write_len = sizeof(buffer);
        if (index_dwl_slot_write == -1)
        {
          /* Destination block is the swap */
          e_ret_status = SFU_LL_FLASH_Write(&flash_if_status, CHUNK_SWAP_ADDR(chunk), buffer, write_len);
          STATUS_FWIMG(e_ret_status == SFU_ERROR, SFU_IMG_FLASH_WRITE_FAILED);
          if (e_ret_status != SFU_SUCCESS)
          {
            return SFU_ERROR;
          }
        }
        else /* index_dwl_slot_write >= 0 */
        {
          /* Destination block is in dwl slot: Do not overwrite the trailer. */
          if (((uint32_t)CHUNK_1_ADDR(DwlSlot, index_dwl_slot_write, chunk)) < (uint32_t)TRAILER_BEGIN(DwlSlot))
          {
            /*  write is possible length can be modified  */
            if ((uint32_t)(CHUNK_1_ADDR(DwlSlot, index_dwl_slot_write, chunk) + write_len) >
                (uint32_t)TRAILER_BEGIN(DwlSlot))
            {
              write_len = TRAILER_BEGIN(DwlSlot) - CHUNK_1_ADDR(DwlSlot, index_dwl_slot_write, chunk);
            }
            e_ret_status = SFU_LL_FLASH_Write(&flash_if_status, CHUNK_1_ADDR(DwlSlot, index_dwl_slot_write, chunk),
                                              buffer, write_len);
            STATUS_FWIMG(e_ret_status == SFU_ERROR, SFU_IMG_FLASH_WRITE_FAILED);
            if (e_ret_status != SFU_SUCCESS)
            {
              return SFU_ERROR;
            }
          }
        }
      }

      /*
       * The block of the active firmware has been backed up.
       * The trailer is updated to memorize this: the CPY bytes at the appropriate index are set to SWAPPED.
       */
      e_ret_status  = AtomicWrite(TRAILER_CPY_TO_DWL_SLOT(DwlSlot, (TRAILER_INDEX(DwlSlot) - 1 - index_active_slot)),
                                  (SFU_LL_FLASH_write_t *) SWAPPED);

      STATUS_FWIMG(e_ret_status == SFU_ERROR, SFU_IMG_FLASH_WRITE_FAILED);
      if (e_ret_status != SFU_SUCCESS)
      {
        return SFU_ERROR;
      }
    }

    /* If CPY_TO_ACTIVE_SLOT(i) is still virgin, then swap the block from dwl slot to active slot */
    e_ret_status = SFU_LL_FLASH_Read((uint8_t *) &trailer,
                                     TRAILER_CPY_TO_ACTIVE_SLOT(DwlSlot, TRAILER_INDEX(DwlSlot) - 1 -
                                                                index_active_slot),
                                     sizeof(trailer));
    if ((e_ret_status == SFU_SUCCESS) && (memcmp(&trailer, NOT_SWAPPED, sizeof(trailer)) == 0))
    {
      /* erase the size of "swap area" at @: active slot + index_active_slot*swap_area_size*/
      e_ret_status = EraseSlotIndex(ActiveSlot, index_active_slot);

      if (e_ret_status !=  SFU_SUCCESS)
      {
        return SFU_ERROR;
      }

      /*
       * Fill block in active slot:
       * The appropriate update image block of dwl slot together with initial image block that has been backed up in
       * dwl slot,
       * are associated to constitute final block in active slot (installing the block of the new firmware image).
       */
      for (chunk = (number_of_chunk - 1); chunk >= 0 ; chunk--)
      {
        /* Read complete chunk of updated image */
        if (index_dwl_slot_read == -1)
        {
          /* ignore return value, no double ecc error is expected, area already read before */
          (void)SFU_LL_FLASH_Read(buffer, CHUNK_SWAP_ADDR(chunk), sizeof(buffer));
        }
        else /* index_dwl_slot_read >= 0 */
        {
          /* ignore return value, no double ecc error is expected, area already read before */
          (void)SFU_LL_FLASH_Read(buffer, CHUNK_1_ADDR(DwlSlot, index_dwl_slot_read, chunk), sizeof(buffer));
        }

        /* Last impacted block: end of block has to be updated with initial image */
        if (index_active_slot == index_active_slot_partial_end)
        {
          /* If chunk is not impacted by updated image, chunk fully takes initial image content */
          if (CHUNK_1_ADDR(DwlSlot, index_dwl_slot_write, chunk) >= (CHUNK_1_ADDR(DwlSlot, index_dwl_slot_write,
                                                                                  0) + offset_block_partial_end))
          {
            /* check return value, as this area has not been read before */
            e_ret_status = SFU_LL_FLASH_Read(buffer, CHUNK_1_ADDR(DwlSlot, index_dwl_slot_write, chunk),
                                             sizeof(buffer));
            STATUS_FWIMG(e_ret_status == SFU_ERROR, SFU_IMG_FLASH_READ_FAILED);
            if (e_ret_status != SFU_SUCCESS)
            {
              return SFU_ERROR;
            }
          }

          /* If chunk is partially impacted by updated image, end of chunk is updated with the initial image content */
          if ((CHUNK_1_ADDR(DwlSlot, index_dwl_slot_write, chunk) < (CHUNK_1_ADDR(DwlSlot, index_dwl_slot_write,
                                                                                  0) + offset_block_partial_end))
              && ((CHUNK_1_ADDR(DwlSlot, index_dwl_slot_write, (chunk + 1))) >
                  (CHUNK_1_ADDR(DwlSlot, index_dwl_slot_write, 0) + offset_block_partial_end)))
          {
            /* check return value, as this area has not been read before */
            e_ret_status = SFU_LL_FLASH_Read(buffer + (offset_block_partial_end % SFU_IMG_CHUNK_SIZE),
                                             CHUNK_1_ADDR(DwlSlot, index_dwl_slot_write, chunk) +
                                             (offset_block_partial_end % SFU_IMG_CHUNK_SIZE),
                                             sizeof(buffer) - (offset_block_partial_end % SFU_IMG_CHUNK_SIZE));
            STATUS_FWIMG(e_ret_status == SFU_ERROR, SFU_IMG_FLASH_READ_FAILED);
            if (e_ret_status != SFU_SUCCESS)
            {
              return SFU_ERROR;
            }
          }
        }

        /* First impacted block: beginning of block has to be updated with initial image */
        if (index_active_slot == index_active_slot_partial_begin)
        {
          /* If chunk is not impacted by updated image, chunk fully takes initial image content */
          if (CHUNK_1_ADDR(DwlSlot, index_dwl_slot_write, (chunk + 1)) <=
              (CHUNK_1_ADDR(DwlSlot, index_dwl_slot_write, 0) + offset_block_partial_begin))
          {
            /* check return value, as this area has not been read before */
            e_ret_status = SFU_LL_FLASH_Read(buffer, CHUNK_1_ADDR(DwlSlot, index_dwl_slot_write, chunk),
                                             sizeof(buffer));
            STATUS_FWIMG(e_ret_status == SFU_ERROR, SFU_IMG_FLASH_READ_FAILED);
            if (e_ret_status != SFU_SUCCESS)
            {
              return SFU_ERROR;
            }
          }

          /* If chunk is partially impacted by updated image, beginning of chunk is updated with the initial image
             content */
          if ((CHUNK_1_ADDR(DwlSlot, index_dwl_slot_write, chunk) <
               (CHUNK_1_ADDR(DwlSlot, index_dwl_slot_write, 0) + offset_block_partial_begin)) &&
              (CHUNK_1_ADDR(DwlSlot, index_dwl_slot_write, (chunk + 1)) >
               (CHUNK_1_ADDR(DwlSlot, index_dwl_slot_write, 0) + offset_block_partial_begin)))
          {
            /* check return value, as this area has not been read before */
            e_ret_status = SFU_LL_FLASH_Read(buffer, CHUNK_1_ADDR(DwlSlot, index_dwl_slot_write, chunk),
                                             offset_block_partial_begin % SFU_IMG_CHUNK_SIZE);
            STATUS_FWIMG(e_ret_status == SFU_ERROR, SFU_IMG_FLASH_READ_FAILED);
            if (e_ret_status != SFU_SUCCESS)
            {
              return SFU_ERROR;
            }
          }
        }
        /* Header block not impacted by partial image: complete block, except header, is updated with initial image */
        else if (index_active_slot == 0)
        {
          /* check return value, as this area has not been read before */
          e_ret_status = SFU_LL_FLASH_Read(buffer, CHUNK_1_ADDR(DwlSlot, index_dwl_slot_write, chunk), sizeof(buffer));
          STATUS_FWIMG(e_ret_status == SFU_ERROR, SFU_IMG_FLASH_READ_FAILED);
          if (e_ret_status != SFU_SUCCESS)
          {
            return SFU_ERROR;
          }
        }

        /* Last block of final image: end of block has to be cleaned with empty data */
        if (index_active_slot == index_active_slot_final_end)
        {
          /* If chunk is completely beyond the final image, then chunk takes empty data */
          if (CHUNK_0_ADDR(ActiveSlot, index_active_slot, chunk) >=
              CHUNK_0_ADDR(ActiveSlot, index_active_slot_final_end, 0) + offset_block_final_end)
          {
            memset(buffer, 0xFF, sizeof(buffer));
          }

          /* If chunk is partially beyond the final image, then chunk is partially updated with empty data */
          if ((CHUNK_0_ADDR(ActiveSlot, index_active_slot, chunk) <
               (CHUNK_0_ADDR(ActiveSlot, index_active_slot_final_end, 0) + offset_block_final_end)) &&
              (CHUNK_0_ADDR(ActiveSlot, index_active_slot, (chunk + 1)) >
               (CHUNK_0_ADDR(ActiveSlot, index_active_slot_final_end, 0) + offset_block_final_end)))
          {
            memset(buffer + (offset_block_final_end % SFU_IMG_CHUNK_SIZE), 0xFF,
                   sizeof(buffer) - (offset_block_final_end % SFU_IMG_CHUNK_SIZE));
          }
        }

        write_len = SFU_IMG_CHUNK_SIZE;
        /*  don't copy header in active slot
            this will be done at the end during validation step */
        if ((index_active_slot == 0) && (chunk == 0))
        {
          write_len = write_len - SFU_IMG_IMAGE_OFFSET;
        }
        e_ret_status = SFU_LL_FLASH_Write(&flash_if_status, CHUNK_0_ADDR_MODIFIED(ActiveSlot, index_active_slot, chunk),
                                          buffer, write_len);
        STATUS_FWIMG(e_ret_status == SFU_ERROR, SFU_IMG_FLASH_WRITE_FAILED);
        if (e_ret_status != SFU_SUCCESS)
        {
          return SFU_ERROR;
        }
      }

      /*
       * The block of the active firmware has been backed up.
       * The trailer is updated to memorize this: the CPY bytes at the appropriate index are set to SWAPPED.
       * Except for block 0, because block 0 is incomplete at this stage (final header installation not yet done)
       */
      if (index_active_slot != 0)
      {
        e_ret_status  = AtomicWrite(TRAILER_CPY_TO_ACTIVE_SLOT(DwlSlot,
                                                               (TRAILER_INDEX(DwlSlot) - 1 - index_active_slot)),
                                    (SFU_LL_FLASH_write_t *) SWAPPED);

        STATUS_FWIMG(e_ret_status == SFU_ERROR, SFU_IMG_FLASH_WRITE_FAILED);
        if (e_ret_status != SFU_SUCCESS)
        {
          return SFU_ERROR;
        }
      }
    }

    /* Decrement all block indexes */
    index_dwl_slot_write--;
    index_dwl_slot_read--;
    index_active_slot--;

    /*
     * After having copied the impacted image blocks from active slot, copy also the first block
     * due to header, if not already copied among the impacted image blocks.
     */
    if ((index_active_slot < index_active_slot_partial_begin) && (index_active_slot >= 0))
    {
      index_active_slot = 0;
      index_dwl_slot_read = -1;  /* force read from swap area, to avoid exiting dwl slot / swap index allowed range */
    }
  }

  /*
   * Now, erase the blocks in active slot, located after final image
   */
  if (offset_block_final_end == 0)
  {
    index_active_slot_empty_begin = index_active_slot_final_end;
  }
  else
  {
    index_active_slot_empty_begin = index_active_slot_final_end + 1;
  }
  index_active_slot = number_of_index_active_slot - 1;
  while (index_active_slot >= index_active_slot_empty_begin)
  {
    e_ret_status = EraseSlotIndex(ActiveSlot, index_active_slot);
    if (e_ret_status !=  SFU_SUCCESS)
    {
      return SFU_ERROR;
    }

    /* Decrement block index */
    index_active_slot--;
  }

  return e_ret_status;
}


/**
  * @brief Decrypt Image in dwl slot
  * @ note Decrypt is done from dwl slot to dwl slot + swap with 2 images (swap contains 1st sector)
  * @param  DwlSlot identification of the downloaded area
  * @param  pFwImageHeader pointer in ram to the header to decrypt
  * @retval SFU_SUCCESS if successful, a SFU_ErrorStatus error otherwise.
  */
#if defined(__ICCARM__)
#pragma inline=never
#endif
static SFU_ErrorStatus DecryptImageInDwlSlot(uint32_t DwlSlot, SE_FwRawHeaderTypeDef *pFwImageHeader)
{
  SFU_ErrorStatus  e_ret_status = SFU_ERROR;
  SE_StatusTypeDef e_se_status;
  SE_ErrorStatus   se_ret_status;
  uint32_t NumberOfChunkPerSwap = SLOT_SIZE(SLOT_SWAP) / SFU_IMG_CHUNK_SIZE;
  SFU_FLASH_StatusTypeDef flash_if_status;
  /*  chunk size is the maximum , the 1st block can be smaller */
  /*  the chunk is static to avoid  large stack */
  uint8_t fw_decrypted_chunk[SFU_IMG_CHUNK_SIZE] __attribute__((aligned(8)));
  uint8_t fw_encrypted_chunk[SFU_IMG_CHUNK_SIZE] __attribute__((aligned(8)));
  uint32_t fw_source_address = 0U;
  uint32_t fw_dest_address_write = 0U;
  uint32_t fw_dest_erase_address = 0U;
  uint32_t fw_decrypted_total_size = 0U;
  uint32_t size;
  uint32_t oldsize;
  uint32_t fw_decrypted_chunk_size;
  uint32_t fw_tag_len;
  uint8_t fw_tag_output[SE_TAG_LEN];
  uint32_t pass_index = 0U;
  uint32_t erase_index = 0U;

  if ((pFwImageHeader == NULL))
  {
    return e_ret_status;
  }

  /* Decryption process*/
  se_ret_status = SE_Decrypt_Init(&e_se_status, pFwImageHeader, SE_FW_IMAGE_PARTIAL);
  if ((se_ret_status == SE_SUCCESS) && (e_se_status == SE_OK))
  {
    e_ret_status = SFU_SUCCESS;

    /* Decryption loop*/
    while ((e_ret_status == SFU_SUCCESS) && (fw_decrypted_total_size < (pFwImageHeader->PartialFwSize)) &&
           (e_se_status == SE_OK))
    {
      if (pass_index == NumberOfChunkPerSwap)
      {
        fw_dest_address_write = SlotStartAdd[DwlSlot];
        fw_dest_erase_address =  fw_dest_address_write;
        erase_index = NumberOfChunkPerSwap;
      }
      if (pass_index == 0U)
      {
        fw_source_address = SlotStartAdd[DwlSlot] + SFU_IMG_IMAGE_OFFSET +
                            (pFwImageHeader->PartialFwOffset % SLOT_SIZE(SLOT_SWAP));
        fw_dest_erase_address = SlotStartAdd[SLOT_SWAP];
        fw_dest_address_write = fw_dest_erase_address + ((SFU_IMG_IMAGE_OFFSET + (pFwImageHeader->PartialFwOffset %
                                                                                  SLOT_SIZE(SLOT_SWAP))) %
                                                         SLOT_SIZE(SLOT_SWAP));
        fw_decrypted_chunk_size = sizeof(fw_decrypted_chunk) -
                                  ((SFU_IMG_IMAGE_OFFSET + (pFwImageHeader->PartialFwOffset % SLOT_SIZE(SLOT_SWAP))) %
                                   sizeof(fw_decrypted_chunk));
        if (fw_decrypted_chunk_size > pFwImageHeader->PartialFwSize)
        {
          fw_decrypted_chunk_size = pFwImageHeader->PartialFwSize;
        }
        pass_index = (((SFU_IMG_IMAGE_OFFSET + (pFwImageHeader->PartialFwOffset % SLOT_SIZE(SLOT_SWAP)))
                       % SLOT_SIZE(SLOT_SWAP)) / sizeof(fw_decrypted_chunk));
      }
      else
      {
        fw_decrypted_chunk_size = sizeof(fw_decrypted_chunk);

        /* For the last 2 pass, divide by 2 remaining buffer to ensure that :
         *     - chunk size greater than 16 bytes : minimum size of a block to be decrypted
         *     - except last one chunk size is 16 bytes aligned
         *
         * Pass n - 1 : remaining bytes / 2 with (16 bytes alignment for crypto AND sizeof(SFU_LL_FLASH_write_t) for
         *              flash programming)
         * Pass n : remaining bytes
         */

        /* Last pass : n */
        if ((pFwImageHeader->PartialFwSize - fw_decrypted_total_size) < fw_decrypted_chunk_size)
        {
          fw_decrypted_chunk_size = pFwImageHeader->PartialFwSize - fw_decrypted_total_size;
        }
        /* Previous pass : n - 1 */
        else if ((pFwImageHeader->PartialFwSize - fw_decrypted_total_size) < ((2U * fw_decrypted_chunk_size) - 16U))
        {
          fw_decrypted_chunk_size = ((pFwImageHeader->PartialFwSize - fw_decrypted_total_size) / 32U) * 16U;

          /* Set dimension to the appropriate length for FLASH programming.
           * Example: 64-bit length for L4.
           */
          if ((fw_decrypted_chunk_size & ((uint32_t)sizeof(SFU_LL_FLASH_write_t) - 1U)) != 0U)
          {
            fw_decrypted_chunk_size = fw_decrypted_chunk_size + ((uint32_t)sizeof(SFU_LL_FLASH_write_t) -
                                                                 (fw_decrypted_chunk_size %
                                                                  (uint32_t)sizeof(SFU_LL_FLASH_write_t)));
          }
        }
        /* All others pass */
        else
        {
          /* nothing */
        }
      }

      size = fw_decrypted_chunk_size;

      /* Decrypt Append*/
      e_ret_status = SFU_LL_FLASH_Read(fw_encrypted_chunk, (uint8_t *) fw_source_address, size);
      if (e_ret_status != SFU_SUCCESS)
      {
        break;
      }
      if (size != 0U)
      {
        se_ret_status = SE_Decrypt_Append(&e_se_status, (uint8_t *)fw_encrypted_chunk, (int32_t)size,
                                          fw_decrypted_chunk, (int32_t *)&fw_decrypted_chunk_size);
      }
      else
      {
        e_ret_status = SFU_SUCCESS;
        fw_decrypted_chunk_size = 0U;
      }
      if ((se_ret_status == SE_SUCCESS) && (e_se_status == SE_OK) && (fw_decrypted_chunk_size == size))
      {
        /* Erase Page */
        if ((pass_index == erase_index)
            || (pass_index == ((SFU_IMG_IMAGE_OFFSET + (pFwImageHeader->PartialFwOffset % SLOT_SIZE(SLOT_SWAP))) /
                               sizeof(fw_decrypted_chunk))))
        {
          (void) SFU_LL_SECU_IWDG_Refresh();
          e_ret_status = SFU_LL_FLASH_Erase_Size(&flash_if_status, (uint8_t *)fw_dest_erase_address,
                                                 SLOT_SIZE(SLOT_SWAP)) ;
          erase_index += NumberOfChunkPerSwap;
          fw_dest_erase_address += SLOT_SIZE(SLOT_SWAP);
        }
        STATUS_FWIMG(e_ret_status == SFU_ERROR, SFU_IMG_FLASH_ERASE_FAILED);

        if (e_ret_status == SFU_SUCCESS)
        {
          /*
           * For last pass with remaining size not aligned on 16 bytes : Set dimension AFTER decrypt to the appropriate
           * length for FLASH programming.
           * Example: 64-bit length for L4.
           */
          if ((size & ((uint32_t)sizeof(SFU_LL_FLASH_write_t) - 1U)) != 0U)
          {
            /*
            * By construction, SFU_IMG_CHUNK_SIZE is a multiple of sizeof(SFU_LL_FLASH_write_t) so there is no risk to
             * read out of the buffer
             */
            oldsize = size;
            size = size + ((uint32_t)sizeof(SFU_LL_FLASH_write_t) - (size % (uint32_t)sizeof(SFU_LL_FLASH_write_t)));
            while (oldsize < size)
            {
              fw_decrypted_chunk[oldsize] = 0xFF;
              oldsize++;
            }
          }

          /* Write Decrypted Data in Flash - size has to be 32-bit aligned */
          e_ret_status = SFU_LL_FLASH_Write(&flash_if_status, (uint8_t *)fw_dest_address_write,  fw_decrypted_chunk,
                                            size);
          STATUS_FWIMG(e_ret_status == SFU_ERROR, SFU_IMG_FLASH_WRITE_FAILED);

          if (e_ret_status == SFU_SUCCESS)
          {
            /* Update flash pointer */
            fw_dest_address_write  += (size);

            /* Update source pointer */
            fw_source_address += size;
            fw_decrypted_total_size += size;
            (void) memset(fw_decrypted_chunk, 0xff, sizeof(fw_decrypted_chunk));
            pass_index += 1U;

          }
        }
      }
    }
  }

#if (SFU_IMAGE_PROGRAMMING_TYPE == SFU_ENCRYPTED_IMAGE)
#if defined(SFU_VERBOSE_DEBUG_MODE)
  TRACE("\r\n\t  %d bytes of ciphertext decrypted.", fw_decrypted_total_size);
#endif /* SFU_VERBOSE_DEBUG_MODE */
#endif /* SFU_ENCRYPTED_IMAGE */

  if ((se_ret_status == SE_SUCCESS) && (e_ret_status == SFU_SUCCESS) && (e_se_status == SE_OK))
  {
    /* Do the Finalization, check the authentication TAG*/
    fw_tag_len = sizeof(fw_tag_output);
    se_ret_status = SE_Decrypt_Finish(&e_se_status, fw_tag_output, (int32_t *)&fw_tag_len);
    if ((se_ret_status != SE_SUCCESS) || (e_se_status != SE_OK))
    {
      e_ret_status = SFU_ERROR;
#if defined(SFU_VERBOSE_DEBUG_MODE)
      TRACE("\r\n\t  Decrypt fails at Finalization stage.");
#endif /* SFU_VERBOSE_DEBUG_MODE */
    }
    else
    {
      /* erase the remaining part of the image after shifting inside dwl slot */
      if (pass_index <= NumberOfChunkPerSwap)
      {
        fw_dest_erase_address = SlotStartAdd[DwlSlot];
      }
      e_ret_status = SFU_LL_FLASH_Erase_Size(&flash_if_status, (uint8_t *)fw_dest_erase_address, SLOT_SIZE(SLOT_SWAP));
      STATUS_FWIMG(e_ret_status == SFU_ERROR, SFU_IMG_FLASH_ERASE_FAILED);
    }
  }
  else
  {
    e_ret_status = SFU_ERROR;
  }
  return e_ret_status;
}


/**
  * @brief  check trailer validity to allow resume installation.
  * @param  ActiveSlot identification of the active image
  * @param  DwlSlot identification of the downloaded area
  * @param  pValidHeader: pointer in ram to header of valid fw to backup
  * @param  pTestHeader: pointer in ram to header of fw to test
  * @retval SFU_SUCCESS if successful, a SFU_ErrorStatus error otherwise.
  */
#if defined(__ICCARM__)
#pragma inline=never
#endif
static SFU_ErrorStatus  FirmwareToResume(uint32_t ActiveSlot, uint32_t DwlSlot, SE_FwRawHeaderTypeDef *pValidHeader,
                                          SE_FwRawHeaderTypeDef *pTestHeader)
{
  SFU_ErrorStatus e_ret_status = SFU_ERROR;
  uint8_t fw_header_trailer_valid[SE_FW_HEADER_TOT_LEN];
  uint8_t fw_header_trailer_test[SE_FW_HEADER_TOT_LEN];
  uint8_t fw_header_active_slot[SE_FW_HEADER_TOT_LEN];

  /*
   *  Initial conditions to be fulfilled :
   *   - Trailer magic is present
   *   - Active slot number is correct
   *   - test header from trailer is signed
   */

  /* check trailer Magic */
  //다운로드 슬롯 트레일러를 점검한다.(설치를 위해서 트레일러에 값이 정상적으로 써 있고 clean 값은 써있지 않은 상태)
  e_ret_status = CheckTrailerMagic(DwlSlot);
  if (e_ret_status == SFU_SUCCESS)
  {
    /* Populate the pTestHeader and verify it the active slot is correct */
    //다운로드 트레일러의 test 헤더를 읽어온다.
    e_ret_status = SFU_LL_FLASH_Read(fw_header_trailer_test, TRAILER_HDR_TEST(DwlSlot), sizeof(fw_header_trailer_test));
    STATUS_FWIMG(e_ret_status == SFU_ERROR, SFU_IMG_FLASH_READ_FAILED);
  }

  if (e_ret_status == SFU_SUCCESS)
  {
    /* Populate pTestHeader parameter */
    //다운로드 트레일러의 test 헤더를 읽어온다. (test 헤더는 새로 설치하는 FW 의 헤더이다.)
    e_ret_status = ParseFWInfo(pTestHeader, fw_header_trailer_test);
  }

  if (e_ret_status == SFU_SUCCESS)
  {
    /* Active slot number correct ?
       As example : if magic is SFU2 active slot should be SLOT_ACTIVE_2 */
    //테스트 헤더의 magic 값을 확인해서 슬롯 번호가 액티브 슬롯의 번호와 일치하는지 확인한다.
    if (ActiveSlot == SFU_IMG_GetFwImageSlot(pTestHeader))
    {
      /* Check header test field in trailer is signed */
      //테스트헤더가 정상적으로 signing 되어 있는지 확인한다.
      e_ret_status = VerifyHeaderSignature((SE_FwRawHeaderTypeDef *)(uint32_t)fw_header_trailer_test);
    }
    else
    {
#if defined(SFU_VERBOSE_DEBUG_MODE)
      TRACE("\r\n\t  No resume required : TRAILER_HDR_TEST not valid!");
#endif /* SFU_VERBOSE_DEBUG_MODE */
      e_ret_status = SFU_ERROR;
    }
  }

  if (e_ret_status != SFU_SUCCESS)
  {
    return SFU_ERROR;
  }

  /* Read header in active slot if any */
  //현재 액티브 슬롯의 헤더를 읽어온다.
  e_ret_status = SFU_LL_FLASH_Read(fw_header_active_slot, (uint8_t *) SlotHeaderAdd[ActiveSlot],
                                   sizeof(fw_header_active_slot));
  STATUS_FWIMG(e_ret_status == SFU_ERROR, SFU_IMG_FLASH_READ_FAILED);

  if (e_ret_status == SFU_SUCCESS)
  {
    /* Verify the header signature in active slot */
    //현재 액티브 슬롯의 헤더가 정상적으로 signing 되어 있는지 확인한다.
    e_ret_status = VerifyHeaderSignature((SE_FwRawHeaderTypeDef *)(uint32_t)fw_header_active_slot);
  }
  if (e_ret_status == SFU_SUCCESS)
  {
    /* Header in active slot is signed and validated, in protected area: it can be trusted. */

    /* Populate pValidHeader parameter */
    //현재 액티브 헤더의 FW 정보를 확인한다.
    e_ret_status = ParseFWInfo(pValidHeader, fw_header_active_slot);

    if (e_ret_status == SFU_SUCCESS)
    {
      /* Read header valid field in trailer */
      //다운로드 트레일러의 valid 헤더 정보를 읽어온다. 트레일러의 valid 헤더 정보는 다운로드 시 액티브 슬롯의 헤더 정보이다.
      //따라서 정상적으로 설치중에 중단된 상태라면 현재 액티브 헤더와 트레일러의 valid 헤더는 동일해야 한다.
      e_ret_status = SFU_LL_FLASH_Read(fw_header_trailer_valid, TRAILER_HDR_VALID(DwlSlot),
                                       sizeof(fw_header_trailer_valid));
      STATUS_FWIMG(e_ret_status == SFU_ERROR, SFU_IMG_FLASH_READ_FAILED);
    }

    if (e_ret_status == SFU_SUCCESS)
    {
      /* Check if header in active slot is same as header valid in trailer */
      //트레일러의 valid 헤더와 현재 액티브 슬롯의 헤더가 동일한지 확인한다.
      if (memcmp(fw_header_active_slot, fw_header_trailer_valid, SE_FW_AUTH_LEN) != 0)
      {
        /*
         * Header in active slot is not identical to header valid field:
         * Either install is already complete (if header in active slot is same as header test field in trailer),
         * or there is hack tentative.
         * In both cases, resume install must not be triggered.
         */
#if defined(SFU_VERBOSE_DEBUG_MODE)
        TRACE("\r\n\t  No resume required : TRAILER_HDR_VALID already stored in active slot!");
#endif /* SFU_VERBOSE_DEBUG_MODE */
        e_ret_status = SFU_ERROR;
      }
      else
      {
        /* Check candidate image version */
        //설치할 test 헤더의 FW 버전과 valid 헤더의 FW 버전을 비교하여 버전이 동일하거나 올라가는지 확인한다.
        //코드에는 valid 헤더 버전을 비교하고 있지만 사실 상 액티브 헤더와 앞서 동일함을 확인하였으므로
        //사실상 액티브 헤더의 버전과 test 헤더의 버전을 확인하는 것과 같다.
        e_ret_status = SFU_IMG_CheckFwVersion(ActiveSlot, pValidHeader->FwVersion, pTestHeader->FwVersion);
      }
    }
    /* At this point, if (e_ret_status == SFU_SUCCESS) then resume of installation is allowed */
  }
  else
  {
    /*
     * Header in active slot is not valid.
     * It is considered this can not be hack attempt, because header in active slot is in
     * protected area.
     * Possible reasons:
     * - Swap has been interrupted during very first image installation (active slot was empty)
     * - Swap has been interrupted during reconstitution of first sector of active slot
     * If header test field in trailer is signed and with proper version, we resume install.
     */

    //액티브 헤더가 정상적이지 않은 경우

    /* Read header valid field in trailer */
    //다운로드 트레일러의 valid 헤더를 읽어온다.
    e_ret_status = SFU_LL_FLASH_Read(fw_header_trailer_valid, TRAILER_HDR_VALID(DwlSlot),
                                     sizeof(fw_header_trailer_valid));
    STATUS_FWIMG(e_ret_status == SFU_ERROR, SFU_IMG_FLASH_READ_FAILED);
    if (e_ret_status == SFU_SUCCESS)
    {
      /* Check header valid field in trailer is signed */
      //valid 헤더의 signature 를 검증한다.
      e_ret_status = VerifyHeaderSignature((SE_FwRawHeaderTypeDef *)(uint32_t)fw_header_trailer_valid);

      if (e_ret_status == SFU_SUCCESS)
      {
        /* active slot was containing an active image */

        /* Populate pValidHeader parameter */
        //valid 헤더의 FW 정보를 읽어온다.
        e_ret_status = ParseFWInfo(pValidHeader, fw_header_trailer_valid);

        if (e_ret_status == SFU_SUCCESS)
        {
          /* Check candidate image version */
          //액티브 헤더가 비정상이므로 valid 헤더의 FW 버전과 test 헤더의 FW 버전을 비교하여 설치 가능한지 확인한다.
          e_ret_status = SFU_IMG_CheckFwVersion(ActiveSlot, pValidHeader->FwVersion, pTestHeader->FwVersion);
        }
      }
      else
      {
        /* active slot was empty or was containing bricked Fw image */
        e_ret_status = SFU_SUCCESS;
      }
    }

    /* At this point, if (e_ret_status == SFU_SUCCESS) then resume of installation is allowed */
  }

  return e_ret_status;
}

/**
  * @brief  Check that there is an Image to Install
  * @param  DwlSlot identification of the downloaded area
  * @param  pFwImageHeader pointer to fw header to install
  * @retval SFU_SUCCESS if Image can be installed, a SFU_ERROR  otherwise.
  */
static SFU_ErrorStatus FirmwareToInstall(uint32_t DwlSlot, SE_FwRawHeaderTypeDef *pFwImageHeader)
{
  SFU_ErrorStatus e_ret_status = SFU_ERROR;
  uint8_t *p_header_swap;
  uint8_t fw_header_dwl_slot[SE_FW_HEADER_TOT_LEN];
  uint32_t trailer_begin = (uint32_t) TRAILER_BEGIN(DwlSlot);
  uint32_t end_of_test_image;
  int32_t ret;
  /*
     * The anti-rollback check is implemented at installation stage (SFU_IMG_InstallNewVersion)
     * to be able to handle a specific error cause.
     */

  /*  check swap header */
  //swap 영역에 올바른 헤더 정보가 있는지 확인한다.
  e_ret_status = CheckAndGetFWHeader(SLOT_SWAP, pFwImageHeader);
  if (e_ret_status == SFU_SUCCESS)
  {
    /*  compare the header in dwl slot with the header in swap */
    // 다운로드 영역의 헤더정보를 읽는다.
    p_header_swap = (uint8_t *) pFwImageHeader;
    e_ret_status = SFU_LL_FLASH_Read(fw_header_dwl_slot, (uint8_t *) SlotStartAdd[DwlSlot],
                                     sizeof(fw_header_dwl_slot));
    if (e_ret_status == SFU_SUCCESS)
    {
      /* image header in dwl slot not consistent with swap header */
      //이미지 끝의 주소를 계산한다. (다운로드 슬롯의 시작 주소 + 이미지 offset + FW 크기)
      end_of_test_image = (SlotStartAdd[DwlSlot] + pFwImageHeader->FwSize +
                           SFU_IMG_IMAGE_OFFSET);

      /* the header in swap must be the same as the header in dwl slot */
      //다운로드 슬롯의 헤더와 swap 영역의 헤더가 동일한지 확인한다.
      //(FW 다운로드 후에 설치 시작 여부를 확인하기 위해서 다운로드 헤더를 swap 영역에 복사해 놓음)
      ret = memcmp(fw_header_dwl_slot, p_header_swap, SE_FW_HEADER_TOT_LEN);

      /* Check if there is enough room for the trailers */
      // FW 코드가 너무 커서 트레일러 영역을 침범하지 않는지 확인
      if ((trailer_begin < end_of_test_image) || (ret != 0))
      {
        /*
          * These error causes are not memorized in the BootInfo area because there won't be any error handling
          * procedure.
          * If this function returns that no new firmware can be installed (as this may be a normal case).
          */
        e_ret_status = SFU_ERROR;

#if defined(SFU_VERBOSE_DEBUG_MODE)
        if (trailer_begin < end_of_test_image)
        {
          TRACE("\r\n\t  The binary image to be installed overlap with the trailer area!");
        } /* check next error cause */

        if (ret != 0)
        {
          TRACE("\r\n\t  The headers in dwl slot and swap area do not match!");
        } /* else do not print the message(s) again */
#endif /* SFU_VERBOSE_DEBUG_MODE */
      }
      else
      {
        e_ret_status = SFU_SUCCESS;
      }
    }
  }
  return e_ret_status;
}

/**
  * @brief Prepares the Candidate FW image for Installation.
  *        This stage depends on the supported features: in this example this consists in decrypting the candidate
  *        image.
  * @note Even if the Firmware Image is in clear format the decrypt function is called.
  *       But, in this case no decrypt is performed, it is only a set of copy operations
  *       to organize the Firmware Image in FLASH as expected by the swap procedure.
  * @param  DwlSlot identification of the downloaded area
  * @param  pFwImageHeader pointer to fw header to install
  * @retval SFU_SUCCESS if successful,SFU_ERROR error otherwise.
  */
static SFU_ErrorStatus PrepareCandidateImageForInstall(uint32_t DwlSlot, SE_FwRawHeaderTypeDef *pFwImageHeader)
{
  SFU_ErrorStatus e_ret_status = SFU_ERROR;
  SE_StatusTypeDef e_se_status;

  /*
    * Control if there is no additional code beyond the firmware image (malicious SW)
    */
  //현재 다운로드 슬롯에 실제 이미지의 크기 외 다른 영역에 쓰레기 값이 없는지 다시한번 확인한다.
  e_ret_status = VerifySlot((uint8_t *) SlotStartAdd[DwlSlot], SLOT_SIZE(DwlSlot),
                            pFwImageHeader->PartialFwSize + (pFwImageHeader->PartialFwOffset %
                                                             SLOT_SIZE(SLOT_SWAP)));

  if (e_ret_status != SFU_SUCCESS)
  {
    SFU_EXCPT_SetError(SFU_EXCPT_ADDITIONAL_CODE_ERR);
    return e_ret_status;
  }

  /*
    * Pre-condition: all checks have been performed,
    *                so all FWIMG module variables are populated.
    * Now we can decrypt in FLASH.
    *
    * Initial FLASH state (focus on dwl slot and swap area):
    *
    * Depending on partial FW offset, we can have:
    * <dwl slot>   : {Candidate Image Header + encrypted FW - page k}
    *                {encrypted FW - page k+1}
    *                .....
    *                {encrypted FW - page k+N}
    *                .....
    * </dwl slot>
    * or:
    * <dwl slot>   : {Candidate Image Header }
    *                {encrypted FW - page k}
    *                .....
    *                {encrypted FW - page k+N}
    *                .....
    * </dwl slot>
    *
    * <Swap area>  : {Candidate Image Header}
    * </Swap area>
    */
  //다운로드 슬롯의 이미지를 복호화한다.
  e_ret_status =  DecryptImageInDwlSlot(DwlSlot, pFwImageHeader);

  if (e_ret_status != SFU_SUCCESS)
  {
    SFU_EXCPT_SetError(SFU_EXCPT_DECRYPT_ERR);
    return e_ret_status;
  }

  /*
    * At this step, the FLASH content looks like this:
    *
    * <dwl slot>   : {decrypted FW - data from page k+1 in page 0}
    *                {decrypted FW - data from page k+2 in page 1}
    *                .....
    *                {decrypted FW - data from page k+N in page N-1}
    *                {page N is now empty}
    *                .....
    * </dwl slot>
    *
    * <Swap area>  : {No Header (shift) + decrypted FW data from page k}
    * </Swap area>
    *
    * The Candidate FW image has been decrypted
    * and a "hole" has been created in dwl slot to be able to swap.
    */

  //복호화가 완료된 이미지를 이용하여 다시한번 fw 의 signature 를 검증한다.
  e_ret_status = VerifyFwSignatureAfterDecrypt(&e_se_status, DwlSlot, pFwImageHeader);
  if (e_ret_status != SFU_SUCCESS)
  {
    SFU_EXCPT_SetError(SFU_EXCPT_SIGNATURE_ERR);
  }

  /* Return the result of this preparation */
  return (e_ret_status);
}


/**
  * @brief  Install the new version
  * @param  ActiveSlot identification of the active image
  * @param  DwlSlot identification of the downloaded area
  * @param  pValidHeader: pointer in ram to header of valid fw to backup
  * @param  pTestHeader: pointer in ram to header of fw to test
  * @retval SFU_SUCCESS if successful,SFU_ERROR error otherwise.
  */
static SFU_ErrorStatus InstallNewVersion(uint32_t ActiveSlot, uint32_t DwlSlot, SE_FwRawHeaderTypeDef *pValidHeader,
                                         SE_FwRawHeaderTypeDef *pTestHeader)
{
  SFU_ErrorStatus e_ret_status = SFU_ERROR;

  /*
    * At this step, the FLASH content looks like this:
    *
    * <dwl slot>   : {decrypted FW - data from page k+1 in page 0}
    *                {decrypted FW - data from page k+2 in page 1}
    *                .....
    *                {decrypted FW - data from page k+N in page N-1}
    *                {page N is now empty}
    *                .....
    * </dwl slot>
    *
    * <Swap area> : {No Header (shift) + decrypted FW data from page k}
    * </Swap area>
    *
    */

  /* erase "swap area" size at the end of dwl slot */
  //다운로드 슬롯에 트레일러 영역을 erase 한다.
  e_ret_status = EraseSlotIndex(DwlSlot, (TRAILER_INDEX(DwlSlot) - 1U));
  if (e_ret_status !=  SFU_SUCCESS)
  {
    SFU_EXCPT_SetError(SFU_EXCPT_TRAILER_ERASING_ERR);
    return e_ret_status;
  }

  /*
    * At this step, the FLASH content looks like this:
    *
    * <dwl slot>   : {decrypted FW - data from page k+1 in page 0}
    *                {decrypted FW - data from page k+2 in page 1}
    *                .....
    *                {decrypted FW - data from page k+N in page N-1}
    *                {page N is now empty}
    *                .....
    *                {at least the swap area size at the end of the last page of dwl slot is empty }
    * </dwl slot>
    *
    * <Swap area>  : {No Header (shift) + decrypted FW data from page 0}
    * </Swap area>
    *
    */


  /*  write trailer  */
  //트레일러 영역에 valid 헤더(현재 액티브 슬롯 헤더) 와 test 헤더(다운로드 fw 헤더) 및 SWAP magic 을 기록한다.
  //트레일러의 이 정보를 보고 설치가 시작되었는지 알 수 있다.
  e_ret_status = WriteTrailerHeader(DwlSlot, pValidHeader, pTestHeader);
  if (e_ret_status != SFU_SUCCESS)
  {
    SFU_EXCPT_SetError(SFU_EXCPT_TRAILER_UPDATE_ERR);
    return e_ret_status;
  }

  /*
    * At this step, the FLASH content looks like this:
    *
    * <dwl slot>   : {decrypted FW - data from page k+1 in page 0}
    *                {decrypted FW - data from page k+2 in page 1}
    *                .....
    *                {decrypted FW - data from page k+N in page N-1}
    *                {page N is now empty}
    *                .....
    *                {the last page of dwl slot ends with the trailer magic patterns and the free space for the trailer
    *                 info}
    * </dwl slot>
    *
    * <Swap area>  : {No Header (shift) + decrypted FW data from page 0}
    * </Swap area>
    *
    * The trailer magic patterns is ActiveFwHeader|CandidateFwHeader|SWAPmagic.
    * This is followed by a free space to store the trailer info (N*CPY_TO_ACTIVE_SLOT) and (N*CPY_TO_DWL_SLOT):
    *
    *<------------------------------------------ Last page of dwl slot --------------------------------------->
    * |HEADER_TOT_LEN|HEADER_TOT_LEN|MAGIC_LENGTH|MAGIC_LENGTH |N*sizeof(FLASH_write_t)|N*sizeof(FLASH_write_t)
    * | header 1     | header 2     |SWAP magic  |CLEAN pattern|N*CPY_TO_ACTIVE_SLOT   | N*CPY_TO_DWL_SLOT
    *
    */

  /*  swap  */
  //다운로드 슬롯과 액티브 슬롯을 서로 swap 한다.
  //내부 코드는 매우 복잡한데 이해를 위해 simple 하게 정리하면 액티브 슬롯 영역을 다운로드 슬롯으로 옮기고
  //반대로 다운로드 슬롯의 내용을 액티브 슬롯으로 이동시킨다. 이 때 swap 슬롯을 사용한다.
  //swap 진행상황은 트레일러의 N*CPY_TO_ACTIVE_SLOT 과 N*CPY_TO_DWL_SLOT 에 기록되기 때문에 swap 도중에 중단되었다가
  //resume 하는 경우에는 이 정보를 바탕으로 다시 swap 을 resume 할 수 있다.
  e_ret_status = SwapFirmwareImages(ActiveSlot, DwlSlot, pTestHeader);
  if (e_ret_status != SFU_SUCCESS)
  {
    SFU_EXCPT_SetError(SFU_EXCPT_FWIMG_SWAP_ERR);
    return e_ret_status;
  }

  /* Validate immediately the new active FW */
  //모든 swap 이 완료되었으면 다운로드 슬롯의 내용이 모두 액티브 슬롯에 기록되었으므로 다운로드 슬롯의 헤더를 액티브 슬롯에 기록한다.
  e_ret_status = SFU_IMG_Validation(ActiveSlot, pTestHeader);
  if (SFU_SUCCESS != e_ret_status)
  {
    SFU_EXCPT_SetError(SFU_EXCPT_HEADER_VALIDATION_ERR);
    return e_ret_status;
  }

  /* clear swap pattern */
  //설치가 완료되었으므로 다운로드 트레일러의 Clean 영역에 0x55 를 writing 하여 설치 완료를 기록해 준다.
  e_ret_status = CleanMagicValue(DwlSlot);
  if (e_ret_status != SFU_SUCCESS)
  {
    /* Memorize the error */
    SFU_EXCPT_SetError(SFU_EXCPT_FWIMG_MAGIC_ERR);
    return e_ret_status;
  }

  return e_ret_status;   /* Leave the function now */
}

/* Functions Definition : services ------------------------------------------- */
/**
  * @brief FW Image Handling (FWIMG) initialization function from swap process.
  *        Checks the validity of the settings related to image handling (slots size and alignments...).
  * @note  The system initialization must have been performed before calling this function (flash driver ready to be
  *        used...etc...).
  *        Must be called first (and once) before calling the other Image handling services.
  * @param  None.
  * @retval SFU_IMG_InitStatusTypeDef SFU_IMG_INIT_OK if successful, an error code otherwise.
  */
SFU_IMG_InitStatusTypeDef SFU_IMG_CheckSwapImageHandling(void)
{
  SFU_IMG_InitStatusTypeDef e_ret_status = SFU_IMG_INIT_OK;
  uint32_t i;

  /*
   * Sanity check: let's make sure the slot sizes are correct
   * to avoid discrepancies between linker definitions and constants in sfu_fwimg_regions.h
   */
  //액티브 슬롯의 크기가 swap 영역 크기와 align 되어 있는지 확인
  for (i = 0U; i < SFU_NB_MAX_ACTIVE_IMAGE; i++)
  {
    if (SlotStartAdd[SLOT_ACTIVE_1 + i] != 0U)
    {
      if (!(SFU_IMG_REGION_IS_MULTIPLE(SLOT_SIZE(SLOT_ACTIVE_1 + i), SLOT_SIZE(SLOT_SWAP))))
      {
        TRACE("\r\n= [FWIMG] SLOT_ACTIVE_%d size (%d) must be a multiple of swap size (%d)\r\n",
              SLOT_ACTIVE_1 + i, SLOT_SIZE(SLOT_ACTIVE_1 + i), SLOT_SIZE(SLOT_SWAP));
        e_ret_status = SFU_IMG_INIT_SLOTS_SIZE_ERROR;
      }
    }
  }

  //다운로드 슬롯의 크기가 swap 영역 크기와 align 되어 있는지 확인
  for (i = 0U; i < SFU_NB_MAX_DWL_AREA; i++)
  {
    if (SlotStartAdd[SLOT_DWL_1 + i] != 0U)
    {
      if (!(SFU_IMG_REGION_IS_MULTIPLE(SLOT_SIZE(SLOT_DWL_1 + i), SLOT_SIZE(SLOT_SWAP))))
      {
        TRACE("\r\n= [FWIMG] SLOT_DWL_%d size (%d) must be a multiple of swap size (%d)\r\n",
              i + 1U, SLOT_SIZE(SLOT_DWL_1 + i), SLOT_SIZE(SLOT_SWAP));
        e_ret_status = SFU_IMG_INIT_SLOTS_SIZE_ERROR;
      }
    }
  }

  /*
   * Sanity check: let's make sure the slot size is correct with regards to the swap procedure constraints.
   * The swap procedure cannot succeed if the trailer info size is bigger than what a chunk used for swapping can carry.
   */
  //트레일러의 크기가 chunk 크기보다 작은지 확인
  for (i = 0U; i < SFU_NB_MAX_DWL_AREA; i++)
  {
    if (SlotStartAdd[SLOT_DWL_1 + i] != 0U)
    {
      if (((int32_t)(SFU_IMG_CHUNK_SIZE - (TRAILER_INDEX(SLOT_DWL_1 + i) * sizeof(SFU_LL_FLASH_write_t)))) < 0)
      {
        e_ret_status = SFU_IMG_INIT_SWAP_SETTINGS_ERROR;
        TRACE("\r\n= [FWIMG] %d bytes required for the swap metadata of SLOT_DWL_%d is too much\r\n",
              (TRAILER_INDEX(SLOT_DWL_1 + i) * sizeof(SFU_LL_FLASH_write_t)), i + 1U);
      } /* else the swap settings are fine from a metadata size perspective */
    }
  }

  /*
   * Sanity check: let's make sure the swap size is correct with regards to the swap procedure constraints.
   * The swap size must be a multiple of the chunks size used to do the swap.
   */
  //swap 영역 크기와 swap chunk 크기의 alignment 확인
#if defined(__GNUC__)
  __IO uint32_t swap_size = SLOT_SIZE(SLOT_SWAP), swap_chunk = SFU_IMG_CHUNK_SIZE;
  if (0U != ((uint32_t)(swap_size % swap_chunk)))
#else
  if (0U != ((uint32_t)(SLOT_SIZE(SLOT_SWAP) % SFU_IMG_CHUNK_SIZE)))
#endif /* __GNUC__ */
  {
    e_ret_status = SFU_IMG_INIT_SWAP_SETTINGS_ERROR;
    TRACE("\r\n= [FWIMG] The swap procedure uses chunks of %d bytes but the swap size (%d) is not a multiple\r\n",
          SFU_IMG_CHUNK_SIZE, SLOT_SIZE(SLOT_SWAP));
  } /* else the swap settings are fine from a chunk size perspective */

  /*
   * Sanity check: let's make sure the chunks used for the swap procedure are big enough with regards to the offset
   * giving the start @ of the firmware
   */
  //이미지 chunk size 가 offset 보다 큰지 확인, 헤더 정보 때문에 첫번째 chunk 에 헤더가 들어가 있어야 하기 때문인 것으로 판단됨
  if (((int32_t)(SFU_IMG_CHUNK_SIZE - SFU_IMG_IMAGE_OFFSET)) < 0)
  {
    e_ret_status = SFU_IMG_INIT_SWAP_SETTINGS_ERROR;
    TRACE("\r\n= [FWIMG] The swap procedure uses chunks of %d bytes but the firmware start offset is %d bytes\r\n",
          SFU_IMG_CHUNK_SIZE, SFU_IMG_IMAGE_OFFSET);
  } /* else the swap settings are fine from a firmware start offset perspective */

  /*
   * Sanity check: let's make sure SWAP slot is properly aligned with regards to flash constraints
   */
  //swap 영역이 align 되어 있는지 확인
  if (!IS_ALIGNED(SlotStartAdd[SLOT_SWAP]))
  {
    e_ret_status = SFU_IMG_INIT_FLASH_CONSTRAINTS_ERROR;
    TRACE("\r\n= [FWIMG] swap (%x) is not properly aligned\r\n",
          SlotStartAdd[SLOT_SWAP]);
  } /* else swap is properly aligned */

  /*
   * Sanity check: let's make sure the swap area does not overlap SB code area protected by WRP)
   */
  //swap 영역이 WRP 보호 영역을 침범하지 않는지 확인
  if (((SlotStartAdd[SLOT_SWAP] - FLASH_BASE) / FLASH_PAGE_SIZE) <= SFU_PROTECT_WRP_PAGE_END_1)
  {
    TRACE("\r\n= [FWIMG] SWAP overlaps SBSFU code area protected by WRP\r\n");
    e_ret_status = SFU_IMG_INIT_FLASH_CONSTRAINTS_ERROR;
  }


  /* FWIMG core initialization completed */
  return e_ret_status;
}



/**
  * @brief Checks if there is a pending firmware installation.
  *        3 situations can occur:
  *        A. Pending firmware installation: a firmware is ready to be installed but the installation has never been
  *           triggered so far.
  *        B. Pending firmware installation resume: a firmware installation has been interrupted and a resume procedure
  *           is required to finalize installation.
  *        C. No firmware installation pending
  * @note  The anti-rollback check is not taken into account at this stage.
  *        But, if the firmware to be installed already carries some metadata (VALID tag) a newly downloaded firmware
  *        should not have then the installation is not considered as pending.
  * @note This function populates the FWIMG module variables:
  *        - case SFU_IMG_FWUPDATE_STOPPED : fw_image_header_to_test & fw_image_header_validated
  *        - case SFU_IMG_FWIMAGE_TO_INSTALL : fw_image_header_to_test
  *        - case SFU_IMG_NO_FWUPDATE : none
  * @param  pDwlSlotToInstall identification of the downloaded slot to install
  * @param  pActiveSlotToResume identification of the active slot to resume
  * @param  pActiveSlotToRollback identification of the active slot to rollback
  * @retval SFU_IMG_ImgInstallStateTypeDef Pending Installation status (pending install, pending resume install, no
  *         pending action)
  */
SFU_IMG_ImgInstallStateTypeDef SFU_IMG_CheckPendingInstallation(uint32_t *pDwlSlotToInstall, \
                                                                uint32_t *pActiveSlotToResume, \
                                                                uint32_t *pActiveSlotToRollback)
{
  uint32_t i;
  uint32_t j;

  /*
   * The order of the checks is important:
   *
   * A. Check if a FW update has been interrupted first.
   *    This check is based on the content of the trailer area and the validity of the update image.
   *
   * B. Check if a Firmware image is waiting to be installed.
   */

  /* Init values */
  *pActiveSlotToResume = SLOT_INACTIVE;
  *pActiveSlotToRollback = SLOT_INACTIVE;
  *pDwlSlotToInstall = SLOT_INACTIVE;

  /* Check image installation stopped or installation to be done
     =========================================================== */
  //액티브 슬롯에 무엇인가 자료가 있는지 확인
  for (i = 0U; i < SFU_NB_MAX_ACTIVE_IMAGE; i++)
  {
    /* Slot configured ? */
    if (SlotStartAdd[SLOT_ACTIVE_1 + i] != 0U)
    {
      /* Check trailer valide : installation stopped ?
         ============================================= */
      //다운로드 슬롯에 무엇인가 자료가 있는지 확인
      for (j = 0U; j < SFU_NB_MAX_DWL_AREA; j++)
      {
        /* Slot configured ? */
        if (SlotStartAdd[SLOT_DWL_1 + j] != 0U)
        {
          //액티브 슬롯과 다운로드 슬롯에 자료가 있는 경우 설치 중이었는지 확인
          if (SFU_SUCCESS == FirmwareToResume(SLOT_ACTIVE_1 + i, SLOT_DWL_1 + j, &fw_image_header_validated,
                                               &fw_image_header_to_test))
          {
            /* A Firmware Update has been stopped
             * fw_image_header_to_test & fw_image_header_validated have been populated
             * Stop at first occurrence : next ones if any will be resumed after reset
             */
            //실제 설치 중이었던 FW 가 있는 경우에는 SFU_IMG_FWUPDATE_STOPPED 를 반환하여 설치를 Resume 한다.
            *pActiveSlotToResume = SLOT_ACTIVE_1 + i;
            *pDwlSlotToInstall = SLOT_DWL_1 + j;
            return SFU_IMG_FWUPDATE_STOPPED;
          }
        }
      }
    }
  }

  /* Install new firmware if requested in the swap
     ============================================= */
  for (j = 0U; j < SFU_NB_MAX_DWL_AREA; j++)
  {
    /* Slot configured ? */
    if (SlotStartAdd[SLOT_DWL_1 + j] != 0U)
    {
      //다운로드 슬롯에 자료가 있는 경우에는 설치 시작인지 확인한다. (설치 시작 여부는 swap 영역에 헤더 자료가 있는지 여부로 판단한다)
      if (SFU_SUCCESS == FirmwareToInstall(SLOT_DWL_1 + j, &fw_image_header_to_test))
      {
        /*
         * A Firmware is available for installation:
         * fw_image_header_to_test has been populated
         * Stop at first occurrence : next ones if any will be installed after reset
         */
        *pDwlSlotToInstall = SLOT_DWL_1 + j;
        return SFU_IMG_FWIMAGE_TO_INSTALL;
      }
    }
  }

  //resume 하거나 install 할 FW 가 없는 경우 SFU_IMG_NO_FWUPDATE 반환
  return SFU_IMG_NO_FWUPDATE;
}

/**
  * @brief Installs a new firmware, performs the post-installation checks and sets the metadata to tag this firmware as
  *        valid if the checks are successful.
  *        The anti-rollback check is performed and errors are memorized in the bootinfo area, but no action is taken
  *        in this procedure.
  *        Cryptographic operations are used (if enabled): the firmware is decrypted and its authenticity is checked
  *        afterwards if the cryptographic scheme allows this (signature check for instance).
  *        The detailed errors are memorized in bootinfo area to be processed as critical errors if needed.
  *        This function modifies the FLASH content.
  *        If this procedure is interrupted during image preparation stage (e.g.: switch off) it cannot be resumed.
  *        If it is interrupted during installation stage, it can be resumed.
  * @note  It is up to the caller to make sure the conditions to call this primitive are met (no check performed before
  *        running the procedure): SFU_IMG_CheckPendingInstallation, SFU_IMG_CheckCandidateVersion should be called
  *        first.
  * @param  DwlSlot identification of the downloaded area
  * @retval SFU_ErrorStatus SFU_SUCCESS if successful, SFU_ERROR otherwise.
  */
SFU_ErrorStatus SFU_IMG_TriggerImageInstallation(uint32_t DwlSlot)
{
  SFU_ErrorStatus e_ret_status = SFU_ERROR;
  uint8_t *pbuffer;
  uint32_t active_slot = 0U;

  /*
   * The preparation stage consists in decrypting the candidate firmware image.
   * fw_image_header_to_test is already populated after SFU_IMG_CheckPendingInstallation() call.
   */
  //다운로드 슬롯에 있는 fw 는 AES 암호화 되어 있는 상태이므로 이를 복호화 하고 signature 를 검증하여 올바른 이미지가 맞는지 확인한다.
  e_ret_status = PrepareCandidateImageForInstall(DwlSlot, &fw_image_header_to_test);
  if (SFU_SUCCESS != e_ret_status)
  {
    goto exit;
  }

  /*
   * Populate fw_image_header_validated
   */
  if (SFU_SUCCESS == e_ret_status)
  {
    //현재 액티브 슬롯의 헤더 정보를 읽어온다.
    active_slot = SFU_IMG_GetFwImageSlot(&fw_image_header_to_test);
    pbuffer = (uint8_t *) SlotHeaderAdd[active_slot];
    e_ret_status = SFU_LL_FLASH_Read((uint8_t *) &fw_image_header_validated, pbuffer,
                                     sizeof(fw_image_header_validated));
  }

  /*
   * Installing the new firmware image consists in swapping the 2 FW images.
   */
  if (SFU_SUCCESS == e_ret_status)
  {
    //다운로드 슬롯에 있는 fw 를 액티브 슬롯에 설치하고 설치 완료 기록을 writing 한다.
    e_ret_status = InstallNewVersion(active_slot, DwlSlot, &fw_image_header_validated, &fw_image_header_to_test);
  }

exit:
  /* Erase downloaded FW in case of authentication/integrity error
     Erase downloaded FW also in case of installation issue to avoid any infinite loop */
  if (SFU_SUCCESS != e_ret_status)
  {
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

/**
  * @brief Resume installation of new valid firmware after installation failure.
  *        If this procedure is interrupted before its completion (e.g.: switch off) it can be resumed.
  *        This function is dedicated (only) to the resume of installation when a new firmware installation is stopped
  *        before its completion.
  *        This function does not handle explicitly the metadata tagging the firmware as valid: these metadata must
  *        have been preserved during the installation procedure.
  *        This function modifies the FLASH content.
  * @note  It is up to the caller to make sure the conditions to call this primitive are met (no check performed before
  *        running the procedure): need to make sure an installation has been interrupted.
  * @param  ActiveSlot identification of the active image
  * @param  DwlSlot identification of the downloaded area
  * @retval SFU_ErrorStatus SFU_SUCCESS if successful, SFU_ERROR otherwise.
  */
SFU_ErrorStatus SFU_IMG_TriggerResumeInstallation(uint32_t ActiveSlot, uint32_t DwlSlot)
{
  SFU_ErrorStatus e_ret_status = SFU_ERROR;

  /*
   * resume swap process
   * fw_image_header_to_test is already populated after SFU_IMG_CheckPendingInstallation()call.
   */
  //액티브 슬롯과 다운로드 슬롯의 swap 을 resume 한다.
  //resume 할 수 있는 이유는 다운로드 슬롯의 트레일러에 swap 된 이력이 기록되어 있어 이 기록을 보고 swap 이 완료되지 않은
  //영역부터 swap 할 수 있기 때문이다.
  e_ret_status = SwapFirmwareImages(ActiveSlot, DwlSlot, &fw_image_header_to_test);
  if (e_ret_status != SFU_SUCCESS)
  {
    SFU_EXCPT_SetError(SFU_EXCPT_FWIMG_SWAP_ERR);
    goto exit;
  }

  /* Validate immediately the new active FW */
  //swap 이 완료되면 다운로드 슬롯의 테스트 헤더를 액티브 슬롯에 기록하여 준다.
  if (e_ret_status == SFU_SUCCESS)
  {
    e_ret_status = SFU_IMG_Validation(ActiveSlot, &fw_image_header_to_test);
    if (e_ret_status != SFU_SUCCESS)
    {
      SFU_EXCPT_SetError(SFU_EXCPT_HEADER_VALIDATION_ERR);
      goto exit;
    }
  }

  /* clear swap pattern */
  //설치가 완료되었기 때문에 트레일러 영역의 clean 영역을 0x55 로 write 하여 완료 처리한다.
  if (e_ret_status == SFU_SUCCESS)
  {
    e_ret_status = CleanMagicValue(DwlSlot);
    if (e_ret_status != SFU_SUCCESS)
    {
      /* Memorize the error */
      SFU_EXCPT_SetError(SFU_EXCPT_FWIMG_MAGIC_ERR);
      goto exit;
    }
  }

exit:
  /* Erase downloaded FW in case of installation issue to avoid any infinite loop */
//설치가 정상적으로 완료되지 않은 경우 다운로드 슬롯을 erase 한다.
  if (SFU_SUCCESS != e_ret_status)
  {
    (void) SFU_IMG_EraseDownloadedImg(DwlSlot);
  }

#if defined(SFU_VERBOSE_DEBUG_MODE)
  if (e_ret_status == SFU_SUCCESS)
  {
    TRACE("\r\n\t  Resume procedure completed.");
  }
  else
  {
    TRACE("\r\n\t  Resume procedure cannot be finalized!");
  }
#endif /* SFU_VERBOSE_DEBUG_MODE */

  return e_ret_status;
}

/**
  * @brief  Get size of the trailer
  * @note   This area mapped at the end of a dwl slot is not available for the firmware image
  * @param  SlotNumber index of the slot in the list
  * @retval Size of the trailer.
  */
uint32_t SFU_IMG_GetTrailerSize(uint32_t SlotNumber)
{
  return TRAILER_SIZE(SlotNumber);
}

/**
  * @brief  Erase downloaded firmware in case of decryption/authentication/integrity error
  * @param  DwlSlot identification of the downloaded area
  * @retval SFU_SUCCESS if successful, a SFU_ErrorStatus error otherwise.
  */
SFU_ErrorStatus SFU_IMG_EraseDownloadedImg(uint32_t DwlSlot)
{
  SFU_FLASH_StatusTypeDef flash_if_status;
  SFU_ErrorStatus e_ret_status_dwl = SFU_ERROR;
  SFU_ErrorStatus e_ret_status_swap = SFU_ERROR;

  e_ret_status_dwl = SFU_LL_FLASH_Erase_Size(&flash_if_status, (uint8_t *) SlotStartAdd[DwlSlot],
                                             SLOT_SIZE(DwlSlot));
  STATUS_FWIMG(e_ret_status_dwl == SFU_ERROR, SFU_IMG_FLASH_ERASE_FAILED);

  e_ret_status_swap = SFU_LL_FLASH_Erase_Size(&flash_if_status, (uint8_t *) SlotStartAdd[SLOT_SWAP],
                                              SLOT_SIZE(SLOT_SWAP));
  STATUS_FWIMG(e_ret_status_swap == SFU_ERROR, SFU_IMG_FLASH_ERASE_FAILED);

  return (((e_ret_status_swap != SFU_SUCCESS) || (e_ret_status_dwl != SFU_SUCCESS)) ?  SFU_ERROR : SFU_SUCCESS);
}

#endif /* !(SFU_NO_SWAP) */

