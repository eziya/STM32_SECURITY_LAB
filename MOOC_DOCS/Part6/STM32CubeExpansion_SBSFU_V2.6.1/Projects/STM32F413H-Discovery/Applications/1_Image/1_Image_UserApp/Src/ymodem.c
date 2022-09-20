/**
  ******************************************************************************
  * @file    ymodem.c
  * @author  MCD Application Team
  * @brief   Ymodem module.
  *          This file provides set of firmware functions to manage Ymodem
  *          functionalities.
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

/** @addtogroup USER_APP User App Example
  * @{
  */

/** @addtogroup  FW_UPDATE Firmware Update Example
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "ymodem.h"
#include "string.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* @note ATTENTION - please keep this variable 32bit aligned */
static uint8_t m_aPacketData[PACKET_1K_SIZE + PACKET_DATA_INDEX +
                             PACKET_TRAILER_SIZE] __attribute__((aligned(4))); /*!<Array used to store Packet Data*/
uint8_t m_aFileName[FILE_NAME_LENGTH + 1U]; /*!< Array used to store File Name data */

/* Private function prototypes -----------------------------------------------*/
static HAL_StatusTypeDef ReceivePacket(uint8_t *pData, uint32_t *puLength, uint32_t uTimeout);
static uint16_t crc_update(uint16_t crc_in, uint8_t incr);
static uint16_t crc16(const uint8_t *data, uint16_t size);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Receive a packet from sender
  * @param  pData
  * @param  puLength
  *     0: end of transmission
  *     2: abort by sender
  *    >0: packet length
  * @param  uTimeout
  * @retval HAL_OK: normally return
  *         HAL_BUSY: abort by user
  */
static HAL_StatusTypeDef ReceivePacket(uint8_t *pData, uint32_t *puLength, uint32_t uTimeout)
{
  uint32_t crc;
  uint32_t packet_size = 0U;
  HAL_StatusTypeDef status;
  uint8_t char1;
#ifdef MINICOM_YMODEM
  uint32_t myIdx = 0;
#endif /* MINICOM_YMODEM */

  *puLength = 0U;

  /* If the SecureBoot configured the IWDG, UserApp must reload IWDG counter with value defined in the
     reload register */
  WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);

  status = (HAL_StatusTypeDef)COM_Receive(&char1, 1, uTimeout);

  if (status == HAL_OK)
  {
    switch (char1)
    {
      /* start of 128-byte data packet */
      case SOH:
        packet_size = PACKET_SIZE;
        break;
      /* start of 1024-byte data packet */
      case STX:
        packet_size = PACKET_1K_SIZE;
        break;
      /* end of transmission */
      case EOT:
        break;
      /* CA + CA : transmission aborted by sender */
      case CA:
        if ((COM_Receive(&char1, 1U, uTimeout) == HAL_OK) && (char1 == CA))
        {
          packet_size = 2U;                               /* specific packet_size to indicate transmission aborted */
        }
        else
        {
          status = HAL_ERROR;
        }
        break;
      /* abort requested */
      case ABORT1:
      case ABORT2:
        status = HAL_BUSY;
        break;
      /* Begin of startup sequence */
      case RB:
        COM_Receive(&char1, 1U, uTimeout);                /* Ymodem startup sequence : rb ==> 0x72 + 0x62 + 0x0D */
        COM_Receive(&char1, 1U, uTimeout);
        packet_size = 3U;                                 /* specific packet_size to indicate transmission started */
        break;
      default:
        status = HAL_ERROR;
        break;
    }
    *pData = char1;

    if (packet_size >= PACKET_SIZE)
    {
#ifndef MINICOM_YMODEM
      /* Receiving the packet */
      status = COM_Receive(&pData[PACKET_NUMBER_INDEX], packet_size + PACKET_OVERHEAD_SIZE, uTimeout);
#else
      status = HAL_OK;
      while ((HAL_OK == status) &&
             ((myIdx + PACKET_NUMBER_INDEX) < (PACKET_1K_SIZE + PACKET_DATA_INDEX + PACKET_TRAILER_SIZE))
            )
      {
        status = COM_Receive(&pData[PACKET_NUMBER_INDEX + myIdx], 1, uTimeout);
        myIdx++;
      }

      if (myIdx > 1)
      {
        status = HAL_OK;
      }
#endif /* MINICOM_YMODEM */

      /* Simple packet sanity check */
      if (status == HAL_OK)
      {
        if (pData[PACKET_NUMBER_INDEX] != ((pData[PACKET_CNUMBER_INDEX]) ^ NEGATIVE_BYTE))
        {
          /* Packet size set to 0 in case of sanity check error */
          packet_size = 0U;
          status = HAL_ERROR;
        }
        else
        {
          /* Check packet CRC */
          crc = pData[ packet_size + PACKET_DATA_INDEX ] << 8U;
          crc += pData[ packet_size + PACKET_DATA_INDEX + 1U ];
          if (crc16(&pData[PACKET_DATA_INDEX], packet_size) != crc)
          {
            /* Packet size set to 0 in case of CRC error */
            packet_size = 0U;
            status = HAL_ERROR;
          }
        }
      }
      else
      {
        /* Packet size set to 0 in case of reception error */
        packet_size = 0U;
      }
    }
  }

  /* Update the length parameter */
  *puLength = packet_size;
  return status;
}

