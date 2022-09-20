/**
  ******************************************************************************
  * @file    se_crypto_bootloader.c
  * @author  MCD Application Team
  * @brief   Secure Engine CRYPTO module.
  *          This file provides set of firmware functions to manage SE Crypto
  *          functionalities. These services are used by the bootloader.
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
#include <string.h>               /* added for memcpy */
#include "se_crypto_bootloader.h"
#include "se_low_level.h"         /* required for assert_param */
#include "se_key.h"               /* required to access the keys when not provided as input parameter (metadata
                                     authentication) */
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
#include "mapping_sbsfu.h"
#elif defined (__ICCARM__) || defined(__GNUC__)
#include "mapping_export.h"
#endif /* __ICCARM__ || __GNUC__ */
#include "se_cm0.h"

/** @addtogroup SE Secure Engine
  * @{
  */

/** @addtogroup SE_CORE SE Core
  * @{
  */

/** @defgroup  SE_CRYPTO SE Crypto
  * @brief Crypto services (used by the bootloader and common crypto functions)
  * @{
  */

/** @defgroup  SE_CRYPTO_BOOTLOADER SE Crypto for Bootloader
  * @brief Crypto functions used by the bootloader.
  * @note In this example: \li the AES GCM crypto scheme is supported (@ref SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM),
  *                        \li the ECC DSA without encryption crypto scheme is supported
  *                        \li (@ref SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256),
  *                        \li the ECC DSA+AES-CBC crypto scheme is supported
  *                        \li (@ref SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256).
  * @{
  */

/** @defgroup SE_CRYPTO_BOOTLOADER_Private_Variables Private Variables
  * @{
  */

#if ( (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256) || (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM) )
/** @defgroup SE_CRYPTO_BOOTLOADER_Private_Variables_Symmeric_Key Symmetric Key Handling
  *  @brief Variable(s) used to handle the symmetric key(s).
  *  @note All these variables must be located in protected memory.
  *  @{
  */


static CRYP_HandleTypeDef hcryp1;
static uint32_t m_aSE_AesIV[4]  = {0};
#define HAL_CRYP_TIMEOUT 0xFF
#if (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM)
static uint8_t aTag[SE_TAG_LEN];                        /*<! AES GCM tag to check */
#endif  /* SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM */

/**
  * @}
  */

#endif /* SECBOOT_CRYPTO_SCHEME */


#if ((SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256) || (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256))
/** @defgroup SE_CRYPTO_BOOTLOADER_Private_Variables_SHA256 SHA256 variables
  *  @brief  Secure Hash Algorithm SHA-256.
  *  @note   We do not use local variable(s) because the context is used to store internal states.
  *  @{
  */
static mbedtls_sha256_context m_SHA256ctx;
#define HAL_PKA_TIMEOUT 0x1400
static PKA_HandleTypeDef hpka;

/**
  * @}
  */
#endif /* SECBOOT_CRYPTO_SCHEME */



/**
  * @}
  */

/** @defgroup SE_CRYPTO_BOOTLOADER_Private_Macros Private Macros
  * @{
  */


/**
  * @brief Clean up the RAM area storing the Firmware key.
  *        This applies only to the secret symmetric key loaded with SE_ReadKey().
  */

#define SE_CLEAN_UP_FW_KEY() do { /* do nothing */; } while(0)


/**
  * @brief Clean up the RAM area storing the ECC Public Key.
  *        This applies only to the public asymmetric key loaded with SE_ReadKey_Pub().
  */

#define SE_CLEAN_UP_PUB_KEY() do { /* do nothing */; } while(0)


/**
  * @}
  */

/** @defgroup SE_CRYPTO_BOOTLOADER_Private_Functions Private Functions
  *  @brief These are private functions used internally by the crypto services provided to the bootloader
  *         i.e. the exported functions implemented in this file (@ref SE_CRYPTO_BOOTLOADER_Exported_Functions).
  *  @note  These functions are not part of the common crypto code because they are used by the bootloader services
  *         only.
  * @{
  */

/** @defgroup SE_CRYPTO_BOOTLOADER_Private_Functions_AES AES Functions
  *  @brief Helpers for AES: high level wrappers for the Cryptolib.
  *  @note This group of functions is empty because we do not provide high level wrappers for AES.
  * @{
  */

/* no high level wrappers provided for AES primitives: the Cryptolib APIs are called directly from  the services
   (@ref SE_CRYPTO_BOOTLOADER_Exported_Functions) */

/**
  * @}
  */

/** @defgroup SE_CRYPTO_BOOTLOADER_Private_Functions_Readkey Readkey Functions
  *  @brief Read the relevant AES symmetric key.
  * @{
  */

#if ( (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256) || (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM) )
/**
  * @brief  AES readkey function.
  * @param  pxSE_Metadata: Firmware metadata.
  * @retval none
  */
