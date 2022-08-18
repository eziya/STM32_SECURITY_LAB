/*
 * app.c
 *
 *  Created on: 2022. 7. 28.
 *      Author: eziya76@gmail.com
 */

#include "app.h"
#include "ecc_pub_key.h"

//FW address & size
#define FW_START_ADD            (FLASH_BASE)
#define FW_SIZE                 (FW_END - FLASH_BASE)
#define FW_SIZE_PAGE_ALIGNED    (FW_SIZE % FLASH_PAGE_SIZE == 0 ? FW_SIZE : (FW_SIZE/FLASH_PAGE_SIZE+1) * FLASH_PAGE_SIZE)
#define FW_END                  (uint32_t)((uint8_t*)& __FW_SECTION_END)

//hash digest, signature, public key location & size
#define HASH_ADD                (FW_START_ADD + FW_SIZE_PAGE_ALIGNED)
#define HASH_SIZE               (CMOX_SHA256_SIZE)
#define SIG_ADD                 (HASH_ADD + HASH_SIZE)
#define SIG_SIZE                (64U)
#define ECC_PUB_ADD             (SIG_ADD + SIG_SIZE)
#define ECC_PUB_SIZE            (64U)

//WRP offset
#define WRP_START_OFFSET        ((HASH_ADD - FLASH_BASE) / FLASH_PAGE_SIZE)
#define WRP_END_OFFSET          WRP_START_OFFSET

//RDP level
#define RDP_LEVEL_CONFIG        OB_RDP_LEVEL_1

extern UART_HandleTypeDef huart2;
extern uint32_t __FW_SECTION_END;

cmox_ecc_handle_t hECC;     //ECC context handle
uint8_t eccBuffer[2000];    //ECC buffer

static void AppHandleMenu(void);
static void AppPrintMenu(void);
static void AppEnableOBWRP(void);
static void AppEnableOBRDP(void);
static bool SingleCallSHA256(const uint8_t* message, const size_t messageLen, uint8_t* digest, size_t* digestLen);
static bool VerifySignatureWithPubKey(const uint8_t* pubKey, size_t pubKeyLen, const uint8_t* hash, size_t hashLen, const uint8_t* signature, size_t signatureLen);

//verify hash digest
bool AppHashVerify(void)
{
  uint8_t digest[HASH_SIZE];
  size_t digestLen;
  bool result = false;

  printf("Start FW Hash Check...\r\n");
  printf("FW start address: 0x%08lx\r\n", FW_START_ADD);
  printf("FW size: 0x%08lx\r\n", FW_SIZE_PAGE_ALIGNED);
  printf("FW HASH address: 0x%08lx\r\n", HASH_ADD);
  printf("FW HASH SIZE: %u\r\n", HASH_SIZE);

  result = SingleCallSHA256(
      (uint8_t*)FW_START_ADD,
      (uint32_t)FW_SIZE_PAGE_ALIGNED,
      digest,
      &digestLen);

  if(result == true && digestLen == HASH_SIZE)
  {
    printf("FW HASH Result: \r\n");
    printHexArray(digest, sizeof(digest));

    printf("Expected HASH Result: \r\n");
    printHexArray((const uint8_t*)HASH_ADD, HASH_SIZE);

    if (memcmp((const uint8_t*)HASH_ADD, digest, HASH_SIZE) == 0)
    {
      printf("FW Hash check pass.\r\n");
    }
    else
    {
      printf("FW Hash check fail.\r\n");
      return false;
    }
  }
  else
  {
    printf("FW Hash computation fail!\r\n");
    return false;
  }

  return true;
}

