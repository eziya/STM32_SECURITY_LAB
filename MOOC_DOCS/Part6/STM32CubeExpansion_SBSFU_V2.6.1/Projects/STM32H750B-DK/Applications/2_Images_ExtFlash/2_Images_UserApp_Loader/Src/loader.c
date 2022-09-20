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
#include "main.h"
#include "loader.h"
#include "loader_low_level.h"
#include "loader_low_level_flash.h"
#include "se_def_metadata.h"
#include <string.h>
#include <stdlib.h>

/* Private defines -----------------------------------------------------------*/
#define LOADER_TIME_OUT            ((uint32_t )0x800U)       /*!< COM Transmit and Receive Timeout*/
#define LOADER_SERIAL_TIME_OUT     ((uint32_t )100U)         /*!< Serial PutByte and PutString Timeout*/

#define IS_CAP_LETTER(c)    (((c) >= (uint8_t)'A') && ((c) <= (uint8_t)'F'))
#define IS_LC_LETTER(c)     (((c) >= (uint8_t)'a') && ((c) <= (uint8_t)'f'))
#define IS_09(c)            (((c) >= (uint8_t)'0') && ((c) <= (uint8_t)'9'))
#define ISVALIDHEX(c)       (IS_CAP_LETTER(c) || IS_LC_LETTER(c) || IS_09(c))
#define ISVALIDDEC(c)       IS_09(c)
#define CONVERTDEC(c)       ((c) - (uint8_t)'0')

#define CONVERTHEX_ALPHA(c) (IS_CAP_LETTER(c) ? ((c) - (uint8_t)'A' + 10U) : ((c) - (uint8_t)'a' + 10U))
#define CONVERTHEX(c)       (IS_09(c) ? ((c) - (uint8_t)'0') : CONVERTHEX_ALPHA(c))

/* Private variables ---------------------------------------------------------*/
#ifdef MINICOM_YMODEM
/*
 * With Minicom Ymodem, the data offset is not aligned because YMODEM_PACKET_DATA_INDEX is 7
 * So we cannot write in FLASH with pData as source, we need a copy in an aligned buffer
 */
/* With Minicom YMODEM each packet carries 128 bytes of data */
static uint8_t alignedBuffer[YMODEM_PACKET_SIZE] __attribute__((aligned(8)));
#endif /* MINICOM_YMODEM */
static uint32_t m_uDwlAreaStart = 0U;                            /*!< Address of download area */
static uint32_t m_uDwlAreaSize = 0U;                             /*!< Size of download area */
static uint32_t m_uFileSizeYmodem = 0U;                          /*!< Ymodem file size being received */
static uint32_t m_uNbrBlocksYmodem = 0U;                         /*!< Number of blocks being received via Ymodem*/
static uint32_t m_uPacketsReceived = 0U;                         /*!< Number of packets received via Ymodem*/
static uint32_t m_uDwlSlot = SLOT_DWL_1;                         /*!< 2 slots per image config : active + dwl */
static uint32_t m_uDwlType = DWL_FW;                             /*!< Type of content downloaded */

/* Private function prototypes -----------------------------------------------*/
static LOADER_StatusTypeDef LOADER_VerifyFwHeader(uint8_t *pBuffer);
static LOADER_StatusTypeDef LOADER_YMODEM_Receive(uint32_t *puSize);
static HAL_StatusTypeDef LOADER_YMODEM_RX_Packet(uint8_t *pData, uint32_t *puLength, uint32_t uTimeout);
static HAL_StatusTypeDef LOADER_Serial_PutByte(uint8_t uParam);
static uint32_t Str2Int(uint8_t *pInputStr, uint32_t *pIntNum);
static LOADER_StatusTypeDef LOADER_YMODEM_HeaderPktRxCpltCallback(uint32_t uFileSize);
static LOADER_StatusTypeDef LOADER_YMODEM_DataPktRxCpltCallback(uint8_t *pData, uint32_t uSize);
static uint32_t LOADER_GetFwImageSlot(SE_FwRawHeaderTypeDef *pFwImageHeader);


/* Functions Definition ------------------------------------------------------*/
/**
  * @brief  Initialize the SFU LOADER.
  * @param  None
  * @retval LOADER_OK if successful, LOADER_ERR otherwise.
  */