static void SE_CRYPTO_AES_ReadKey(SE_FwRawHeaderTypeDef *pxSE_Metadata)
{
  uint32_t key = 0U;

  /* Read the AES Symmetric Key :
     - SFU1 / SE_ReadKey_1() ==> read key for SLOT_ACTIVE_1
     - SFU2 / SE_ReadKey_2() ==> read key for SLOT_ACTIVE_2
     - SFU3 / SE_ReadKey_3() ==> read key for SLOT_ACTIVE_3 */
  if (memcmp(pxSE_Metadata->SFUMagic, SFUM_1, strlen(SFUM_1)) == 0)
  {
    key = 0U;
  }
#if (SFU_NB_MAX_ACTIVE_IMAGE > 1U)
  else if (memcmp(pxSE_Metadata->SFUMagic, SFUM_2, strlen(SFUM_2)) == 0)
  {
    key = 1U;
  }
#endif  /* (NB_FW_IMAGES > 1) */
#if (SFU_NB_MAX_ACTIVE_IMAGE > 2U)
  else if (memcmp(pxSE_Metadata->SFUMagic, SFUM_3, strlen(SFUM_3)) == 0)
  {
    key = 2U;
  }
#endif  /* (NB_FW_IMAGES > 2) */
  else
  {
    /* nothing to do */
    return;
  }

  /* Load key from CPU2 */
  HAL_NVIC_EnableIRQ(IPCC_C1_RX_IRQn);
  HAL_NVIC_EnableIRQ(IPCC_C1_TX_IRQn);
  SHCI_C2_FUS_LoadUsrKey(SBSFU_AES_KEY_IDX + key);
}
#endif /* SECBOOT_CRYPTO_SCHEME */


#if ( (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256) || (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256) )
/**
  * @brief  Read public key function.
  * @param  pxSE_Metadata: Firmware metadata.
  * @retval none
  */
static void SE_CRYPTO_ReadKey_Pub(SE_FwRawHeaderTypeDef *pxSE_Metadata, uint8_t *paSE_PubKey)
{
  /* Read the AES Symmetric Key :
     - SFU1 / SE_ReadKey_1() ==> read key for SLOT_ACTIVE_1
     - SFU2 / SE_ReadKey_2() ==> read key for SLOT_ACTIVE_2
     - SFU3 / SE_ReadKey_3() ==> read key for SLOT_ACTIVE_3 */
  if (memcmp(pxSE_Metadata->SFUMagic, SFUM_1, strlen(SFUM_1)) == 0)
  {
    SE_ReadKey_1_Pub(paSE_PubKey);
  }
#if (SFU_NB_MAX_ACTIVE_IMAGE > 1U)
  else if (memcmp(pxSE_Metadata->SFUMagic, SFUM_2, strlen(SFUM_2)) == 0)
  {
    SE_ReadKey_2_Pub(paSE_PubKey);
  }
#endif  /* (NB_FW_IMAGES > 1) */
#if (SFU_NB_MAX_ACTIVE_IMAGE > 2U)
  else if (memcmp(pxSE_Metadata->SFUMagic, SFUM_3, strlen(SFUM_3)) == 0)
  {
    SE_ReadKey_3_Pub(paSE_PubKey);
  }
#endif  /* (NB_FW_IMAGES > 2) */
  else
  {
    /* nothing to do */
    ;
  }
}
#endif /* SECBOOT_CRYPTO_SCHEME */

/**
  * @}
  */

/** @defgroup SE_CRYPTO_BOOTLOADER_Private_Functions_HASH Hash Functions
  *  @brief Hash algorithm(s): high level wrapper(s) for the Cryptolib.
  * @{
  */


#if ( (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256) || (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256) )

static int32_t SE_CRYPTO_SHA256_HASH_DigestCompute(const uint8_t *InputMessage, const int32_t InputMessageLength,
                                                   uint8_t *MessageDigest, int32_t *MessageDigestLength);

/**
  * @brief  SHA256 HASH digest compute example.
  * @param  InputMessage: pointer to input message to be hashed.
  * @param  InputMessageLength: input data message length in byte.
  * @param  MessageDigest: pointer to output parameter that will handle message digest
  * @param  MessageDigestLength: pointer to output digest length.
  * @retval error status: 0 if success
  */
static int32_t SE_CRYPTO_SHA256_HASH_DigestCompute(const uint8_t *InputMessage, const int32_t InputMessageLength,
                                                   uint8_t *MessageDigest, int32_t *MessageDigestLength)
{
  int32_t ret;
  int32_t error_status = -1;

  /*
   * InputMessageLength is never negative (see SE_CRYPTO_Authenticate_Metadata)
   * so the cast to size_t is valid
   */

  /* init of a local sha256 context */
  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);

  ret = mbedtls_sha256_starts_ret(&ctx, 0);   /* 0 for sha256 */

  if (0 == ret)
  {
    ret = mbedtls_sha256_update_ret(&ctx, InputMessage, (size_t)InputMessageLength); /* casting is fine because size_t
                                                                                        is unsigned int in ARM C */

    if (0 == ret)
    {
      ret = mbedtls_sha256_finish_ret(&ctx, MessageDigest);

      if (0 == ret)
      {
        error_status = 0;
        *MessageDigestLength = 32; /* sha256 */
      }
      else
      {
        *MessageDigestLength = 0;
      }
    }
  }

  mbedtls_sha256_free(&ctx);

  return error_status;
}


#endif /* SECBOOT_CRYPTO_SCHEME */

/**
  * @}
  */

/**
  * @}
  */


/** @defgroup SE_CRYPTO_BOOTLOADER_Exported_Functions Exported Functions
  * @brief The implementation of these functions is crypto-dependent (but the API is crypto-agnostic).
  *        These functions use the generic SE_FwRawHeaderTypeDef structure to fill crypto specific structures.
  * @{
  */

