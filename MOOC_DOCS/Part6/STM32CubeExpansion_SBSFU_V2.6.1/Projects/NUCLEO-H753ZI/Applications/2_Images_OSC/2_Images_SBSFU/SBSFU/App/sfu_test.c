/**
  ******************************************************************************
  * @file    sfu_test.c
  * @author  MCD Application Team
  * @brief   Test Protections module.
  *          This file provides set of firmware functions to manage Test Protections
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

/* Includes ------------------------------------------------------------------*/
#include "sfu_test.h"
#include "main.h"
#include "sfu_trace.h"
#include "sfu_low_level_flash_int.h"
#include "sfu_low_level_security.h"
#include "sfu_low_level.h"
#include "string.h" /* required for strncmp */

#if defined(SFU_TEST_PROTECTION)

extern RTC_HandleTypeDef RtcHandle;

/* Private defines -----------------------------------------------------------*/
#define TEST_ISOLATION 1

/* Automatic tests : list of tests (Address, Message, Operation */
/* Flash programming by 256 bits */
#define SHIFT_FLASH_WRITE (32U-1U)

/* Private variables ---------------------------------------------------------*/
static ProtectionTest_t aProtectTests[] =
{
#ifdef SFU_MPU_PROTECT_ENABLE
  {SFU_RAM_BASE, "MPU SRAM1 start add", TEST_EXECUTE},
  {SFU_RAM_END - 3U, "MPU SRAM1 end add", TEST_EXECUTE},             /* -3 : previous 32 bits for execute test */
  {PERIPH_BASE, "Peripheral base add", TEST_EXECUTE},
  {((uint32_t)0x5FFFFFFF - 3U), "Peripheral end address", TEST_EXECUTE},    /* -3 : previous 32 bits for execute test */
  /* READ_FLASH is possible only in privileged mode */
  {INTVECT_START, "Init. Vector", TEST_READ_FLASH},
  {SLOT_DWL_1_START, "MPU code dwl slot1 begin add", TEST_EXECUTE},
  {SLOT_DWL_1_END - 3U, "MPU code dwl slot1 end add", TEST_EXECUTE},   /* -3 : alignment for 32bits execute test */
  {SLOT_DWL_2_START, "MPU code dwl slot2 begin add", TEST_EXECUTE},
  {SLOT_DWL_2_END - 3U, "MPU code dwl slot2 end add", TEST_EXECUTE},   /* -3 : alignment for 32bits execute test */
  {SLOT_DWL_3_START, "MPU code dwl slot3 begin add", TEST_EXECUTE},
  {SLOT_DWL_3_END - 3U, "MPU code dwl slot3 end add", TEST_EXECUTE},   /* -3 : alignment for 32bits execute test */
  {SWAP_START, "MPU code swap begin add", TEST_EXECUTE},
  {SWAP_END - 3U, "MPU code swap end add", TEST_EXECUTE},       /* -3 : alignment for 32bits execute test */
  {SLOT_ACTIVE_1_START, "MPU code active slot1 begin add", TEST_EXECUTE},
  {SLOT_ACTIVE_1_END - 3U, "MPU code active1 slot end add", TEST_EXECUTE}, /* -3 : alignment for 32bits execute test */
  {SLOT_ACTIVE_2_START, "MPU code active slot2 begin add", TEST_EXECUTE},
  {SLOT_ACTIVE_2_END - 3U, "MPU code active slot2 end add", TEST_EXECUTE}, /* -3 : alignment for 32bits execute test */
  {SLOT_ACTIVE_3_START, "MPU code active slot3 begin add", TEST_EXECUTE},
  {SLOT_ACTIVE_3_END - 3U, "MPU code active slot3 end add", TEST_EXECUTE}, /* -3 : alignment for 32bits execute test */
#endif /* SFU_MPU_PROTECT_ENABLE */
#ifdef SFU_WRP_PROTECT_ENABLE
  {INTVECT_START, "WRP start add", TEST_WRITE_FLASH},
  {SFU_ROM_ADDR_END - SHIFT_FLASH_WRITE, "WRP end add", TEST_WRITE_FLASH}, /* -7 : alignment for 64bits writing test */
  {SB_REGION_ROM_START, "WRP internal (SB) add", TEST_ERASE_FLASH},
#endif /* SFU_WRP_PROTECT_ENABLE */
#ifdef SFU_PCROP_PROTECT_ENABLE
  {SE_KEY_REGION_ROM_START, "PCROP start add", TEST_WRITE_FLASH},
  {SE_KEY_REGION_ROM_END - SHIFT_FLASH_WRITE, "PCROP end add", TEST_WRITE_FLASH}, /* -7 : alignment for 64bits
                                                                                         writing test */
  {SE_KEY_REGION_ROM_START, "PCROP start add", TEST_READ_FLASH},
  {SE_KEY_REGION_ROM_END, "PCROP end add", TEST_READ_FLASH},
#endif /* SFU_PCROP_PROTECT_ENABLE */
#if defined(TEST_ISOLATION)
  {SE_CODE_REGION_ROM_START, "Isolated code start add", TEST_WRITE_FLASH},        /* WRP protection applies  */
  {SE_CODE_REGION_ROM_END - SHIFT_FLASH_WRITE, "Isolated code end add", TEST_WRITE_FLASH},/* -7 : alignment for
                                                                         64bits writing test + WRP protection applies */
  {SE_CODE_REGION_ROM_START, "Isolated code start add", TEST_READ_FLASH},
  {SE_KEY_REGION_ROM_START, "Isol KeyConst start addr", TEST_READ_FLASH},
  {SE_KEY_REGION_ROM_END, "Isol KeyConst end addr", TEST_READ_FLASH},
  {SE_CODE_REGION_ROM_END, "Isolated code end add", TEST_READ_FLASH},
  {SE_IF_REGION_ROM_START - 1U, "Isolated MPU region last add", TEST_READ_FLASH}, /* last protected add as the
                                                                           isolated region is bigger than the SE code */
  {SE_STARTUP_REGION_ROM_START + 1U, "Isolated startup add", TEST_EXECUTE}, /* +1 : contains the SE startup code
                                                                    that initializes all the variables in the binary. */
  {SE_CODE_REGION_ROM_START, "Isolated code start add", TEST_EXECUTE},
  {SE_CODE_REGION_ROM_END - 3U, "Isolated code end add", TEST_EXECUTE}, /* -3 : alignment for 32bits execute test */
  {SE_REGION_RAM_START, "Isolated Vdata add", TEST_WRITE_RAM},
  {SE_REGION_RAM_END - 3U, "Isolated Vdata add", TEST_WRITE_RAM},     /* -3 : alignment for 32bits writing test */
  {SE_REGION_RAM_START, "Isolated Vdata add", TEST_READ_RAM},
  {SE_REGION_RAM_END, "Isolated Vdata add", TEST_READ_RAM},
  {SE_REGION_RAM_START, "Isolated Vdata add", TEST_EXECUTE},
  {SE_REGION_RAM_END - 3U, "Isolated Vdata add", TEST_EXECUTE},       /* -3 : alignment for 32bits writing test */
  /*
   * The H7/G0/G4 does not protect the active slot headers with isolation.
   * So the active slot headers is not protected against SB_SFU.
   * But, it is protected from the user application thanks to the secure user memory.
   */
  /*
   * On H7, with RSS V2.0, SWAP bit is always disable when secure memory set.
   * We do not protect the symmetric @ of the bank 1 protected regions.
   * ==> tests removed.
   */
#endif /* TEST_ISOLATION  */
#ifdef SFU_MPU_PROTECT_ENABLE
  /* Testing the MPU protection of the OBs */
  /*
   * OBs are not mapped to any memory address and can be accessed only through the Flash register interface.
   * No testing required.
   */
#endif /* SFU_MPU_PROTECT_ENABLE  */
  /*  Check RCC access not possible  */
  {RCC_BASE, "RCC", TEST_WRITE_RAM},
  {RCC_BASE + 1020U, "RCC end", TEST_WRITE_RAM},
  /*  Check ITCM RAM not executable , not readable */
  {D1_ITCMRAM_BASE, "0x0 ITCM RAM", TEST_EXECUTE},
  {D1_ITCMRAM_BASE, "0x0 ITCM RAM", TEST_READ_RAM},
  {D1_ITCMICP_BASE, "0x3FFF ITCM RAM", TEST_EXECUTE},
  {D1_ITCMICP_BASE, "0x3FFF ITCM RAM", TEST_READ_RAM},
  {0xAAAAAAAA, "Execution successful", TEST_END},
};