LOADER_StatusTypeDef LOADER_Init(void)
{
  /*
   * Sanity check to make sure that the local loader cannot read out of the buffer bounds
   * when doing a length alignment before writing in FLASH.
   */
#ifndef MINICOM_YMODEM
  if (0U != (uint32_t)(YMODEM_PACKET_1K_SIZE % (uint32_t)sizeof(LOADER_LL_FLASH_write_t)))
  {
    /* The packet buffer (payload part) must be a multiple of the FLASH write length  */
    TRACE("\r\nPacket Payload size (%d) is not matching the FLASH constraints", YMODEM_PACKET_1K_SIZE);
    return LOADER_ERR;
  }
#else
  if (0U != (uint32_t)(YMODEM_PACKET_SIZE % (uint32_t)sizeof(LOADER_LL_FLASH_write_t)))
  {
    /* The packet buffer (payload part) must be a multiple of the FLASH write length  */
    TRACE("\r\nPacket Payload size (%d) is not matching the FLASH constraints", YMODEM_PACKET_1K_SIZE);
    return LOADER_ERR;
  }
#endif /* MINICOM_YMODEM */

  return LOADER_OK;
}

/**
  * @brief  DeInitialize the SFU LOADER.
  * @param  None
  * @retval LOADER_OK, successful.
  */
LOADER_StatusTypeDef LOADER_DeInit(void)
{
  return LOADER_OK;
}

/**
  * @brief  Download a new User Fw via Ymodem protocol.
  *         Writes firmware received in FLASH.
  * @param  pRxSize: Size of the downloaded image.
  * @param  pDwSlot: Identification of the dowloaded area.
  * @param  pDwlType: Type of downloaded content : FW, Wireless stack, FUS ...
  * @retval LOADER_OK if successful, see LOADER_StatusTypeDef otherwise.
  */
LOADER_StatusTypeDef LOADER_DownloadNewUserFw(uint32_t *pRxSize, uint32_t *pDwlSlot, uint32_t *pDwlType)
{
  LOADER_StatusTypeDef e_ret_status = LOADER_OK;

  /* If the SecureBoot configured the IWDG, loader must reload IWDG counter with value defined in the reload register */
  WRITE_REG(IWDG1->KR, IWDG_KEY_RELOAD);

  /* Transfer FW Image via YMODEM protocol */
  (void) printf("\r\n\t  File> Transfer> YMODEM> Send ");

  /* Initialize CRC, needed to control data integrity */
  if (LOADER_LL_CRC_Init() != HAL_OK)
  {
    e_ret_status = LOADER_ERR_DOWNLOAD;
  }

  /* Receive the FW in RAM and write it in the Flash */
  if (e_ret_status == LOADER_OK)
  {
    e_ret_status = LOADER_YMODEM_Receive(pRxSize);
    if (e_ret_status == LOADER_OK)
    {
      /* Check received FW size */
      if (*pRxSize == 0U)
      {
        /* Download file size is not correct */
        e_ret_status = LOADER_ERR_DOWNLOAD;
      }
    }
  }

  /*
   * return :
   * - type of downloaded content : FW/Others
   * - slot identification
   */
  *pDwlType = m_uDwlType;
  *pDwlSlot = m_uDwlSlot;

  return e_ret_status;
}

/* Private Functions Definition ------------------------------------------------*/
/**
  * @brief  Verifies the Raw Fw Header received. It checks if the header is
  *         authentic and if the fields are ok with the device (e.g. size and version).
  * @param  pBuffer: pointer to header buffer.
  * @retval LOADER_OK if successful, see LOADER_StatusTypeDef otherwise.
  */
static LOADER_StatusTypeDef LOADER_VerifyFwHeader(uint8_t *pBuffer)
{
  LOADER_StatusTypeDef e_ret_status = LOADER_ERR_CMD_AUTH_FAILED;
  uint32_t slot_number;
  SE_FwRawHeaderTypeDef *p_header;

  /* Parse the received buffer and identify the dwl slot */
  p_header = (SE_FwRawHeaderTypeDef *)(uint32_t)pBuffer;

  /* Check if the received header packet is authentic : SFU1 / SFU2 / SFU3 */
  slot_number = LOADER_GetFwImageSlot(p_header);
  if (slot_number != SLOT_INACTIVE)
  {
    /* Magic SFU1/SFU2/SFU3 identified ==> this is a FW */
    m_uDwlType = DWL_FW;

    /*
     * 2 slots per image configuration. Identify the download area.
     *   It will be based on SFU magic :
     *   - same number as active_slot if configured.
     *   - else : SLOT_DWL_1
     *    As example : Header magic is SFU2, active slot is SLOT_ACTIVE_2 and dwl slot will be SLOT_DWL2.
     */
    m_uDwlSlot = slot_number - SLOT_ACTIVE_1 + SLOT_DWL_1;
    if (SLOT_SIZE(m_uDwlSlot) == 1U)
    {
      m_uDwlSlot = SLOT_DWL_1;
    }
    /* Assign the download flash address to be used during the YMODEM process */
    m_uDwlAreaStart =  SlotStartAdd[m_uDwlSlot];
    m_uDwlAreaSize  =  SLOT_SIZE(m_uDwlSlot);

    /*
     * Check length :
     * We do not check the length versus the trailer constraint (related to SWAP mechanism).
     * If the firmware is too big to have some room left for the trailer info then the installation will not be
     * triggered.
     *
     * Nevertheless, we can still detect if the FW is too big to be downloaded (cannot be written in dwl slot).
     * This will avoid download issues (but the installation itself can still be rejected) or overflows.
     * The dwl slot must contain the HEADER and also the binary FW (encrypted).
     * But there is an offset of FW_OFFSET_IMAGE bytes to respect.
     */

    if ((p_header->PartialFwSize + (p_header->PartialFwOffset % SLOT_SIZE(SLOT_SWAP))) > (SLOT_SIZE(
          m_uDwlSlot) - IMAGE_OFFSET))
    {
      /* The firmware cannot be written in dwl slot */
      e_ret_status = LOADER_ERR_FW_LENGTH;
    }
    else
    {
      e_ret_status = LOADER_OK;
    }
  }

  return e_ret_status;
}