/**
  * @brief Secure Engine Encrypt Init function.
  * @param pxSE_Metadata: Firmware metadata.
  * @param SE_FwType: Type of Fw Image.
  *        This parameter can be SE_FW_IMAGE_COMPLETE or SE_FW_IMAGE_PARTIAL.
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_CRYPTO_Encrypt_Init(SE_FwRawHeaderTypeDef *pxSE_Metadata, uint32_t SE_FwType)
{
  SE_ErrorStatus e_ret_status = SE_ERROR;

#if (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM)
  uint32_t i;
  uint32_t j;
#elif ( (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256) || (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256) )
  /* The bootloader does not need the encrypt service in this crypto scheme: reject this request later on */
#else
#error "The current example does not support the selected crypto scheme."
#endif /* SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM */

  UNUSED(SE_FwType);

  /* Check the pointers allocation */
  if (pxSE_Metadata == NULL)
  {
    return e_ret_status;
  }

#if (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM)
  /* AES1 HW peripheral initialization */
  hcryp1.Instance = AES1;

  if (HAL_CRYP_DeInit(&hcryp1) != HAL_OK)
  {
    return SE_ERROR;
  }

  hcryp1.Init.DataType = CRYP_DATATYPE_8B;
  hcryp1.Init.DataWidthUnit = CRYP_DATAWIDTHUNIT_BYTE;
  hcryp1.Init.KeySize = CRYP_KEYSIZE_128B;
  hcryp1.Init.Algorithm = CRYP_AES_GCM_GMAC;
  hcryp1.Init.KeyIVConfigSkip = CRYP_KEYIVCONFIG_ONCE;
  hcryp1.Init.HeaderSize = 0; /* no Additional authenticated data considered */
  hcryp1.Init.pKey = NULL;    /* Key loaded by CPU2 */

  for (i = 0U; i < 4U; i ++)
  {
    j = 4 * i;
    if (i != 3U)
    {
      m_aSE_AesIV[i] = pxSE_Metadata->Nonce[j] << 24    | pxSE_Metadata->Nonce[j + 1] << 16 | \
                       pxSE_Metadata->Nonce[j + 2] << 8 | pxSE_Metadata->Nonce[j + 3];
    }
    else
    {
      m_aSE_AesIV[i] = 0x2;
    }
  }

  hcryp1.Init.pInitVect = m_aSE_AesIV;

  if (HAL_CRYP_Init(&hcryp1) == HAL_OK)
  {
    e_ret_status = SE_SUCCESS;
  }

  /* Read the Symmetric Key */
  SE_CRYPTO_AES_ReadKey(pxSE_Metadata);

#elif ( (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256) || (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256) )
  /* The bootloader does not need the encrypt service in this crypto scheme: reject this request */
  e_ret_status = SE_ERROR;
#else
#error "The current example does not support the selected crypto scheme."
#endif /* SECBOOT_CRYPTO_SCHEME */

  /* Return status*/
  return e_ret_status;
}

/**
  * @brief Secure Engine Header Append function.
  * @param pInputBuffer: pointer to Input Buffer.
  * @param InputSize: Input Size (bytes).
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_CRYPTO_Header_Append(const uint8_t *pInputBuffer, int32_t InputSize)
{
  SE_ErrorStatus e_ret_status = SE_ERROR;
#if (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM)
#endif /* SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM */

  /* Check the pointers allocation */
  if (pInputBuffer == NULL)
  {
    return e_ret_status;
  }

#if (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM)
  /* Unsupported service */
  e_ret_status = SE_ERROR;
#elif ( (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256) || (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256) )
  /* The bootloader does not need this service in this crypto scheme: reject this request */
  e_ret_status = SE_ERROR;

  /* Prevent unused argument(s) compilation warning */
  UNUSED(InputSize);
#else
#error "The current example does not support the selected crypto scheme."
#endif /* SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256 || SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256 */
  /* Return status*/
  return e_ret_status;
} /* SECBOOT_CRYPTO_SCHEME */

/**
  * @brief Secure Engine Encrypt Append function.
  * @param pInputBuffer: pointer to Input Buffer.
  * @param InputSize: Input Size (bytes).
  * @param pOutputBuffer: pointer to Output Buffer.
  * @param pOutputSize: pointer to Output Size (bytes).
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_CRYPTO_Encrypt_Append(const uint8_t *pInputBuffer, int32_t InputSize, uint8_t *pOutputBuffer,
                                        int32_t *pOutputSize)
{
  SE_ErrorStatus e_ret_status = SE_ERROR;
#if (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM)
#endif /* SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM */

  /* Check the pointers allocation */
  if ((pInputBuffer == NULL) || (pOutputBuffer == NULL) || (pOutputSize == NULL))
  {
    return e_ret_status;
  }

#if (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM)
  if (InputSize < 0)
  {
    return (e_ret_status);
  }
  /* else the cast to size_t is valid */

  /* The cast "(size_t *)pOutputSize" is valid because we should not get more bytes than InputSize so this fits in an
     int32_t */
  if (HAL_CRYP_Encrypt(&hcryp1, (uint32_t *)pInputBuffer, InputSize, (uint32_t *)pOutputBuffer,
                       HAL_CRYP_TIMEOUT) == HAL_OK)
  {
    e_ret_status = SE_SUCCESS;
  }

#elif ( (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256) || (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256) )
  /* The bootloader does not need the encrypt service in this crypto scheme: reject this request */
  e_ret_status = SE_ERROR;

  /* Prevent unused argument(s) compilation warning */
  UNUSED(InputSize);
#else
#error "The current example does not support the selected crypto scheme."
#endif /* SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256 || SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256 */

  /* Return status*/
  return e_ret_status;
} /* SECBOOT_CRYPTO_SCHEME */

