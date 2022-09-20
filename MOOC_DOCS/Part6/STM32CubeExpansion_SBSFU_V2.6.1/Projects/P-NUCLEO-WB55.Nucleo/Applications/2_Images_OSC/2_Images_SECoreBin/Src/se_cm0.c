/**
  ******************************************************************************
  * @file    se_cm0.c
  * @author  MCD Application Team
  * @brief   Secure Engine communication with CM0.
  *          This file provides set of functions to manage communication with CM0
  *          for Customer Key Services as well as stack or FUS update services.
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

#include "se_cm0.h"
#include "stm32wbxx_hal.h"
#include "hw.h"
#include "sfu_standalone_loader.h"
#if defined(__ARMCC_VERSION)
#include "mapping_sbsfu.h"
#elif defined (__ICCARM__) || defined(__GNUC__)
#include "mapping_export.h"
#endif /* __ICCARM__ || __GNUC__ */

/* Private defines -----------------------------------------------------------*/
#define POOL_SIZE (CFG_TLBLE_EVT_QUEUE_LENGTH*4U*DIVC(( sizeof(TL_PacketHeader_t) + TL_BLE_EVENT_FRAME_SIZE ), 4U))

/* Private variables ---------------------------------------------------------*/
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t EvtPool[POOL_SIZE];
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static TL_CmdPacket_t SystemCmdBuffer;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t SystemSpareEvtBuffer[sizeof(TL_PacketHeader_t) + TL_EVT_HDR_SIZE + 255U]; //CFG_TLBLE_MOST_EVENT_PAYLOAD_SIZE=16

static __IO uint8_t FLAG_WAIT_CPU2_RDY = 1U;
static __IO uint8_t FLAG_WAIT_RESP = 0U;

/* Private function prototypes -----------------------------------------------*/
static void Tl_Init(void);
static void Reset_IPCC(void);
static SE_ErrorStatus SE_CM0_Update_Start(void);
static SE_ErrorStatus SE_CM0_Update_Continue(void);

/**
  * @brief  Initialization of CM0 communication
  * @note
  * @param  None
  * @retval None
  */
void CM0_Init(void)
{
  Reset_IPCC();
  Tl_Init();
}

/**
  * @brief  DeInitialization of CM0 communication
  * @note
  * @param  None
  * @retval None
  */
void CM0_DeInit(void)
{
  /* If FUS is running : try to start the wireless stack */
  if (SHCI_C2_FUS_GetState(NULL) == FUS_STATE_VALUE_IDLE)
  {
    /* Start WS */
    SHCI_C2_FUS_StartWs();

    /* reset triggered by FUS should occur */
    HAL_Delay(2000U);
  }

  /* Allow the UserApp to fake a set C2BOOT as it has already been set */
  SHCI_C2_Reinit();

}

/**
  * @brief  Transport layer initialization
  * @note
  * @param  None
  * @retval None
  */
static void Tl_Init( void )
{
  TL_MM_Config_t tl_mm_config;
  SHCI_TL_HciInitConf_t SHci_Tl_Init_Conf;
  /**< Reference table initialization */
  TL_Init();

  /**< System channel initialization */
  SHci_Tl_Init_Conf.p_cmdbuffer = (uint8_t*)&SystemCmdBuffer;
  SHci_Tl_Init_Conf.StatusNotCallBack = NULL;
  shci_init(NULL, (void*) &SHci_Tl_Init_Conf);

  /**< Memory Manager channel initialization */
  tl_mm_config.p_SystemSpareEvtBuffer = SystemSpareEvtBuffer;
  tl_mm_config.p_AsynchEvtPool = EvtPool;
  tl_mm_config.AsynchEvtPoolSize = POOL_SIZE;
  TL_MM_Init( &tl_mm_config );

  TL_Enable();

  // Check channel 2
  while(FLAG_WAIT_CPU2_RDY==1);
  return;
}

/**
  * @brief  Reset IPCC
  * @note
  * @param  None
  * @retval None
  */