/**
  * @brief  Get firmware image ID from header.
  * @param  pFwImageHeader pointer to a structure to handle the header info
  * @retval Fw Id : SLOT_ACTIVE_1 / 2 /... / SFU_NB_MAX_ACTIVE_IMAGE
  */
static uint32_t LOADER_GetFwImageSlot(SE_FwRawHeaderTypeDef *pFwImageHeader)
{
  uint32_t active_slot;

  if (memcmp(pFwImageHeader->SFUMagic, SFUM_1, strlen(SFUM_1)) == 0)
  {
    active_slot = SLOT_ACTIVE_1;
  }
  else if (memcmp(pFwImageHeader->SFUMagic, SFUM_2, strlen(SFUM_2)) == 0)
  {
    active_slot = SLOT_ACTIVE_2;
  }
  else if (memcmp(pFwImageHeader->SFUMagic, SFUM_3, strlen(SFUM_3)) == 0)
  {
    active_slot = SLOT_ACTIVE_3;
  }
  else
  {
    active_slot = SLOT_INACTIVE;
  }

  return active_slot;
}
/**
  * @brief  Receive a file using the ymodem protocol.
  * @param  peCOMStatus: YMODEM_StatusTypeDef result of reception/programming.
  * @param  puSize: size of received file.
  * @retval HAL_OK if successful, HAL_ERROR otherwise
  */