/**
  * @brief Secure Engine Encrypt Finish function.
  * @param pOutputBuffer: pointer to Output Buffer.
  * @param pOutputSize: pointer to Output Size (bytes).
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_CRYPTO_Encrypt_Finish(uint8_t *pOutputBuffer, int32_t *pOutputSize)
{
  SE_ErrorStatus e_ret_status = SE_ERROR;
#if (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM)
#endif /* SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM */

  /* Check the pointers allocation */
  if ((pOutputBuffer == NULL) || (pOutputSize == NULL))
  {
    /* Clean-up the key in RAM */
    SE_CLEAN_UP_FW_KEY();

    return e_ret_status;
  }

#if (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM)
  /* HAL function call to compute TAG */
  if (HAL_CRYPEx_AESGCM_GenerateAuthTAG(&hcryp1, (uint32_t *)pOutputBuffer, HAL_CRYP_TIMEOUT) == HAL_OK)
  {
    *pOutputSize = SE_TAG_LEN;
    e_ret_status = SE_SUCCESS;
  }
  HAL_CRYP_DeInit(&hcryp1);

#elif ( (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256) || (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256) )
  /* The bootloader does not need the encrypt service in this crypto scheme: reject this request */
  e_ret_status = SE_ERROR;
#else
#error "The current example does not support the selected crypto scheme."
#endif /* SECBOOT_CRYPTO_SCHEME */

  /* Clean-up the key in RAM */
  SE_CLEAN_UP_FW_KEY();

  /* Return status*/
  return e_ret_status;
}

/**
  * @brief Secure Engine Decrypt Init function.
  * @param pxSE_Metadata: Firmware metadata.
  * @param SE_FwType: Type of Fw Image.
  *        This parameter can be SE_FW_IMAGE_COMPLETE or SE_FW_IMAGE_PARTIAL.
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_CRYPTO_Decrypt_Init(SE_FwRawHeaderTypeDef *pxSE_Metadata, uint32_t SE_FwType)
{
  SE_ErrorStatus e_ret_status = SE_ERROR;
#if ( (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256) || (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM) )
  uint32_t i;
  uint32_t j;
#elif (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256)
#else
#error "The current example does not support the selected crypto scheme."
#endif /* SECBOOT_CRYPTO_SCHEME */

#if (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256) || (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256)
  UNUSED(SE_FwType);
#endif /* SECBOOT_CRYPTO_SCHEME */

  /* Check the pointers allocation */
  if (pxSE_Metadata == NULL)
  {
    return SE_ERROR;
  }

#if (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM)
  /* Store FwTag in order to test it during Decrypt_Finish() execution */
  if (SE_FwType == SE_FW_IMAGE_COMPLETE)
  {
    memcpy(aTag, (uint8_t *)pxSE_Metadata->FwTag, SE_TAG_LEN);
  }
  else if (SE_FwType == SE_FW_IMAGE_PARTIAL)
  {
    memcpy(aTag, (uint8_t *)pxSE_Metadata->PartialFwTag, SE_TAG_LEN);
  }
  else
  {
    return SE_ERROR;
  }

  /* AES1 HW peripheral initialization */
  hcryp1.Instance = AES1;

  if (HAL_CRYP_DeInit(&hcryp1) != HAL_OK)
  {
    return SE_ERROR;
  }

  hcryp1.Init.DataType = CRYP_DATATYPE_8B;
  hcryp1.Init.DataWidthUnit = CRYP_DATAWIDTHUNIT_BYTE;
  hcryp1.Init.KeySize = CRYP_KEYSIZE_128B;
  hcryp1.Init.Algorithm = CRYP_AES_GCM_GMAC;
  hcryp1.Init.KeyIVConfigSkip = CRYP_KEYIVCONFIG_ONCE;
  hcryp1.Init.HeaderSize = 0; /* no Additional authenticated data considered */
  hcryp1.Init.pKey = NULL;    /* Key loaded by CPU2 */

  for (i = 0U; i < 4U; i ++)
  {
    j = 4 * i;
    if (i != 3U)
    {
      m_aSE_AesIV[i] = pxSE_Metadata->Nonce[j] << 24    | pxSE_Metadata->Nonce[j + 1] << 16 | \
                       pxSE_Metadata->Nonce[j + 2] << 8 | pxSE_Metadata->Nonce[j + 3];
    }
    else
    {
      m_aSE_AesIV[i] = 0x2;
    }
  }

  hcryp1.Init.pInitVect = m_aSE_AesIV;

  if (HAL_CRYP_Init(&hcryp1) == HAL_OK)
  {
    e_ret_status = SE_SUCCESS;
  }

  /* Read the Symmetric Key */
  SE_CRYPTO_AES_ReadKey(pxSE_Metadata);

