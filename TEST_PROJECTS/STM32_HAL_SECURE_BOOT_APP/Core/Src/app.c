/*
 * app.c
 *
 *  Created on: 2022. 7. 28.
 *      Author: eziya76@gmail.com
 */

#include "app.h"

#define FW_START_ADD            (FLASH_BASE)
#define FW_SIZE                 (FW_END - FLASH_BASE)
#define FW_SIZE_PAGE_ALIGNED    (FW_SIZE % FLASH_PAGE_SIZE == 0 ? \
                                FW_SIZE : (FW_SIZE/FLASH_PAGE_SIZE+1)*FLASH_PAGE_SIZE)
#define FW_END                  (uint32_t)((uint8_t*)& __FW_SECTION_END)

#define HASH_ADD                (FW_START_ADD + FW_SIZE_PAGE_ALIGNED)
#define HASH_SIZE               (CMOX_SHA256_SIZE)

#define WRP_START_OFFSET        ((HASH_ADD - FLASH_BASE) / FLASH_PAGE_SIZE)
#define WRP_END_OFFSET          WRP_START_OFFSET

#define RDP_LEVEL_CONFIG        OB_RDP_LEVEL_1

extern UART_HandleTypeDef huart2;
extern uint32_t __FW_SECTION_END;

static void AppHandleMenu(void);
static void AppPrintMenu(void);
static void AppReadSecureMem(void);
static void AppEnableOBWRP(void);
static void AppEnableOBRDP(void);
static bool SingleCallSHA256(const uint8_t* message, const size_t messageLen, uint8_t* digest, size_t* digestLen);

//verify hash
void AppHashVerify(void)
{
  uint8_t digest[HASH_SIZE];
  size_t digestLen;
  bool result = false;

  printf("\r\nStart FW Hash Check...\r\n");
  printf("\tFW start address: 0x%08lx\r\n", FW_START_ADD);
  printf("\tFW size: 0x%08lx\r\n", FW_SIZE_PAGE_ALIGNED);
  printf("\tFW HASH address: 0x%08lx\r\n", HASH_ADD);
  printf("\tFW HASH SIZE: %u\r\n", HASH_SIZE);

  result = SingleCallSHA256(
      (uint8_t*)FW_START_ADD,
      (uint32_t)FW_SIZE_PAGE_ALIGNED,
      digest,
      &digestLen);

  if(result == true && digestLen == HASH_SIZE)
  {
    printf("\r\nFW HASH Result: \r\n");
    printHexArray(digest, sizeof(digest));

    printf("\r\nExpected HASH Result: \r\n");
    printHexArray((const uint8_t*)HASH_ADD, HASH_SIZE);

    printf("\r\n");
    if (memcmp((const uint8_t*)HASH_ADD, digest, HASH_SIZE) == 0)
    {
      printf("\r\nFW Hash check pass\r\n");
    }
    else
    {
      printf("\r\nFW Hash check fail\r\n");
      goto ERROR;
    }
  }
  else
  {
    printf("\r\nFW Hash computation fail!\r\n");
    goto ERROR;
  }
  return;

ERROR:
  Error_Handler();
}

//toggle led
void AppRun(void)
{
  AppHandleMenu();

  while(1)
  {
    HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
    HAL_Delay(100);
  }
}

//read & handle uart input
static void AppHandleMenu(void)
{
  uint8_t key = 0U;
  uint8_t exit = 0U;

  AppPrintMenu();

  while(exit == 0U)
  {
    __HAL_UART_FLUSH_DRREGISTER(&huart2);

    HAL_UART_Receive(&huart2, &key, 1U, HAL_MAX_DELAY);

    switch (key)
    {
    case '1' :
      AppReadSecureMem();
      break;
    case '2' :
      AppEnableOBRDP();
      break;
    case '3' :
      AppEnableOBWRP();
      break;
    case 'x' :
      exit = 1U;
      break;
    default:
      printf("Invalid key!\r\n");
      break;
    }

    AppPrintMenu();
  }
}

//show menu
static void AppPrintMenu(void)
{
  printf("\r\n");
  printf("=================== Test Menu ===========================\r\n\n");
  printf("Test Protection: Secure User Memory Read -------------- 1\r\n\n");
  printf("Test Protection: Enable RDP Option Bytes -------------- 2\r\n\n");
  printf("Test Protection: Enable WRP Option Bytes -------------- 3\r\n\n");
  printf("Previous Menu ----------------------------------------- x\r\n\n");
}