static LOADER_StatusTypeDef LOADER_YMODEM_Receive(uint32_t *puSize)
{
  LOADER_StatusTypeDef e_ret_status = LOADER_OK;
  uint32_t i;
  uint32_t packet_length = 0U;
  uint32_t session_done = 0U;
  uint32_t file_done;
  uint32_t errors = 0U;
  uint32_t session_begin = 0U;
  uint32_t filesize = 0U;
  uint32_t packets_received;
  uint8_t *file_ptr;
  uint8_t file_size[YMODEM_FILE_SIZE_LENGTH + 1U];
  uint8_t tmp;
  static uint8_t  m_aPacketData[YMODEM_PACKET_1K_SIZE + YMODEM_PACKET_DATA_INDEX +
                                YMODEM_PACKET_TRAILER_SIZE] __attribute__((aligned(8))); /*!<Array used to store Packet */



  while ((session_done == 0U) && (e_ret_status == LOADER_OK))
  {
    packets_received = 0U;
    file_done = 0U;
    while ((file_done == 0U) && (e_ret_status == LOADER_OK))
    {
      switch (LOADER_YMODEM_RX_Packet(m_aPacketData, &packet_length, YMODEM_DOWNLOAD_TIMEOUT))
      {
        case HAL_OK:
          errors = 0U;
          switch (packet_length)
          {
            case 3U:
              /* Startup sequence */
              break;
            case 2U:
              /* Abort by sender */
              (void) LOADER_Serial_PutByte(YMODEM_ACK);
              e_ret_status = LOADER_ERR_ABORT;
              break;
            case 0U:
              /* End of transmission */
              (void) LOADER_Serial_PutByte(YMODEM_ACK);
              *puSize = filesize;
              file_done = 1U;
              break;
            default:
              /* Normal packet */
              if (m_aPacketData[YMODEM_PACKET_NUMBER_INDEX] != (packets_received & 0x000000FFU))
              {
                /* No NACK sent for a better synchro with remote : packet will be repeated */
              }
              else
              {
                if (packets_received == 0U)
                {
                  /* File name packet */
                  if (m_aPacketData[YMODEM_PACKET_DATA_INDEX] != 0U)
                  {
                    /* File name skipped */
                    i = 0U;
                    file_ptr = m_aPacketData + YMODEM_PACKET_DATA_INDEX;
                    while ((*file_ptr != 0U) && (i < YMODEM_FILE_NAME_LENGTH))
                    {
                      file_ptr++;
                    }
                    /* end of file name ('\0') skipped */
                    file_ptr ++;

                    /* File size extraction */
                    i = 0U;
                    while ((*file_ptr != (uint8_t)' ') && (i < YMODEM_FILE_SIZE_LENGTH))
                    {
                      file_size[i] = *file_ptr;
                      i++;
                      file_ptr++;
                    }
                    file_size[i] = (uint8_t)'\0';
                    (void) Str2Int(file_size, &filesize);

                    /* Header packet received callback call*/
                    e_ret_status = LOADER_YMODEM_HeaderPktRxCpltCallback((uint32_t) filesize);
                    if (e_ret_status == LOADER_OK)
                    {
                      /* Continue reception */
                      (void) LOADER_Serial_PutByte(YMODEM_ACK);
                      (void) LOADER_LL_UART_Flush();
                      (void) LOADER_Serial_PutByte(YMODEM_CRC16);
                    }
                    else
                    {
                      /* End session */
                      tmp = YMODEM_CA;
                      (void) LOADER_LL_UART_Transmit(&tmp, 1U, YMODEM_NAK_TIMEOUT);
                      (void) LOADER_LL_UART_Transmit(&tmp, 1U, YMODEM_NAK_TIMEOUT);
                    }
                  }
                  /* File header packet is empty, end session */
                  else
                  {
                    (void) LOADER_Serial_PutByte(YMODEM_ACK);
                    file_done = 1U;
                    session_done = 1U;
                    break;
                  }
                }
                else /* Data packet */
                {
                  /* Data packet received callback call*/
                  e_ret_status = LOADER_YMODEM_DataPktRxCpltCallback(&m_aPacketData[YMODEM_PACKET_DATA_INDEX],
                                                                     packet_length);
                  if (e_ret_status == LOADER_OK)
                  {
                    /* Continue reception */
                    (void) LOADER_Serial_PutByte(YMODEM_ACK);
                  }
                  else /* An error occurred while writing to Flash memory / Checking header */
                  {
                    /* End session */
                    (void) LOADER_Serial_PutByte(YMODEM_CA);
                    (void) LOADER_Serial_PutByte(YMODEM_CA);
                  }
                }
                packets_received ++;
                session_begin = 1;
              }
              break;
          }
          break;
        case HAL_BUSY: /* Abort actually */
          (void) LOADER_Serial_PutByte(YMODEM_CA);
          (void) LOADER_Serial_PutByte(YMODEM_CA);
          e_ret_status = LOADER_ERR_ABORT;
          break;
        default:
          if (session_begin > 0U)
          {
            errors ++;
          }
          if (errors > YMODEM_MAX_ERRORS)
          {
            /* Abort communication */
            (void) LOADER_Serial_PutByte(YMODEM_CA);
            (void) LOADER_Serial_PutByte(YMODEM_CA);
            e_ret_status = LOADER_ERR_COM;
          }
          else
          {
            (void) LOADER_Serial_PutByte(YMODEM_CRC16);          /* Ask for a packet */
#ifndef MINICOM_YMODEM
            (void) BSP_LED_Toggle(LOADER_STATUS_LED);
            /* Minicom does not accept this extra character */
            (void) printf("\b.");                                /* Replace C char by . on display console */
#endif /* MINICOM_YMODEM */
          }
          break;
      }
    }
  }
  /* Make sure the status LED is turned off */
  (void) BSP_LED_Off(LOADER_STATUS_LED);

  return e_ret_status;
}

/**
  * @brief  Receive a packet from sender
  * @param  pData: pointer to received data.
  * @param  puLength
  *     0: end of transmission
  *     2: abort by sender
  *    >0: packet length
  * @param  uTimeOut: receive timeout (ms).
  * @retval HAL_OK: normally return
  *         HAL_BUSY: abort by user
  */