/* Automatic test : list of operation */
uint8_t aTestOperation[][20] =
{
  "write 32 bytes",
  "read 1 byte",
  "read 1 byte",
  "erase 512 bytes",
  "write 4 bytes",
  "read 1 byte",
  "execute",
  "execute",
  "end"
};

/* Private function prototypes -----------------------------------------------*/
static void SFU_TEST_Protection(void);

/* Functions Definition ------------------------------------------------------*/
/**
  * @brief  Automatic test of PCROP/WRP/FWALL/MPU protections
  * @param  None.
  * @retval None.
  */
static void SFU_TEST_Protection(void)
{
  uint32_t  offset = 0;
  uint32_t test_idx;
  uint32_t flashErrCode = 0;
  const uint8_t pattern[]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                              0, 0
                             };
  uint32_t page_error = 0U;
  FLASH_EraseInitTypeDef p_erase_init;
  TestStatus status = TEST_IN_PROGRESS;
  __IO uint8_t tmp;
  void (*func)(void);

  /*
    * Now checking the security
    * We will try to access protected @ and run protected code.
    * Each time a a flash error will be raised or a reset will be generated by IP protection.
    * At next boot we read the backup register to check the next @ until all addresses have been verified.
    * WatchDog should be disabled : while(1) loop in case HardFault when trying to execute code @ address protected by
    * FWALL/Code Isolation
    */

  do
  {
    /* slow down execution */
    HAL_Delay(100);

    /* Increment test number for next execution */
    test_idx = HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR1);
    HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR1, (test_idx + 1));
    offset = aProtectTests[test_idx].type == TEST_READ_FLASH_BANK2 ? (FLASH_BANK_SIZE) : 0;
    printf("\r\n= [TEST] %s @ %s %08x", aTestOperation[aProtectTests[test_idx].type], aProtectTests[test_idx].msg,
           aProtectTests[test_idx].address + offset);

    /* Remove test for slots not configured (taking into accounbt SFU_IMG_IMAGE_OFFSET) */
    if ((aProtectTests[test_idx].address != 0U) && (aProtectTests[test_idx].address < 0xFFFFFF00U)
        && (aProtectTests[test_idx].address > 0x00000FFFU))
    {
      switch (aProtectTests[test_idx].type)
      {
        /* Trying to write 64bits in FLASH : WRP flag is set for WRP or PCROP protected area
           or reset generated if under FWALL or MPU protection */
        case TEST_WRITE_FLASH :
          HAL_FLASH_Unlock();
          HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, aProtectTests[test_idx].address, (uint32_t) pattern);
          HAL_FLASH_Lock();
          flashErrCode = HAL_FLASH_GetError();
          printf("\r\nflash error code: %x", flashErrCode);
          if ((flashErrCode & HAL_FLASH_ERROR_WRP) == 0U)
          {
            status = TEST_ERROR;
          }

          /*
           * OBs protection test: MPU generates a MEMORY FAULT and a RESET
           * So the test is FAILED if we reach this line
           */
          if (0 == strncmp("OBs @", (const char *)aProtectTests[test_idx].msg, 5))
          {
            status = TEST_ERROR;
          }
          /* else not an OB protection test so probably not an error */
          break;

          /* Trying to read in FLASH : RDP set in case of PCROP protected area
             or reset generated if under FWALL or MPU protection */
        case TEST_READ_FLASH_BANK2:
        case TEST_READ_FLASH :
          tmp = *(uint8_t *)(aProtectTests[test_idx].address + offset);
          HAL_Delay(1);                                                 /* ensure Flag is set */
          if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_RDPERR_BANK1 | FLASH_FLAG_RDPERR_BANK2) == 0)
          {
            printf(" value : %d", tmp);
            status = TEST_ERROR;
          }
          __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_RDPERR_BANK1 | FLASH_FLAG_RDPERR_BANK2);
          /*
           * When Isolation activated : RESET should be generated
           * So the test is FAILED if we reach this line
           */
          if (0 == strncmp("Isolated", (const char *)aProtectTests[test_idx].msg, 8))
          {
            status = TEST_ERROR;
          }

          break;

        /* Trying to erase 512 bytes in FLASH : WRP flag set for WRP or PCROP protected area
           or reset generated if under FWALL or MPU protection */
        case TEST_ERASE_FLASH :
          HAL_FLASH_Unlock();
          p_erase_init.Banks       = SFU_LL_FLASH_INT_GetBank(aProtectTests[test_idx].address);
          p_erase_init.TypeErase    = FLASH_TYPEERASE_SECTORS;
          p_erase_init.VoltageRange = FLASH_VOLTAGE_RANGE_4;
          p_erase_init.Sector       = SFU_LL_FLASH_GetSector(aProtectTests[test_idx].address);
          p_erase_init.NbSectors    = 1;
          HAL_FLASHEx_Erase(&p_erase_init, &page_error);
          HAL_FLASH_Lock();
          if ((HAL_FLASH_GetError() & HAL_FLASH_ERROR_WRP) == 0U)
          {
            status = TEST_ERROR;
          }
          break;

        /* Trying to write in RAM : reset generated if under FWALL or MPU protection */
        case TEST_WRITE_RAM :
          *(uint32_t *)aProtectTests[test_idx].address = 0x00000000;
          status = TEST_ERROR;
          break;

        /* Trying to read in RAM : reset generated if under FWALL or MPU protection */
        case TEST_READ_RAM :
          tmp = *(uint8_t *)aProtectTests[test_idx].address;
          printf(" value : %d ", tmp);
          status = TEST_ERROR;
          break;

          /* Tryning to execute code : reset generated if under FWALL or MPU protection */
        case TEST_EXECUTE_BANK2:
        case TEST_EXECUTE :
          func = (void(*)(void))(aProtectTests[test_idx].address + offset);
          func();
          status = TEST_ERROR;
          break;

        /* End of execution */
        case TEST_END :
          status = TEST_COMPLETED;
          break;

        default :
          break;
      }
    }
  } while (status == TEST_IN_PROGRESS) ;

  if (status == TEST_ERROR)
  {
    SFU_TEST_Error();
  }
  else if (status == TEST_COMPLETED)
  {
    /* reset for next run (need a RESET of the platform to restart a session) */
    printf("\b\b\b\b\b\b\b\b");
    printf("\r\nYou can RESET the platform to restart the tests.\r\n");
    HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR0, MAGIC_TEST_DONE);              /* no more test in progress */
  }
}