#elif (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256)

  /* AES1 HW peripheral initialization */
  hcryp1.Instance = AES1;

  if (HAL_CRYP_DeInit(&hcryp1) != HAL_OK)
  {
    return SE_ERROR;
  }

  hcryp1.Init.DataType = CRYP_DATATYPE_8B;
  hcryp1.Init.DataWidthUnit = CRYP_DATAWIDTHUNIT_BYTE;
  hcryp1.Init.KeySize = CRYP_KEYSIZE_128B;
  hcryp1.Init.Algorithm = CRYP_AES_CBC;
  hcryp1.Init.KeyIVConfigSkip = CRYP_KEYIVCONFIG_ONCE;

  hcryp1.Init.pKey = NULL;  /* Key loaded by CPU2 */
  for (i = 0U; i < 4U; i ++)
  {
    j = 4 * i;
    m_aSE_AesIV[i] = pxSE_Metadata->InitVector[j] << 24    | pxSE_Metadata->InitVector[j + 1] << 16 | \
                     pxSE_Metadata->InitVector[j + 2] << 8 | pxSE_Metadata->InitVector[j + 3];
  }
  hcryp1.Init.pInitVect = m_aSE_AesIV;

  if (HAL_CRYP_Init(&hcryp1) == HAL_OK)
  {
    e_ret_status = SE_SUCCESS;
  }
  else
  {
    e_ret_status = SE_ERROR;
  }

  /* Read the Symmetric Key */
  SE_CRYPTO_AES_ReadKey(pxSE_Metadata);

#elif (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256)
  /* Nothing to do as we won't decrypt anything */
  e_ret_status = SE_SUCCESS;
#else
#error "The current example does not support the selected crypto scheme."
#endif /* SECBOOT_CRYPTO_SCHEME */

  /* Return status*/
  return e_ret_status;
}

/**
  * @brief Secure Engine Decrypt Append function.
  * @param pInputBuffer: pointer to Input Buffer.
  * @param InputSize: Input Size (bytes).
  * @param pOutputBuffer: pointer to Output Buffer.
  * @param pOutputSize: pointer to Output Size (bytes).
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_CRYPTO_Decrypt_Append(const uint8_t *pInputBuffer, int32_t InputSize, uint8_t *pOutputBuffer,
                                        int32_t *pOutputSize)
{
  SE_ErrorStatus e_ret_status = SE_ERROR;

  /* DecryptImageInDwlSlot() always starts by calling the Decrypt service with a 0 byte buffer */
  if (0 == InputSize)
  {
    /* Nothing to do but we must return a success for the decrypt operation to continue */
    return (SE_SUCCESS);
  }

  /* Check the pointers allocation */
  if ((pInputBuffer == NULL) || (pOutputBuffer == NULL) || (pOutputSize == NULL))
  {
    return SE_ERROR;
  }

  if (InputSize < 0)
  {
    *pOutputSize = 0;
    return (SE_ERROR);
  }
  /* else casting to size_t is valid */

#if (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM)
  if (HAL_CRYP_Decrypt(&hcryp1, (uint32_t *)pInputBuffer, InputSize, (uint32_t *)pOutputBuffer,
                       HAL_CRYP_TIMEOUT) == HAL_OK)
  {
    e_ret_status = SE_SUCCESS;
  }

#elif (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256)
  e_ret_status = SE_ERROR;
  /* HAL function call to launch HW decryption */
  if (HAL_CRYP_Decrypt(&hcryp1, (uint32_t *)pInputBuffer, InputSize, (uint32_t *)pOutputBuffer,
                       HAL_CRYP_TIMEOUT) == HAL_OK)
  {
    e_ret_status = SE_SUCCESS;
  }
#elif (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256)
  /*
   * The firmware is not encrypted.
   * The only thing we need to do is to recopy the input buffer in the output buffer
   */
  (void)memcpy(pOutputBuffer, pInputBuffer, (uint32_t)InputSize);
  *pOutputSize = InputSize;
  e_ret_status = SE_SUCCESS;
#else
#error "The current example does not support the selected crypto scheme."
#endif /* SECBOOT_CRYPTO_SCHEME */

  /* Return status*/
  return e_ret_status;
}

/**
  * @brief Secure Engine Decrypt Finish function.
  *        This parameter can be a value of @ref SE_Status_Structure_definition.
  * @param pOutputBuffer: pointer to Output Buffer.
  * @param puOutputSize: pointer to Output Size (bytes).
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_CRYPTO_Decrypt_Finish(uint8_t *pOutputBuffer, int32_t *pOutputSize)
{
  SE_ErrorStatus e_ret_status = SE_ERROR;
#if ( (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256) || (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM) )
#if (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM)
  uint32_t i;
#endif /* SECBOOT_CRYPTO_SCHEME */
#endif /* SECBOOT_CRYPTO_SCHEME */

  /* Check the pointers allocation */
  if ((pOutputBuffer == NULL) || (pOutputSize == NULL))
  {
    /* Clean-up the key in RAM */
    SE_CLEAN_UP_FW_KEY();

    /* clean-up */
#if (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM)
#elif (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256)
#endif /* SECBOOT_CRYPTO_SCHEME */

    return SE_ERROR;
  }

#if (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM)

  /* HAL function call to compute FW TAG */
  if (HAL_CRYPEx_AESGCM_GenerateAuthTAG(&hcryp1, (uint32_t *)pOutputBuffer, HAL_CRYP_TIMEOUT) == HAL_OK)
  {
    e_ret_status = SE_SUCCESS;
    for (i = 0; i < SE_TAG_LEN; i++)
    {
      /*
       * Compare computed FW TAG with FwTag field in FW metadata structure.
       * If the user does not want this check then this function shall be empty.
       *
       * This check is kept here and not moved in the crypto services on SE side
       * because we do not want the SE services to deal with the FLASH mapping.
       */
      if (pOutputBuffer[i] != aTag[i])
      {
        e_ret_status = SE_ERROR;
      }
    }
  }
  HAL_CRYP_DeInit(&hcryp1);
#elif (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256)
  /* HAL function call to deinitialize HW crypto peripheral */
  HAL_CRYP_DeInit(&hcryp1);
  e_ret_status = SE_SUCCESS;