static HAL_StatusTypeDef LOADER_YMODEM_RX_Packet(uint8_t *pData, uint32_t *puLength, uint32_t uTimeout)
{
  uint32_t crc;
  uint32_t packet_size = 0U;
  HAL_StatusTypeDef e_ret_status;
  HAL_StatusTypeDef e_rx_status;
  uint8_t char1;
#ifdef MINICOM_YMODEM
  uint32_t myIdx = 0; /* index indicating where to write in pData */
#endif /* MINICOM_YMODEM */

  *puLength = 0U;

  /* If the SecureBoot configured the IWDG, loader must reload IWDG counter with value defined in the reload register */
  WRITE_REG(IWDG1->KR, IWDG_KEY_RELOAD);

  e_rx_status = LOADER_LL_UART_Receive(&char1, 1, uTimeout);

  if (e_rx_status == HAL_OK)
  {
    e_ret_status = HAL_OK;

    switch (char1)
    {
      case YMODEM_SOH:
        packet_size = YMODEM_PACKET_SIZE;
        break;
      case YMODEM_STX:
        packet_size = YMODEM_PACKET_1K_SIZE;
        break;
      case YMODEM_EOT:
        break;
      case YMODEM_CA:
        if ((LOADER_LL_UART_Receive(&char1, 1U, uTimeout) == HAL_OK) && (char1 == YMODEM_CA))
        {
          packet_size = 2U;
        }
        else
        {
          e_ret_status = HAL_ERROR;
        }
        break;
      case YMODEM_ABORT1:
      case YMODEM_ABORT2:
        e_ret_status = HAL_BUSY;
        break;
      case YMODEM_RB:
        (void)LOADER_LL_UART_Receive(&char1, 1U, uTimeout);  /* Ymodem startup sequence : rb ==> 0x72 + 0x62 + 0x0D */
        (void)LOADER_LL_UART_Receive(&char1, 1U, uTimeout);
        packet_size = 3U;
        break;
      default:
        e_ret_status = HAL_ERROR;
        break;
    }
    *pData = char1;

    if (packet_size >= YMODEM_PACKET_SIZE)
    {
#ifndef MINICOM_YMODEM
      e_rx_status = LOADER_LL_UART_Receive(&pData[YMODEM_PACKET_NUMBER_INDEX],
                                           (uint16_t)(packet_size + YMODEM_PACKET_OVERHEAD_SIZE), uTimeout);
#else
      e_rx_status = HAL_OK;
      /* Receive byte per byte until no data left or no space left in buffer */
      while ((HAL_OK == e_rx_status) &&
             ((myIdx + YMODEM_PACKET_NUMBER_INDEX) < YMODEM_PACKET_1K_SIZE +
              YMODEM_PACKET_DATA_INDEX +
              YMODEM_PACKET_TRAILER_SIZE)
            )
      {
        e_rx_status = LOADER_LL_UART_Receive(&pData[YMODEM_PACKET_NUMBER_INDEX + myIdx],
                                             1, uTimeout);
        myIdx++;
      }

      if (myIdx > 1)
      {
        /* We received some bytes */
        e_rx_status = HAL_OK;
      }
#endif /* MINICOM_YMODEM */

      /* Simple packet sanity check */
      if (e_rx_status == HAL_OK)
      {
        e_ret_status = HAL_OK;

        if (pData[YMODEM_PACKET_NUMBER_INDEX] != ((pData[YMODEM_PACKET_CNUMBER_INDEX])
                                                  ^ YMODEM_NEGATIVE_BYTE))
        {
          packet_size = 0U;
          e_ret_status = HAL_ERROR;
        }
        else
        {
          /* Check packet CRC*/
          crc = (((uint32_t)pData[ packet_size + YMODEM_PACKET_DATA_INDEX ]) << 8U) & 0x0000FF00U;;
          crc += pData[ packet_size + YMODEM_PACKET_DATA_INDEX + 1U ];

          if (LOADER_LL_CRC_Calculate((uint32_t *)(uint32_t)&pData[YMODEM_PACKET_DATA_INDEX], packet_size) != crc)
          {
            packet_size = 0U;
            e_ret_status = HAL_ERROR;
          }
        }
      }
      else
      {
        e_ret_status = HAL_ERROR;
      }
    }
  }
  else
  {
    e_ret_status = HAL_ERROR;
  }

  *puLength = packet_size;
  return e_ret_status;
}

/**
  * @brief  Transmit a byte to the COM Port.
  * @param  uParam: The byte to be sent.
  * @retval HAL_OK if successful, HAL_ERROR otherwise.
  */
static HAL_StatusTypeDef LOADER_Serial_PutByte(uint8_t uParam)
{
  return LOADER_LL_UART_Transmit(&uParam, 1U, LOADER_SERIAL_TIME_OUT);
}

/**
  * @brief  Convert a string to an integer.
  * @param  pInputStr: The string to be converted.
  * @param  pIntNum: The integer value.
  * @retval 1: Correct
  *         0: Error
  */