//verify signature
bool AppSignatureVerify(void)
{
  printf("Start FW signature check...\r\n");
  printf("FW Signature address: 0x%08lx\r\n", SIG_ADD);
  printf("FW Signature SIZE: %u\r\n", SIG_SIZE);

  printf("FW SIGNATURE data: \r\n");
  printHexArray((const uint8_t*)SIG_ADD, SIG_SIZE);

  //verify public key
  if(memcmp((const uint8_t*)ECC_PUB_ADD, (uint8_t*)SIGN_ECC_PUB_KEY, ECC_PUB_SIZE) != 0)
  {
    printf("Public key comparison failed!\r\n");
    return false;
  }
  else
  {
    printf("ECC Public Key: \r\n");
    printHexArray((const uint8_t*)ECC_PUB_ADD, ECC_PUB_SIZE);
    printf("Public key comparison pass.\r\n");
  }

  //verify signature
  if(!VerifySignatureWithPubKey((const uint8_t*)ECC_PUB_ADD, ECC_PUB_SIZE, (const uint8_t*)HASH_ADD, HASH_SIZE, (const uint8_t*)SIG_ADD, SIG_SIZE))
  {
    printf("Signature verification failed!\r\n");
    return false;
  }
  else
  {
      printf("Signature verification pass.\r\n");
  }

  return true;
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
    //clear data register
    __HAL_UART_FLUSH_DRREGISTER(&huart2);

    HAL_UART_Receive(&huart2, &key, 1U, HAL_MAX_DELAY);

    switch (key)
    {
    case '1' :
      AppEnableOBWRP();
      break;
    case '2' :
      AppEnableOBRDP();
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
  printf("Test Protection: Enable WRP Option Bytes -------------- 1\r\n\n");
  printf("Test Protection: Enable RDP Option Bytes -------------- 2\r\n\n");
  printf("Previous Menu ----------------------------------------- x\r\n\n");
}

//enable WRP protection
static void AppEnableOBWRP(void)
{
  FLASH_OBProgramInitTypeDef optionBytes;
  int32_t isOBChangeToApply = 0;

  //unlock flash & option bytes
  HAL_FLASH_Unlock();
  HAL_FLASH_OB_Unlock();

  //read option bytes
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
  optionBytes.WRPArea = OB_WRPAREA_ZONE_A;
  HAL_FLASHEx_OBGetConfig(&optionBytes);

  if (optionBytes.RDPLevel == OB_RDP_LEVEL_2)
  {
    if((optionBytes.WRPStartOffset > WRP_START_OFFSET) ||
        (optionBytes.WRPEndOffset < WRP_END_OFFSET))
    {
      printf("Unable to write WRP offset when RDP level is 2.\r\n");
      goto ERROR;
    }
  }
  else
  {
    //configure WRP offset
    if((optionBytes.WRPStartOffset > WRP_START_OFFSET) || (optionBytes.WRPEndOffset < WRP_END_OFFSET))
    {
      optionBytes.WRPStartOffset = WRP_START_OFFSET;
      optionBytes.WRPEndOffset = WRP_END_OFFSET;

      printf("\r\nTry to apply WRP from page [%08lx] to [%08lx]\r\n",
          optionBytes.WRPStartOffset,
          optionBytes.WRPEndOffset);

      if(HAL_FLASHEx_OBProgram(&optionBytes) != HAL_OK)
      {
        printf("Fail to program option bytes.\r\n");
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
    printf("Unable to modify RDP level when RDP level is 2.\r\n");
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
        printf("Fail to program option bytes.\r\n");
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

// verify signature
bool VerifySignatureWithPubKey(const uint8_t* pubKey, size_t pubKeyLen, const uint8_t* hash, size_t hashLen, const uint8_t* signature, size_t signatureLen)
{
  cmox_ecc_retval_t retVal;
  uint32_t fault_check = CMOX_ECC_AUTH_FAIL;

  //allocate resources
  cmox_ecc_construct(&hECC, CMOX_ECC256_MATH_FUNCS, eccBuffer, sizeof(eccBuffer));

  //verify signature
  retVal = cmox_ecdsa_verify(
      &hECC,
      CMOX_ECC_CURVE_SECP256R1,   //SECP256R1
      pubKey,                     //public key for verification
      pubKeyLen,                  //public key length
      hash,                       //hash to compare
      hashLen,                    //hash length
      signature,                  //signature to verify
      signatureLen,               //signature length
      &fault_check);              //verification result

  //check the result
  if((retVal != CMOX_ECC_AUTH_SUCCESS) || (fault_check != CMOX_ECC_AUTH_SUCCESS)) return false;

  //clean up resources
  cmox_ecc_cleanup(&hECC);

  return true;
}