/**
  * @brief  Initialization of backup register and start test
  * @param  None.
  * @retval None.
  */
void SFU_TEST_Init(void)
{
  /* Use register DR0 to know which if the test sequence is in progress or done */
  /* Use register DR1 to know which test to run after a reset */

  if (MAGIC_TEST_DONE == HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR0))      /* Test done */
  {
    return;
  }
  else if (MAGIC_TEST_INIT != HAL_RTCEx_BKUPRead(&RtcHandle,
                                                 RTC_BKP_DR0)) /* Test not done nor in progress: Init magic (DR0) and
                                                                  reset test index (DR1) */
  {
    HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR0, MAGIC_TEST_INIT);
    HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR1, 0x0000);
  }

  /* Start test */
  SFU_TEST_Protection();
}

/**
  * @brief  Reset backup register to be able to execute a new test sequence
  * @param  None.
  * @retval None.
  */
void SFU_TEST_Reset(void)
{
  HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR0, MAGIC_TEST_INIT);
  HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR1, 0x0000);
}

/**
  * @brief  Error detected during test sequence
  * @param  None.
  * @retval None.
  */
void SFU_TEST_Error(void)
{
  printf(" ===> KO!!!!!");
  HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR0, MAGIC_TEST_DONE);
}

#endif /* SFU_TEST_PROTECTION */