static uint32_t Str2Int(uint8_t *pInputStr, uint32_t *pIntNum)
{
  uint32_t i = 0U;
  uint32_t res = 0U;
  uint32_t val = 0U;
  uint8_t  digit;

  if ((pInputStr[0U] == (uint8_t)'0') && ((pInputStr[1U] == (uint8_t)'x') || (pInputStr[1U] == (uint8_t)'X')))
  {
    i = 2U;
    while ((i < 11U) && (pInputStr[i] != (uint8_t)'\0'))
    {
      if (ISVALIDHEX(pInputStr[i]))
      {
        digit = CONVERTHEX(pInputStr[i]);
        val = (val << 4U) + digit;
      }
      else
      {
        /* Return 0, Invalid input */
        res = 0U;
        break;
      }
      i++;
    }

    /* valid result */
    if (pInputStr[i] == (uint8_t)'\0')
    {
      *pIntNum = val;
      res = 1U;
    }
  }
  else /* max 10-digit decimal input */
  {
    while ((i < 11U) && (res != 1U))
    {
      if (pInputStr[i] == (uint8_t)'\0')
      {
        *pIntNum = val;
        /* return 1 */
        res = 1U;
      }
      else if (((pInputStr[i] == (uint8_t)'k') || (pInputStr[i] == (uint8_t)'K')) && (i > 0U))
      {
        val = val << 10U;
        *pIntNum = val;
        res = 1U;
      }
      else if (((pInputStr[i] == (uint8_t)'m') || (pInputStr[i] == (uint8_t)'M')) && (i > 0U))
      {
        val = val << 20U;
        *pIntNum = val;
        res = 1U;
      }
      else if (ISVALIDDEC(pInputStr[i]))
      {
        digit =  CONVERTDEC(pInputStr[i]);
        val = (val * 10U) + digit;
      }
      else
      {
        /* return 0, Invalid input */
        res = 0U;
        break;
      }
      i++;
    }
  }

  return res;
}


/* Callback Functions Definition ------------------------------------------------*/

/**
  * @brief  Ymodem Header Packet Transfer completed callback.
  * @param  uFileSize: Dimension of the file that will be received.
  * @retval LOADER_OK, successful.
  */
static LOADER_StatusTypeDef LOADER_YMODEM_HeaderPktRxCpltCallback(uint32_t uFileSize)
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
  m_uNbrBlocksYmodem = (m_uFileSizeYmodem + (YMODEM_PACKET_1K_SIZE - 1U)) / YMODEM_PACKET_1K_SIZE;
#else
  /* Minicom sends 128 bytes YMODEM packets */
  m_uNbrBlocksYmodem = (m_uFileSizeYmodem + (YMODEM_PACKET_SIZE - 1U)) / YMODEM_PACKET_SIZE;
#endif /* MINICOM_YMODEM */

  /* NOTE : delay inserted for Ymodem protocol*/
  HAL_Delay(1000U);

  return LOADER_OK;
}

/**
  * @brief  Ymodem Data Packet Transfer completed callback.
  * @param  pData: Pointer to the buffer.
  * @param  uSize: Packet dimension.
  * @retval LOADER_OK if successful, see LOADER_StatusTypeDef otherwise.
  */
