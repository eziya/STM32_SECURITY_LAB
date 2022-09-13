/*
 * boot.c
 *
 *  Created on: 2022. 8. 23.
 *      Author: eziya76@gmail.com
 */
#include "boot.h"
#include "ecc_pub_key.h"

MetaBlock *pMetaBlock = (MetaBlock *)META_BLOCK_ADDR;
cmox_ecc_handle_t hECC;     //ECC context handle
uint8_t eccBuffer[2000];    //ECC buffer

static void PrintMetaData(void);
static bool VerifyMetaDataMagicNumber(void);
static bool VerifyMetaDataHash(void);
static bool VerifyMetaDataSignature(void);
static bool VerifyAppHash(void);
static bool SingleCallSHA256(const uint8_t* message, const size_t messageLen, uint8_t* digest, size_t* digestLen);
static bool VerifySignatureWithPubKey(const uint8_t* pubKey, size_t pubKeyLen, const uint8_t* hash, size_t hashLen, const uint8_t* signature, size_t signatureLen);
static bool AppEnableOBSecureMem(void);
static bool AppEnableOBRDP(void);

#define OB_RDP_ENABLE       //for easy debugging, you could disable RDP protection

bool ApplyOptionBytesProtections(void)
{
  printf("\r\n");
  printf("Applying option bytes protections.\r\n");

  if(!AppEnableOBSecureMem())
  {
    printf("AppEnableOBSecureMem failed.\r\n");
    return false;
  }

#ifdef OB_RDP_ENABLE
  if(!AppEnableOBRDP())
  {
    printf("AppEnableOBRDP failed.\r\n");
    return false;
  }
#endif

  return true;
}

bool VerifyApplication(void)
{

  if(cmox_initialize(NULL) != CMOX_INIT_SUCCESS)
  {
    Error_Handler();
  }

  PrintMetaData();

  if(!VerifyMetaDataMagicNumber())
  {
    printf("VerifyMetaDataMagicNumber failed.\r\n");
    return false;
  }

  if(!VerifyMetaDataHash())
  {
    printf("VerifyMetaDataHash failed.\r\n");
    return false;
  }

  if(!VerifyMetaDataSignature())
  {
    printf("VerifyMetaDataSignature failed.\r\n");
    return false;
  }

  if(!VerifyAppHash())
  {
    printf("VerifyAppHash failed.\r\n");
    return false;
  }

  if(cmox_finalize(NULL) != CMOX_INIT_SUCCESS)
  {
    Error_Handler();
  }

  return true;
}

void jump_to_application(void)
{
  typedef  void (*pFunction)(uint32_t a, uint32_t b, uint32_t c);

  pFunction JumpToApplication;
  uint32_t JumpAddress;

  //erase SRAM before you jump to application
  for (uint32_t *pRam = (uint32_t *)BL_SRAM1_START; pRam < (uint32_t *)BL_SRAM1_END; pRam++)
  {
    *pRam = 0U;
  }

  // Jump to ExitSecureMemory function with parameters (Magic Number & Application address)
  // refer to AN2606
  JumpAddress = *(__IO uint32_t*) (BL_EXIT_STICKY + 4);
  JumpToApplication = (pFunction) JumpAddress;
  JumpToApplication(JumpAddress, MAGIC_NUMBER, APP_ADDR);
}

static void PrintMetaData(void)
{
  printf("\r\n");
  printf("Meta data Information...\r\n");
  printf("Meta data @0x%08x:\r\n", META_BLOCK_ADDR);
  printf("Magic number: 0x%08lx\r\n", pMetaBlock->magicNum);
  printf("App size: 0x%08lx\r\n", pMetaBlock->appSize);
  printf("App version: 0x%08lx\r\n", pMetaBlock->appVer);

  printf("\r\n");
  printf("App hash\r\n");
  printHexArray(pMetaBlock->appHash, HASH_LEN);
  printf("Meta data hash\r\n");
  printHexArray(pMetaBlock->metaHash, HASH_LEN);
  printf("Meta data signature\r\n");
  printHexArray(pMetaBlock->metaSig, SIG_LEN);
}

static bool VerifyMetaDataMagicNumber(void)
{
  return (pMetaBlock->magicNum == MAGIC_NUMBER) ? true : false;
}

static bool VerifyMetaDataHash(void)
{
  uint8_t digest[HASH_LEN];
  size_t digestLen;
  bool result = false;

  //calculate the hash of meta(128B) and compare it with the written value.
  result = SingleCallSHA256(
      (uint8_t*)META_BLOCK_ADDR,
      (uint32_t)META_DATA_LEN,
      digest,
      &digestLen);

  if(result == true && digestLen == HASH_LEN)
  {
    printf("\r\n");

    if(memcmp((const uint8_t*)pMetaBlock->metaHash, digest, HASH_LEN) == 0)
    {
      printf("Meta data hash check pass.\r\n");
    }
    else
    {
      printf("Meta data hash check fail.\r\n");
      return false;
    }
  }
  else
  {
    printf("Meta data hash computation fail!\r\n");
    return false;
  }

  return true;
}