#elif (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256)
  /* Nothing to do */
  e_ret_status = SE_SUCCESS;
#else
#error "The current example does not support the selected crypto scheme."
#endif /* SECBOOT_CRYPTO_SCHEME */

  /* Clean-up the key in RAM */
  SE_CLEAN_UP_FW_KEY();

  /* Return status*/
  return e_ret_status;
}


/**
  * @brief Secure Engine AuthenticateFW Init function.
  *        FW authentication procedure.
  * @param pKey: pointer to the key.
  * @param pxSE_Metadata: Firmware metadata.
  * @param SE_FwType: Type of Fw Image.
  *        This parameter can be SE_FW_IMAGE_COMPLETE or SE_FW_IMAGE_PARTIAL.
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_CRYPTO_AuthenticateFW_Init(SE_FwRawHeaderTypeDef *pxSE_Metadata, uint32_t SE_FwType)
{
  SE_ErrorStatus e_ret_status = SE_ERROR;

  /*
   * Depending on the crypto scheme, the Firmware Tag (signature) can be:
   *   - either an AES GCM tag
   *   - or a SHA256 digest (encapsulated in the authenticated FW metadata)
   */
#if (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM)
  e_ret_status = SE_CRYPTO_Encrypt_Init(pxSE_Metadata, SE_FwType);
#elif ( (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256) || (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256) )
  int32_t ret; /* mbedTLS return code */

  UNUSED(pxSE_Metadata);
  UNUSED(SE_FwType);

  mbedtls_sha256_init(&m_SHA256ctx);

  ret = mbedtls_sha256_starts_ret(&m_SHA256ctx, 0); /* is224:0 for sha256 */

  if (0 == ret)
  {
    e_ret_status = SE_SUCCESS;
  }
  else
  {
    mbedtls_sha256_free(&m_SHA256ctx);
  }
#else
#error "The current example does not support the selected crypto scheme."
#endif /* SECBOOT_CRYPTO_SCHEME */

  /* Return status*/
  return e_ret_status;
}

/**
  * @brief Secure Engine AuthenticateFW Append function.
  * @param pInputBuffer: pointer to Input Buffer.
  * @param InputSize: Input Size (bytes).
  * @param pOutputBuffer: pointer to Output Buffer.
  * @param pOutputSize: pointer to Output Size (bytes).
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_CRYPTO_AuthenticateFW_Append(const uint8_t *pInputBuffer, int32_t InputSize, uint8_t *pOutputBuffer,
                                               int32_t *pOutputSize)
{
  SE_ErrorStatus e_ret_status = SE_ERROR;
  /*
   * Depending on the crypto scheme, the Firmware Tag (signature) can be:
   *   - either an AES GCM tag
   *   - or a SHA256 digest
   */
#if (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM)
  e_ret_status = SE_CRYPTO_Encrypt_Append(pInputBuffer, InputSize, pOutputBuffer, pOutputSize);
#elif ( (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256) || (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256) )
  int32_t ret; /* mbedTLS return code */

  /* The parameters below are useless for the HASH but are needed for API compatibility with other procedures */
  (void)pOutputBuffer;
  (void)pOutputSize;

  if (InputSize < 0)
  {
    return (SE_ERROR);
  }
  /* else the cast to size_t is valid */

  ret = mbedtls_sha256_update_ret(&m_SHA256ctx, pInputBuffer, (size_t)InputSize);

  if (0 == ret)
  {
    e_ret_status = SE_SUCCESS;
  }
#else
#error "The current example does not support the selected crypto scheme."
#endif /* SECBOOT_CRYPTO_SCHEME */

  /* Return status*/
  return e_ret_status;
}

/**
  * @brief Secure Engine AuthenticateFW Finish function.
  * @param pOutputBuffer: pointer to Output Buffer.
  * @param pOutputSize: pointer to Output Size (bytes).
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_CRYPTO_AuthenticateFW_Finish(uint8_t *pOutputBuffer, int32_t *pOutputSize)
{
  SE_ErrorStatus e_ret_status = SE_ERROR;
  /*
   * Depending on the crypto scheme, the Firmware Tag (signature) can be:
   *   - either an AES GCM tag
   *   - or a SHA256 digest
   */
#if (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM)
  e_ret_status = SE_CRYPTO_Encrypt_Finish(pOutputBuffer, pOutputSize);
#elif ( (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256) || (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256) )
  int32_t ret = mbedtls_sha256_finish_ret(&m_SHA256ctx, pOutputBuffer);

  if (0 == ret)
  {
    e_ret_status = SE_SUCCESS;
    *pOutputSize = 32; /* sha256 so 32 bytes */
  }
  else
  {
    *pOutputSize = 0;
  }

  mbedtls_sha256_free(&m_SHA256ctx);
#else
#error "The current example does not support the selected crypto scheme."
#endif /* SECBOOT_CRYPTO_SCHEME */

  /* Return status*/
  return e_ret_status;
}