/**
  * @brief  Software CRC16 calculation.
  * @retval uint16_t CRC intermediate value
  */
static uint16_t crc_update(uint16_t crc_in, uint8_t incr)
{
  uint16_t xor = crc_in >> 15;
  uint16_t out = crc_in << 1;

  if (incr)
  {
    out++;
  }
  if (xor)
  {
    out ^= 0x1021;
  }
  return out;
}

/**
  * @brief  Software CRC16 calculation.
  * @param  pdata: pointer to data buffer.
  * @param  size: buffer length in bytes.
  * @retval uint16_t CRC
  */
static uint16_t crc16(const uint8_t *pdata, uint16_t size)
{
  uint16_t crc;
  uint8_t i;

  for (crc = 0; size > 0; size--, pdata++)
  {
    for (i = 0x80; i; i >>= 1)
    {
      crc = crc_update(crc, *pdata & i);
    }
  }
  for (i = 0; i < 16; i++)
  {
    crc = crc_update(crc, 0);
  }
  return crc;
}

/**
  * @brief  Init of Ymodem module.
  * @param None.
  * @retval None.
  */
void Ymodem_Init(void)
{
}


/* Public functions ---------------------------------------------------------*/
/**
  * @brief  Receive a file using the ymodem protocol with CRC16.
  * @param  puSize The uSize of the file.
  * @param  uFlashDestination where the file has to be downloaded.
  * @retval COM_StatusTypeDef result of reception/programming
  */