static bool VerifyMetaDataSignature(void)
{
  if(!VerifySignatureWithPubKey((const uint8_t*)SIGN_ECC_PUB_KEY, ECC_PUB_LEN,
      (const uint8_t*)pMetaBlock->metaHash, HASH_LEN, (const uint8_t*)pMetaBlock->metaSig, SIG_LEN))
  {
    printf("Meta data authentication failed!\r\n");
    return false;
  }

  printf("Meta data authentication passed.\r\n");
  return true;
}

static bool VerifyAppHash(void)
{
  uint8_t digest[HASH_LEN];
  size_t digestLen;
  bool result = false;

  //calculate the hash of application and compare it with the written value.
  result = SingleCallSHA256(
      (uint8_t*)APP_ADDR,
      (uint32_t)pMetaBlock->appSize,
      digest,
      &digestLen);

  if(result == true && digestLen == HASH_LEN)
  {
    if(memcmp((const uint8_t*)pMetaBlock->appHash, digest, HASH_LEN) == 0)
    {
      printf("Application hash check pass.\r\n");
    }
    else
    {
      printf("Application hash check fail.\r\n");
      return false;
    }
  }
  else
  {
    printf("Application hash computation fail!\r\n");
    return false;
  }

  return true;
}

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

bool VerifySignatureWithPubKey(const uint8_t* pubKey, size_t pubKeyLen, const uint8_t* hash, size_t hashLen, const uint8_t* signature, size_t signatureLen)
{
  cmox_ecc_retval_t retVal;
  uint32_t fault_check = CMOX_ECC_AUTH_FAIL;

  cmox_ecc_construct(&hECC, CMOX_ECC256_MATH_FUNCS, eccBuffer, sizeof(eccBuffer));

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

  if((retVal != CMOX_ECC_AUTH_SUCCESS) || (fault_check != CMOX_ECC_AUTH_SUCCESS))
  {
    cmox_ecc_cleanup(&hECC);
    return false;
  }

  cmox_ecc_cleanup(&hECC);
  return true;
}


static bool AppEnableOBSecureMem(void)
{
  FLASH_OBProgramInitTypeDef optionBytes;

  //unlock flash & option bytes
  HAL_FLASH_Unlock();
  HAL_FLASH_OB_Unlock();

  //get option bytes
  HAL_FLASHEx_OBGetConfig(&optionBytes);

  //if SEC_SIZE is not correct
  if(optionBytes.SecSize != SEC_SIZE)
  {
    optionBytes.OptionType = OPTIONBYTE_SEC;
    optionBytes.SecSize = SEC_SIZE;
    if(HAL_FLASHEx_OBProgram(&optionBytes) != HAL_OK)
    {
      printf("Fail to program option bytes.\r\n");
      HAL_FLASH_OB_Lock();
      HAL_FLASH_Lock();
      return false;
    }
    else
    {
      HAL_FLASH_OB_Launch();
    }
  }

  printf("Secure memory is enabled: [0x%08lx]\r\n", (FLASH_BASE + (optionBytes.SecSize * FLASH_PAGE_SIZE)));

  HAL_FLASH_OB_Lock();
  HAL_FLASH_Lock();

  return true;
}

//enable RDP level1
static bool AppEnableOBRDP(void)
{
  FLASH_OBProgramInitTypeDef optionBytes;

  //unlock flash & option bytes
  HAL_FLASH_Unlock();
  HAL_FLASH_OB_Unlock();

  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
  optionBytes.WRPArea = OB_WRPAREA_ZONE_A;
  HAL_FLASHEx_OBGetConfig(&optionBytes);

  if (optionBytes.RDPLevel == OB_RDP_LEVEL_2)
  {
    printf("Unable to modify RDP level when RDP level is 2.\r\n");
    HAL_FLASH_OB_Lock();
    HAL_FLASH_Lock();
    return false;
  }
  else
  {
    if(optionBytes.RDPLevel != OB_RDP_LEVEL_1)
    {
      optionBytes.OptionType = OPTIONBYTE_RDP | OPTIONBYTE_USER;
      optionBytes.RDPLevel = OB_RDP_LEVEL_1;
      if (HAL_FLASHEx_OBProgram(&optionBytes) != HAL_OK)
      {
        printf("Fail to program option bytes.\r\n");
        HAL_FLASH_OB_Lock();
        HAL_FLASH_Lock();
        return false;
      }
      else
      {
        HAL_FLASH_OB_Launch();
      }
    }
  }

  printf("RDP level is [0x%02lx]\r\n", optionBytes.RDPLevel);

  HAL_FLASH_OB_Lock();
  HAL_FLASH_Lock();

  return true;
}