//read secure memory
static void AppReadSecureMem(void)
{
  FLASH_OBProgramInitTypeDef optionBytes;

  HAL_FLASH_Unlock();
  HAL_FLASH_OB_Unlock();

  //clear option error
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);

  //read option bytes
  optionBytes.WRPArea = OB_WRPAREA_ZONE_A;
  HAL_FLASHEx_OBGetConfig(&optionBytes);

  HAL_FLASH_OB_Lock();
  HAL_FLASH_Lock();

  //configure secure memory address
  volatile uint32_t *pdata[] = {(uint32_t*)0x08000000, (uint32_t*)0x080061FC};

  //print secure memory information
  printf("====== Test Protection: Secure User Memory =================\r\n\n");
  printf("If the Secure User Memory is enabled you should not be able to read the content.\r\n");
  printf("-- Secure User Area size config [0x%08lx]\r\n", (optionBytes.SecSize * FLASH_PAGE_SIZE) + FLASH_BASE);
  printf("-- Flash CR SEC_PROT bit value: 0x%ld\r\n", (FLASH->CR & 0x10000000)>>28);
  printf("-- Reading from address [0x%08lx], [0x%08lx]\r\n", (uint32_t)pdata[0], (uint32_t)pdata[1]);
  printf("-- [0x%08lx]  [0x%08lx]\r\n", *pdata[0], *pdata[1]);
}

//enable WRP protection
static void AppEnableOBWRP(void)
{
  FLASH_OBProgramInitTypeDef optionBytes;
  int32_t isOBChangeToApply = 0;

  //unlock flash & option bytes
  HAL_FLASH_Unlock();
  HAL_FLASH_OB_Unlock();

  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
  optionBytes.WRPArea = OB_WRPAREA_ZONE_A;
  HAL_FLASHEx_OBGetConfig(&optionBytes);

  if (optionBytes.RDPLevel == OB_RDP_LEVEL_2)
  {
    //you're not able to modify option bytes when RDP is level 2
    if((optionBytes.WRPStartOffset > WRP_START_OFFSET) ||
        (optionBytes.WRPEndOffset < WRP_END_OFFSET))
    {
      goto ERROR;
    }
  }
  else
  {
    if((optionBytes.WRPStartOffset > WRP_START_OFFSET) ||
        (optionBytes.WRPEndOffset < WRP_END_OFFSET))
    {
      optionBytes.WRPStartOffset = WRP_START_OFFSET;
      optionBytes.WRPEndOffset = WRP_END_OFFSET;

      printf("\r\nTry to apply WRP from page [%08lx] to [%08lx]\r\n",
          optionBytes.WRPStartOffset,
          optionBytes.WRPEndOffset);

      if(HAL_FLASHEx_OBProgram(&optionBytes) != HAL_OK)
      {
        goto ERROR;
      }
      isOBChangeToApply++;
    }
    printf("\r\nWRP already applied from page [%08lx] to [%08lx]\r\n",
        optionBytes.WRPStartOffset,
        optionBytes.WRPEndOffset);

    if ( isOBChangeToApply > 0 )
    {
      HAL_FLASH_OB_Launch();
    }
  }

  HAL_FLASH_OB_Lock();
  HAL_FLASH_Lock();

  return;

  ERROR:
  HAL_FLASH_OB_Lock();
  HAL_FLASH_Lock();

  Error_Handler();
}

//enable RDP level1
static void AppEnableOBRDP(void)
{
  FLASH_OBProgramInitTypeDef optionBytes;
  int32_t isOBChangeToApply = 0;

  //unlock flash & option bytes
  HAL_FLASH_Unlock();
  HAL_FLASH_OB_Unlock();

  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
  optionBytes.WRPArea = OB_WRPAREA_ZONE_A;
  HAL_FLASHEx_OBGetConfig(&optionBytes);

  if (optionBytes.RDPLevel == OB_RDP_LEVEL_2)
  {
    //you're not able to modify option bytes when RDP is level 2
    goto ERROR;
  }
  else
  {
    if(optionBytes.RDPLevel != RDP_LEVEL_CONFIG)
    {
      optionBytes.OptionType = OPTIONBYTE_RDP | OPTIONBYTE_USER;
      optionBytes.RDPLevel = RDP_LEVEL_CONFIG;
      if (HAL_FLASHEx_OBProgram(&optionBytes) != HAL_OK)
      {
        goto ERROR;
      }

      printf("\r\nSet RDP to [0x%02lx], please power off and power on again.\r\n", optionBytes.RDPLevel);
      isOBChangeToApply++;
    }
    else
    {
      printf("\r\nRDP level set to [0x%02lx]\r\n", optionBytes.RDPLevel);
    }

    if(isOBChangeToApply > 0)
    {
      HAL_FLASH_OB_Launch();
    }
  }

  HAL_FLASH_OB_Lock();
  HAL_FLASH_Lock();

  return;

  ERROR:
  HAL_FLASH_OB_Lock();
  HAL_FLASH_Lock();

  Error_Handler();
}

//calculate SHA256 hash
static bool SingleCallSHA256(const uint8_t* message, const size_t messageLen, uint8_t* digest, size_t* digestLen)
{
  cmox_hash_retval_t retVal;

  retVal = cmox_hash_compute(
      CMOX_SHA256_ALGO,
      message, messageLen,
      digest,
      CMOX_SHA256_SIZE,
      digestLen);

  if((retVal != CMOX_HASH_SUCCESS) || (*digestLen != CMOX_SHA256_SIZE)) return false;

  return true;
}