static void Reset_IPCC( void )
{
  LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_IPCC);

  LL_C1_IPCC_ClearFlag_CHx(
      IPCC,
      LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4
      | LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

  LL_C2_IPCC_ClearFlag_CHx(
      IPCC,
      LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4
      | LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

  LL_C1_IPCC_DisableTransmitChannel(
      IPCC,
      LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4
      | LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

  LL_C2_IPCC_DisableTransmitChannel(
      IPCC,
      LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4
      | LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

  LL_C1_IPCC_DisableReceiveChannel(
      IPCC,
      LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4
      | LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

  LL_C2_IPCC_DisableReceiveChannel(
      IPCC,
      LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4
      | LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

  return;
}

/**
  * @brief  Interrupt service routine called when the system channel
  *         reports a packet has been received
  * @param  pdata Packet
  * @retval None
  */
void shci_notify_asynch_evt(void *pdata)
{
  FLAG_WAIT_CPU2_RDY = 0U;
  return;
}

/**
  * @brief  This function is called when an System HCI command is sent and the response is
  *         received from the CPU2.
  * @param  flag: Release flag
  * @retval None
  */
void shci_cmd_resp_release(uint32_t flag)
{
  FLAG_WAIT_RESP = 1U;
  return;
}

/**
  * @brief  This function is called when an System HCO Command is sent and the response
  *         is waited from the CPU2.
  *         The application shall implement a mechanism to not return from this function
  *         until the waited event is received.
  *         This is notified to the application with shci_cmd_resp_release().
  *         It is called from the same context the System HCI command has been sent.
  * @param  timeout: Waiting timeout
  * @retval None
  */
void shci_cmd_resp_wait(uint32_t timeout)
{
  while ((FLAG_WAIT_RESP & 0x1) == 0U);
  FLAG_WAIT_RESP = 0U;
  return;
}

/**
  * @brief  CM0 update process startup
  *         ==> Force FUS execution on CM0
  * @note
  * @param  None
  * @retval None
  */

/* Place code in a specific section and remove specific warning */
#if defined(__ICCARM__)
#pragma diag_suppress=Pe111
#endif /* __ICCARM__ */


static SE_ErrorStatus SE_CM0_Update_Start(void)
{
  uint8_t fus_state_value;

  /*
   * The CPU2 firmware update procedure is starting from now
   * There may be several device reset during CPU2 firmware upgrade
   * The CM0 state variable in shared memory is set on CM0_UPDATE_TO_BE_STARTED
   *
   * Wireless Firmware upgrade:
   * Once the upgrade is over, the CPU2 will run the wireless stack
   *
   * FUS Firmware Upgrade:
   * Once the upgrade is over, the CPU2 will run FUS and the FUS return the Idle state
   *
   * If stack is running two FUS_GetState consecutive request will force stack to launch FUS
   */

  CM0_UPDATE_PROCESS = CM0_UPDATE_TO_BE_STARTED;
  fus_state_value = SHCI_C2_FUS_GetState(NULL);

  /* Check if FUS or stack is running */
  if (fus_state_value == FUS_STATE_VALUE_ERROR)
  {
    /* stack is running a second FUS_GetState command will force the stack to launch FUS */
    (void) SHCI_C2_FUS_GetState(NULL);

    /* waiting for reset from FUS */
    while(1)
    {
      HAL_Delay(2000U);
      WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
    }
  }
  else
  {
    /* Reset to restart SBSFU */
    NVIC_SystemReset();
  }

  /* Should never reach this code */
  return SE_ERROR;
}

/**
  * @brief  CM0 update process in ptogress
  *         ==> Send FWUpgrade command to FUS
  *         ==> Finally force wireless stack execution on CM0
  * @note
  * @param  None
  * @retval None
  */
static SE_ErrorStatus SE_CM0_Update_Continue(void)
{
  uint8_t fus_state_value;
  SHCI_FUS_GetState_ErrorCode_t error;
  SE_ErrorStatus e_ret_status = SE_ERROR;

  /*
   * The CPU2 firmware update procedure is starting from now
   * There may be several device reset during CPU2 firmware upgrade
   * The CM0 state variable in shared memory is set on CM0_UPDATE_IN_PROGRESS
   *
   * Wireless Firmware upgrade:
   * Once the upgrade is over, the CPU2 will run the wireless stack
   *
   * FUS Firmware Upgrade:
   * Once the upgrade is over, the CPU2 will run FUS and the FUS return the Idle state
   *
   * If stack is running two FUS_GetState consecutive request will force stack to launch FUS
   */

  fus_state_value = SHCI_C2_FUS_GetState(NULL);

  switch(fus_state_value)
  {
    case FUS_STATE_VALUE_ERROR :
      /* stack is running : this is the end of UPGRADE process
         ===================================================== */
      CM0_UPDATE_PROCESS = CM0_UPDATE_DONE;
      STANDALONE_LOADER_STATE = STANDALONE_LOADER_NO_REQ;

      /* Update successful */
      e_ret_status = SE_SUCCESS;
      break;

    case FUS_STATE_VALUE_IDLE :
      /* FUS is running and upgrade not started : Send FW Upgrade command
         ================================================================ */
      if (CM0_UPDATE_PROCESS == CM0_UPDATE_TO_BE_STARTED)
      {
        CM0_UPDATE_PROCESS = CM0_UPDATE_IN_PROGRESS;
        SHCI_C2_FUS_FwUpgrade(0,0);

        /* Wait for reset from FUS  */
        while(1)
        {
          HAL_Delay(4000U);
          WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);

          /* Check if no error detected by FUS like : FUS_STATE_IMG_NOT_FOUND ... */
          fus_state_value = SHCI_C2_FUS_GetState(&error);
          if (error != FUS_STATE_ERROR_NO_ERROR)
          {
            /* error ==> restart in SBSFU mode */
            STANDALONE_LOADER_STATE = STANDALONE_LOADER_NO_REQ;
            e_ret_status = SE_ERROR;
            break;
          }
        }
      }
      /* FUS is running and upgrade started : Restart wireless stack
         =========================================================== */
      else if (CM0_UPDATE_PROCESS == CM0_UPDATE_IN_PROGRESS)
      {
        SHCI_C2_FUS_StartWs();

        /* Wait for reset from FUS : nothing to do */
        while(1)
        {
          HAL_Delay(2000U);
          WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
        }
      }
      /* FUS is running and no upgrade identified : error
         ================================================ */
      else
      {
        /* error ==> restart in SBSFU mode */
        STANDALONE_LOADER_STATE = STANDALONE_LOADER_NO_REQ;

        /* Update failure */
        e_ret_status = SE_ERROR;
      }
      break;

    default :
      /* Upgrade is ON GOING
         =================== */

      /* Wait for reset from FUS  : nothing to do */
      while(1)
      {
        HAL_Delay(2000U);
        WRITE_REG(IWDG->KR, IWDG_KEY_RELOAD);
      }
  }
  return e_ret_status;
}

/**
  * @brief  CM0 update process
  * @note
  * @param  None
  * @retval None
  */
SE_ErrorStatus CM0_Update(void)
{
  SE_ErrorStatus e_ret_status = SE_ERROR;

  /* Enable IPCC interrupt */
  HAL_NVIC_EnableIRQ(IPCC_C1_RX_IRQn);
  HAL_NVIC_EnableIRQ(IPCC_C1_TX_IRQn);

  if (STANDALONE_LOADER_STATE == STANDALONE_LOADER_DWL_REQ)
  {
    /* New download started ==> CM0 process restarted from initial stage */
    CM0_UPDATE_PROCESS = CM0_UPDATE_NOT_INITIALIZED;
    e_ret_status = SE_SUCCESS;
  }
  else if (CM0_UPDATE_PROCESS == CM0_UPDATE_NOT_INITIALIZED)
  {
    /* Force FUS execution on CM0 */
    e_ret_status = SE_CM0_Update_Start();
  }
  else if ((CM0_UPDATE_PROCESS == CM0_UPDATE_TO_BE_STARTED) ||
           (CM0_UPDATE_PROCESS == CM0_UPDATE_IN_PROGRESS))
  {
    /* Send FWUpgrade command to FUS
       And finnaly, force wireless stack execution on CM0 */
    e_ret_status = SE_CM0_Update_Continue();
  }
  else
  {
    e_ret_status = SE_ERROR;
  }

  return e_ret_status;
}