#ifndef MINICOM_YMODEM
/* Teraterm variant of the YMODEM protocol: uses 1kB packets */
static LOADER_StatusTypeDef LOADER_YMODEM_DataPktRxCpltCallback(uint8_t *pData, uint32_t uSize)
{
  static uint32_t m_uDwlAreaAddress = 0U;                      /*!< Address of to write in download area */
  LOADER_StatusTypeDef e_ret_status = LOADER_OK;
  uint32_t rx_size = uSize;
  uint8_t *p_data = pData;
  uint32_t uOldSize;

  /*Increase the number of received packets*/
  m_uPacketsReceived++;

  /* Last packet : size of data to write could be different than YMODEM_PACKET_1K_SIZE */
  if (m_uPacketsReceived == m_uNbrBlocksYmodem)
  {
    /*Extracting actual payload from last packet*/
    if (0U == (m_uFileSizeYmodem % YMODEM_PACKET_1K_SIZE))
    {
      /* The last packet must be fully considered */
      rx_size = YMODEM_PACKET_1K_SIZE;
    }
    else
    {
      /* The last packet is not full, drop the extra bytes */
      rx_size = m_uFileSizeYmodem - ((uint32_t)(m_uFileSizeYmodem / YMODEM_PACKET_1K_SIZE) * YMODEM_PACKET_1K_SIZE);
    }
  }

  /* First packet : Contains the FW header (SE_FW_HEADER_TOT_LEN bytes length) which is not encrypted  */
  if (m_uPacketsReceived == 1U)
  {
    /* Verify header */
    e_ret_status = LOADER_VerifyFwHeader(p_data);
    m_uDwlAreaAddress = m_uDwlAreaStart;

    /* Clear Download application area */
    if (e_ret_status == LOADER_OK)
    {
      if (LOADER_LL_FLASH_Erase_Size((uint8_t *) m_uDwlAreaAddress, SLOT_SIZE(m_uDwlSlot)) != HAL_OK)
      {
        e_ret_status = LOADER_ERR_FLASH_ACCESS;
      }
    }
  }

  if (e_ret_status == LOADER_OK)
  {
    /*
     * Set dimension to the appropriate length for FLASH programming.
     * Example: 64-bit length for L4.
     */
    if ((rx_size % (uint32_t)sizeof(LOADER_LL_FLASH_write_t)) != 0U)
    {
      /*
       * By construction, the length of the buffer (fw_decrypted_chunk or pData) must be a multiple of sizeof(write_t)
       * to avoid reading out of the buffer
       */
      uOldSize = rx_size;
      rx_size = rx_size + ((uint32_t)sizeof(LOADER_LL_FLASH_write_t) -
                           (rx_size % (uint32_t)sizeof(LOADER_LL_FLASH_write_t)));
      /*
       * In case of alignment with LOADER_LL_FLASH_write_t structure
       * Initialization of the additional data to the "erasing" pattern
       */
      while (uOldSize < rx_size)
      {
        p_data[uOldSize] = 0xFF;
        uOldSize++;
      }
    }

    /* Check size to avoid writing beyond DWL area */
    if ((m_uDwlAreaAddress + rx_size) > (m_uDwlAreaStart + m_uDwlAreaSize))
    {
      e_ret_status = LOADER_ERR_FW_LENGTH;
    }
  }

  /* Write Data in Flash */
  if (e_ret_status == LOADER_OK)
  {
    if (LOADER_LL_FLASH_Write((uint8_t *)m_uDwlAreaAddress, p_data, rx_size) == HAL_OK)
    {
      m_uDwlAreaAddress += (rx_size);
    }
    else
    {
      e_ret_status = LOADER_ERR_FLASH_ACCESS;
    }
  }

  /* Last packet : reset m_uPacketsReceived */
  if (m_uPacketsReceived == m_uNbrBlocksYmodem)
  {
    m_uPacketsReceived = 0U;
  }

  /* Reset data counters in case of error */
  if (e_ret_status != LOADER_OK)
  {
    /*Reset of the ymodem variables */
    m_uFileSizeYmodem = 0U;
    m_uPacketsReceived = 0U;
    m_uNbrBlocksYmodem = 0U;
  }

  return e_ret_status;
}
#else /* MINICOM_YMODEM */
/* Minicom variant of the YMODEM protocol: uses 128B packets */
static LOADER_StatusTypeDef LOADER_YMODEM_DataPktRxCpltCallback(uint8_t *pData, uint32_t uSize)
{
  static uint8_t fw_header[sizeof(SE_FwRawHeaderTypeDef)] __attribute__((aligned(8)));
  /* Size of downloaded Image initialized with first packet (header) and checked along download process */
  static uint32_t m_uDwlImgSize = 0U;
  static uint32_t m_uDwlAreaAddress = 0U;                      /*!< Address of to write in download area */

  LOADER_StatusTypeDef e_ret_status = LOADER_OK;
  uint32_t rx_size = uSize;
  uint8_t *p_data = pData;
  uint32_t uOldSize;

  /*Increase the number of received packets*/
  m_uPacketsReceived++;

  /* Last packet : size of data to write could be different than YMODEM_PACKET_SIZE */
  if (m_uPacketsReceived == m_uNbrBlocksYmodem)
  {
    /*Extracting actual payload from last packet*/
    if (0 == (m_uFileSizeYmodem % YMODEM_PACKET_SIZE))
    {
      /* The last packet must be fully considered */
      rx_size = YMODEM_PACKET_SIZE;
    }
    else
    {
      /* The last packet is not full, drop the extra bytes */
      rx_size = m_uFileSizeYmodem - ((uint32_t)(m_uFileSizeYmodem / YMODEM_PACKET_SIZE) * YMODEM_PACKET_SIZE);
    }
  }

  /* First packet : Contains the FW header (SE_FW_HEADER_TOT_LEN bytes length) which is not encrypted  */
  if (1 == m_uPacketsReceived)
  {
    /* First packet: beginning of header */
    if (YMODEM_PACKET_SIZE == rx_size)
    {
      /* Copy the beginning of the header */
      memcpy(fw_header, p_data, rx_size);
      rx_size = 0; /* nothing more to do */
    }
    else
    {
      /* Unexpected case */
      e_ret_status = LOADER_ERR;
    }
  }
  else if (2 == m_uPacketsReceived)
  {
    /* Second packet: end of header (64 bytes) */
    memcpy(fw_header + YMODEM_PACKET_SIZE, p_data, 64);

    /* Verify header */
    e_ret_status = LOADER_VerifyFwHeader(fw_header);
    m_uDwlAreaAddress = m_uDwlAreaStart;

    if (e_ret_status == LOADER_OK)
    {
      /*
       * Downloaded Image size : Header size + gap for image alignment to (UpdateFwOffset % sector size) + Partial
       * Image size
       */
      m_uDwlImgSize = ((SE_FwRawHeaderTypeDef *)(uint32_t)fw_header)->PartialFwSize +
                      (((SE_FwRawHeaderTypeDef *)(uint32_t)fw_header)->PartialFwOffset % SLOT_SIZE(SLOT_SWAP)) +
                      IMAGE_OFFSET;

      /* Clear Download application area (including TRAILERS area) */
      if ((e_ret_status == LOADER_OK)
          && (LOADER_LL_FLASH_Erase_Size((uint8_t *) m_uDwlAreaAddress, SLOT_SIZE(m_uDwlSlot)) != HAL_OK))
      {
        e_ret_status = LOADER_ERR_FLASH_ACCESS;
      }

      /* Write the FW header (SE_FW_HEADER_TOT_LEN bytes length) at start of DWL area */
      if (LOADER_LL_FLASH_Write((uint8_t *)m_uDwlAreaAddress, fw_header, SE_FW_HEADER_TOT_LEN) == HAL_OK)
      {
        /* Shift the DWL area pointer, to align image with (PartialFwOffset % sector size) in DWL area */
        m_uDwlAreaAddress += SE_FW_HEADER_TOT_LEN;
        /* Update remaining packet size to write */
        rx_size -= 64; /* not the full header length because 128 from previous packet + 64 from this packet */

        /* Update pData pointer to received packet data */
        p_data += 64;
      }
    }
  }
  /* Else: data packet */

  if (e_ret_status == LOADER_OK)
  {
    if (m_uDwlAreaAddress < (m_uDwlAreaStart + IMAGE_OFFSET))
    {
      /* Skip the padding bytes */
      m_uDwlAreaAddress += rx_size;
      rx_size = 0;
    }
    else
    {
      if (((IMAGE_OFFSET / YMODEM_PACKET_SIZE) + 1) == m_uPacketsReceived)
      {
        /* End of padding bytes : we must take the partial offset into account */
        m_uDwlAreaAddress += ((SE_FwRawHeaderTypeDef *)(uint32_t)fw_header)->PartialFwOffset % SLOT_SIZE(SLOT_SWAP);
      }
      /* else nothing more to do */

      /* Check size to avoid writing beyond DWL image size */
      if ((m_uDwlAreaAddress + rx_size) > (m_uDwlAreaStart + m_uDwlImgSize))
      {
        e_ret_status = LOADER_ERR_FW_LENGTH;
      }

      /*
       * Set dimension to the appropriate length for FLASH programming.
       * Example: 64-bit length for L4.
       */
      if ((rx_size % (uint32_t)sizeof(LOADER_LL_FLASH_write_t)) != 0)
      {
        /*
         * By construction, the length of the buffer (fw_decrypted_chunk or pData) must be a multiple of sizeof(write_t)
         * to avoid reading out of the buffer
         */
        uOldSize = rx_size;
        rx_size = rx_size + ((uint32_t)sizeof(LOADER_LL_FLASH_write_t) -
                             (rx_size % (uint32_t)sizeof(LOADER_LL_FLASH_write_t)));
        /*
         * In case of alignment with LOADER_LL_FLASH_write_t structure
         * Initialization of the additional data to the "erasing" pattern
         */
        while (uOldSize < rx_size)
        {
          p_data[uOldSize] = 0xFF;
          uOldSize++;
        }
      }

      /* Check size to avoid writing beyond DWL area */
      if ((m_uDwlAreaAddress + rx_size) > (m_uDwlAreaStart + m_uDwlAreaSize))
      {
        e_ret_status = LOADER_ERR_FW_LENGTH;
      }

      /* data must be aligned to write in FLASH */
      if (YMODEM_PACKET_SIZE >= rx_size)
      {
        memcpy(alignedBuffer, p_data, rx_size);
      }
      else
      {
        /* Abnormal case, we expect Minicom Ymodem to send 128 bytes packets */
        e_ret_status = LOADER_ERR;
      }
    }

    /* Write Data in Flash */
    if (e_ret_status == LOADER_OK)
    {
      if (LOADER_LL_FLASH_Write((uint8_t *)m_uDwlAreaAddress, alignedBuffer, rx_size) == HAL_OK)
      {
        m_uDwlAreaAddress += (rx_size);
      }
      else
      {
        e_ret_status = LOADER_ERR_FLASH_ACCESS;
      }
    }

    /* Last packet : reset m_uPacketsReceived */
    if (m_uPacketsReceived == m_uNbrBlocksYmodem)
    {
      m_uPacketsReceived = 0U;
    }

    /* Reset data counters in case of error */
    if (e_ret_status != LOADER_OK)
    {
      /*Reset of the ymodem variables */
      m_uFileSizeYmodem = 0U;
      m_uPacketsReceived = 0U;
      m_uNbrBlocksYmodem = 0U;
    }
  }

  return e_ret_status;
}
#endif /* MINICOM_YMODEM */