/**
  * @brief Secure Engine Authenticate Metadata function.
  *        Authenticates the header containing the Firmware metadata.
  * @param pxSE_Metadata: Firmware metadata.
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_CRYPTO_Authenticate_Metadata(SE_FwRawHeaderTypeDef *pxSE_Metadata)
{
  SE_ErrorStatus e_ret_status = SE_ERROR;

#if ( (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256) || (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256) )
  /******************************************************************************/
  /******** Parameters for Elliptic Curve P-256 SHA-256 from FIPS 186-3**********/
  /******************************************************************************/
  /* PKA operation need abs(a) */
  static const uint8_t P_256_absA[] __attribute__((aligned(4))) =
  {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x03
  };
  /* PKA operation need the sign of A */
  static const uint32_t P_256_a_sign = 1;

  static const uint8_t P_256_p[] __attribute__((aligned(4))) =
  {
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
  };
  static const uint8_t P_256_n[] __attribute__((aligned(4))) =
  {
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xBC, 0xE6, 0xFA, 0xAD, 0xA7, 0x17, 0x9E, 0x84, 0xF3, 0xB9,
    0xCA, 0xC2, 0xFC, 0x63, 0x25, 0x51
  };
  static const uint8_t P_256_Gx[] __attribute__((aligned(4))) =
  {
    0x6B, 0x17, 0xD1, 0xF2, 0xE1, 0x2C, 0x42, 0x47, 0xF8, 0xBC, 0xE6, 0xE5, 0x63,
    0xA4, 0x40, 0xF2, 0x77, 0x03, 0x7D, 0x81, 0x2D, 0xEB, 0x33, 0xA0, 0xF4, 0xA1,
    0x39, 0x45, 0xD8, 0x98, 0xC2, 0x96
  };
  static const uint8_t P_256_Gy[] __attribute__((aligned(4))) =
  {
    0x4F, 0xE3, 0x42, 0xE2, 0xFE, 0x1A, 0x7F, 0x9B, 0x8E, 0xE7, 0xEB, 0x4A, 0x7C,
    0x0F, 0x9E, 0x16, 0x2B, 0xCE, 0x33, 0x57, 0x6B, 0x31, 0x5E, 0xCE, 0xCB, 0xB6,
    0x40, 0x68, 0x37, 0xBF, 0x51, 0xF5
  };
  /**
    * Variable used to store the Asymmetric Key inside the protected area (even if this is a public key it is stored
    * like the secret key).
    * Please make sure this variable is placed in protected SRAM1.
    */
  static uint8_t m_aSE_PubKey[SE_ASYM_PUBKEY_LEN];
#endif /* SECBOOT_CRYPTO_SCHEME */

  /**
    * Local variables for authentication procedure.
    */
#if (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM)
  uint32_t i;
  uint32_t j;
  uint8_t fw_tag_output[SE_TAG_LEN];
  uint8_t result = 0x00;
#elif ( (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256) || (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256) )
  int32_t status = 0;
  PKA_ECDSAVerifInTypeDef ECDSA_verif = {0};
  const uint8_t *pPub_x;
  const uint8_t *pPub_y;
  const uint8_t *pSign_r;
  const uint8_t *pSign_s;
  /* Firmware metadata to be authenticated and reference MAC */
  const uint8_t *pPayload;    /* Metadata payload */
  int32_t payloadSize;        /* Metadata length to be considered for hash */
  uint8_t *pSign;             /* Reference MAC (ECCDSA signed SHA256 of the FW metadata) */
  uint8_t MessageDigest[32];      /* The message digest is a sha256 so 32 bytes */
  int32_t MessageDigestLength = 0;
#endif /* SECBOOT_CRYPTO_SCHEME */

#if ( (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256) || (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256) )
  /* the key to be used for crypto operations (as this is a pointer to m_aSE_FirmwareKey or m_aSE_PubKey it can be a
     local variable, the pointed data is protected) */
  uint8_t *pKey;
#endif /* SECBOOT_CRYPTO_SCHEME */

  if (NULL == pxSE_Metadata)
  {
    return SE_ERROR;
  }

#if (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM)

  /* AES1 HW peripheral initialization */
  hcryp1.Instance = AES1;

  if (HAL_CRYP_DeInit(&hcryp1) != HAL_OK)
  {
    return SE_ERROR;
  }

  hcryp1.Init.DataType = CRYP_DATATYPE_8B;
  hcryp1.Init.DataWidthUnit = CRYP_DATAWIDTHUNIT_BYTE;
  hcryp1.Init.KeySize = CRYP_KEYSIZE_128B;
  hcryp1.Init.Algorithm = CRYP_AES_GCM_GMAC;
  hcryp1.Init.HeaderSize = (uint32_t)(SE_FW_AUTH_LEN) / 4;  ; /* Authenticated part of the header */
  hcryp1.Init.Header = (uint32_t *)pxSE_Metadata;
  hcryp1.Size = 0;
  hcryp1.Init.pKey = NULL;    /* Key loaded by CPU2 */

  for (i = 0U; i < 4U; i ++)
  {
    j = 4 * i;
    if (i != 3U)
    {
      m_aSE_AesIV[i] = pxSE_Metadata->Nonce[j] << 24    | pxSE_Metadata->Nonce[j + 1] << 16 | \
                       pxSE_Metadata->Nonce[j + 2] << 8 | pxSE_Metadata->Nonce[j + 3];
    }
    else
    {
      m_aSE_AesIV[i] = 0x2;
    }
  }

  hcryp1.Init.pInitVect = m_aSE_AesIV;

  if (HAL_CRYP_Init(&hcryp1) == HAL_OK)
  {
    /* Read the Symmetric Key */
    SE_CRYPTO_AES_ReadKey(pxSE_Metadata);

    /* Process header */
    if (HAL_CRYP_Encrypt(&hcryp1, NULL, 0, NULL, HAL_CRYP_TIMEOUT) == HAL_OK)
    {
      /* Compute header tag */
      if (HAL_CRYPEx_AESGCM_GenerateAuthTAG(&hcryp1, (uint32_t *)fw_tag_output, HAL_CRYP_TIMEOUT) == HAL_OK)
      {
        e_ret_status = SE_ERROR;
        /*
          * Time-constant comparison done to avoid basic attacks based on time execution
          *
          * This check is kept here and not moved in the crypto services on SE side
          * because we do not want the SE services to deal with the FLASH mapping.
          */
        for (i = 0; i < SE_TAG_LEN; i++)
        {
          result |= fw_tag_output[i] ^ pxSE_Metadata->HeaderSignature[i];
        }
        if (result == 0x00)
        {
          e_ret_status = SE_SUCCESS;
        }
      }
    }
  }
  HAL_CRYP_DeInit(&hcryp1);