COM_StatusTypeDef Ymodem_Receive(uint32_t *puSize, uint32_t uFlashDestination, YMODEM_CallbacksTypeDef *appCb)
{
  uint32_t i;
  uint32_t packet_length;
  uint32_t session_done = 0U;
  uint32_t file_done;
  uint32_t errors = 0U;
  uint32_t session_begin = 0U;
  uint32_t ramsource;
  uint32_t filesize = 0U;
  uint8_t *file_ptr;
  uint8_t file_size[FILE_SIZE_LENGTH + 1U];
  uint8_t tmp;
  uint32_t packets_received;
  COM_StatusTypeDef e_result = COM_OK;

  while ((session_done == 0U) && (e_result == COM_OK))
  {
    packets_received = 0U;
    file_done = 0U;
    while ((file_done == 0U) && (e_result == COM_OK))
    {
      switch (ReceivePacket(m_aPacketData, &packet_length, DOWNLOAD_TIMEOUT))
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
              Serial_PutByte(ACK);
              e_result = COM_ABORT;
              break;
            case 0U:
              /* End of transmission */
              Serial_PutByte(ACK);
              *puSize = filesize;
              file_done = 1U;           /* file reception ended */
              break;
            default:
              /* Normal packet */
              if (m_aPacketData[PACKET_NUMBER_INDEX] != (packets_received & 0xff))
              {
                /* Serial_PutByte(NAK); */
              }
              else
              {
                /* first packet : header (file name + file size) */
                if (packets_received == 0U)
                {
                  /* File name packet */
                  if (m_aPacketData[PACKET_DATA_INDEX] != 0U)
                  {
                    /* File name extraction */
                    i = 0U;
                    file_ptr = m_aPacketData + PACKET_DATA_INDEX;
                    while ((*file_ptr != 0U) && (i < FILE_NAME_LENGTH))
                    {
                      m_aFileName[i++] = *file_ptr++;
                    }

                    /* File size extraction */
                    m_aFileName[i++] = '\0';
                    i = 0U;
                    file_ptr ++;
                    while ((*file_ptr != ' ') && (i < FILE_SIZE_LENGTH))
                    {
                      file_size[i++] = *file_ptr++;
                    }
                    file_size[i++] = '\0';
                    Str2Int(file_size, &filesize);

                    /* Header packet received callback call */
                    if (appCb->Ymodem_HeaderPktRxCpltCallback((uint32_t) filesize) == HAL_OK)
                    {
                      /* Send acknowledge and next packet requested */
                      Serial_PutByte(ACK);
                      COM_Flush();
                      Serial_PutByte(CRC16);
                    }
                    else
                    {
                      /* In case of error : transmission aborted
                         Send CA + CA */
                      tmp = CA;
                      COM_Transmit(&tmp, 1U, NAK_TIMEOUT);
                      COM_Transmit(&tmp, 1U, NAK_TIMEOUT);
                      e_result = COM_ABORT;
                      break;
                    }

                  }
                  /* File header packet is empty, end session */
                  else
                  {
                    Serial_PutByte(ACK);
                    file_done = 1;           /* file reception ended */
                    session_done = 1;        /* sesion ended */
                    break;
                  }
                }
                else /* Data packet */
                {
                  ramsource = (uint32_t) & m_aPacketData[PACKET_DATA_INDEX];

                  /* Data packet received callback call*/
                  if (appCb->Ymodem_DataPktRxCpltCallback((uint8_t *) ramsource, uFlashDestination,
                                                          (uint32_t) packet_length) == HAL_OK)
                  {
                    /* Packet acknowledged : newt one requested */
                    Serial_PutByte(ACK);
                  }
                  else /* An error occurred while writing to Flash memory */
                  {
                    /* Abort transmission : CA + CA */
                    tmp = CA;
                    COM_Transmit(&tmp, 1U, NAK_TIMEOUT);
                    COM_Transmit(&tmp, 1U, NAK_TIMEOUT);
                    e_result = COM_ERROR;
                  }
                }
                packets_received ++;
                session_begin = 1U;
              }
              break;
          }
          break;
        case HAL_BUSY: /* Abort actually */
          Serial_PutByte(CA);
          Serial_PutByte(CA);
          e_result = COM_ABORT;
          break;
        default:
          /* Com aborted after 5 consecutives error */
          if (session_begin > 0U)
          {
            errors ++;
          }
          if (errors > MAX_ERRORS)
          {
            /* Abort communication */
            Serial_PutByte(CA);
            Serial_PutByte(CA);
            e_result = COM_ABORT;
          }
          else
          {
            Serial_PutByte(CRC16); /* Ask for a packet */
#ifndef MINICOM_YMODEM
            printf("\b.");         /* Replace C char by . on display console */
#endif /* MINICOM_YMODEM */
          }
          break;
      }
    }
  }
  return e_result;
}

/**
  * @}
  */

/**
  * @}
  */