#elif ( (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256) || (SECBOOT_CRYPTO_SCHEME == SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256) )
  /* Retrieve the ECC Public Key */
  SE_CRYPTO_ReadKey_Pub(pxSE_Metadata, &m_aSE_PubKey[0U]);
  pKey = &(m_aSE_PubKey[0]);

  /* Set the local variables required to handle the Firmware Metadata during the authentication procedure */
  pPayload = (const uint8_t *)pxSE_Metadata;
  payloadSize = SE_FW_AUTH_LEN; /* Authenticated part of the header */
  pSign = pxSE_Metadata->HeaderSignature;

  /* signature to be verified with r and s components */
  pSign_r = pSign;
  pSign_s = (uint8_t *)(pSign + 32);

  /* Compute the SHA256 of the Firmware Metadata */
  status = SE_CRYPTO_SHA256_HASH_DigestCompute(pPayload,
                                               payloadSize,
                                               (uint8_t *)MessageDigest,
                                               &MessageDigestLength);

  if (0 == status)
  {
    /* Set the local variables dealing with the public key */
    pPub_x  =  pKey;
    pPub_y = (uint8_t *)(pKey + 32);

    /* HAL function call to initialize HW PKA and launch ECDSA verification */
    hpka.Instance = PKA;
    if (HAL_PKA_Init(&hpka) == HAL_OK)
    {
      /* Set input parameters */
      ECDSA_verif.primeOrderSize =  sizeof(P_256_n);
      ECDSA_verif.modulusSize =     sizeof(P_256_p);
      ECDSA_verif.coefSign =        P_256_a_sign;
      ECDSA_verif.coef =            P_256_absA;
      ECDSA_verif.modulus =         P_256_p;
      ECDSA_verif.basePointX =      P_256_Gx;
      ECDSA_verif.basePointY =      P_256_Gy;
      ECDSA_verif.primeOrder =      P_256_n;

      ECDSA_verif.pPubKeyCurvePtX = pPub_x;
      ECDSA_verif.pPubKeyCurvePtY = pPub_y;
      ECDSA_verif.RSign =           pSign_r;
      ECDSA_verif.SSign =           pSign_s;
      ECDSA_verif.hash =            MessageDigest;

      /* Launch the verification */
      if (HAL_PKA_ECDSAVerif(&hpka, &ECDSA_verif, HAL_PKA_TIMEOUT) == HAL_OK)
      {
        /* Compare to expected result */
        if (HAL_PKA_ECDSAVerif_IsValidSignature(&hpka) == 1)
        {
          e_ret_status = SE_SUCCESS;
        }
      }
    }
    /* HAL function call to deinitialize HW PKA peripheral */
    HAL_PKA_DeInit(&hpka);
  } /* else the status is already SE_ERROR */

#else
#error "The current example does not support the selected crypto scheme."
#endif /* SECBOOT_CRYPTO_SCHEME */

  /* Clean-up the key in RAM */
#if (SECBOOT_CRYPTO_SCHEME == SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM)
  /* Symmetric key */
  SE_CLEAN_UP_FW_KEY();
#else
  /* ECC public key */
  SE_CLEAN_UP_PUB_KEY();
#endif /* SECBOOT_CRYPTO_SCHEME */

  /* Return status*/
  return e_ret_status;
}


/**
  * @brief Secure Engine lock and remove the keys from AES HW.
  * @param none.
  * @retval none.
  */
void SE_CRYPTO_Lock_CKS_Keys(void)
{
  /* Lock the key 1 */
  HAL_NVIC_EnableIRQ(IPCC_C1_RX_IRQn);
  HAL_NVIC_EnableIRQ(IPCC_C1_TX_IRQn);
  SHCI_C2_FUS_LockUsrKey(SBSFU_AES_KEY_IDX);
#if (SFU_NB_MAX_ACTIVE_IMAGE > 1U)
  /* Lock the key 2 */
  SHCI_C2_FUS_LockUsrKey(SBSFU_AES_KEY_IDX + 1U);
#endif /* (SFU_NB_MAX_ACTIVE_IMAGE > 1U) */
#if (SFU_NB_MAX_ACTIVE_IMAGE > 2U)
  /* Lock the key 3 */
  SHCI_C2_FUS_LockUsrKey(SBSFU_AES_KEY_IDX + 2U);
#endif /* (SFU_NB_MAX_ACTIVE_IMAGE > 2U) */

  /* Unload the keys by resetting AES1 using AES1RST bit in RCC_AHB2RSTR
     As keys are locked they cannot be re-loaded */
  __HAL_RCC_AES1_FORCE_RESET();
  __HAL_RCC_AES1_RELEASE_RESET();
}
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

