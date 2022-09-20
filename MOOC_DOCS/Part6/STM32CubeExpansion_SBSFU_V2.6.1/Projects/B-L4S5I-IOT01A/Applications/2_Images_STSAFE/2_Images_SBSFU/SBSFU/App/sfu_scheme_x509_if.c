/**
  ******************************************************************************
  * @file    sfu_scheme_x509_if.c
  * @author  MCD Application Team
  * @brief   X509 ECDSA Scheme Interface module.
  *          This file provides set of firmware functions to interface the sbsfu
  *          state machine with the crypto scheme.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file in
  * the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "se_crypto_config.h"
#if (SECBOOT_CRYPTO_SCHEME == SECBOOT_X509_ECDSA_WITHOUT_ENCRYPT_SHA256)

/* Includes ------------------------------------------------------------------*/
#include <string.h>               /* added for memcpy */
#include "se_def.h"
#include "se_interface_bootloader.h"
#include "tkms.h"
#include "se_interface_kms.h"
#include "sfu_trace.h"
#include "sfu_scheme_x509_config.h"
#include "sfu_scheme_x509_core.h"
#include "mbedtls/x509.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/sha256.h"
#include "mbedtls/platform.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t SchemeInitialized = 0;
#if defined(SBSFU_X509_USE_PKCS11DIGEST)
static CK_SESSION_HANDLE session = (CK_SESSION_HANDLE) NULL;
#else
static mbedtls_sha256_context *pSHA256_ctx = NULL;
#endif /* SBSFU_X509_USE_PKCS11DIGEST */

/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

/**
  * @brief Firmware Authentication Init function.
  * @param peSE_Status Secure Engine Status.
  *        This parameter can be a value of @ref SE_Status_Structure_definition.
  * @param pxSE_Metadata pointer to Firmware header structure
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_AuthenticateFW_Init(SE_StatusTypeDef *peSE_Status, SE_FwRawHeaderTypeDef *pxSE_Metadata,
                                      uint32_t SE_FwType)
{
  SE_ErrorStatus e_ret_status = SE_SUCCESS;
  *peSE_Status = SE_OK;

#if !defined(SBSFU_X509_USE_PKCS11DIGEST)
  /*
   * Check context is not in use
   * note: because these api's do not pass a context around, we can't run multiple
   * hashes at the same time
   */
  if (NULL != pSHA256_ctx)
  {
    e_ret_status = SE_ERROR; /* context may be in use, error */
  }

  if (SE_SUCCESS == e_ret_status)
  {
    pSHA256_ctx = calloc(sizeof(mbedtls_sha256_context), 1);
    if (NULL == pSHA256_ctx)
    {
      e_ret_status = SE_ERROR; /* calloc failed */
    }
  }

  if (SE_SUCCESS == e_ret_status)
  {
    mbedtls_sha256_init(pSHA256_ctx);
    if (0 != mbedtls_sha256_starts_ret(pSHA256_ctx, 0))
    {
      e_ret_status = SE_ERROR; /* !=0 => error */
    }
  }

  /*
   * if there was an error cleanup
   */
  if (SE_ERROR == e_ret_status)
  {
    if (NULL != pSHA256_ctx)
    {
      mbedtls_sha256_free(pSHA256_ctx);
      free(pSHA256_ctx);
    }
  }
  if (SE_ERROR == e_ret_status)
  {
#if defined(SFU_X509_VERBOSE_DEBUG_MODE)
    TRACE("\n\r= [SBOOT] SE_AuthenticateFW_Init FAILED");
#endif /* SFU_X509_VERBOSE_DEBUG_MODE */
    /* This is a hack, we are pretending to be the SE because the partitioning of the SBSFU code is not ideal */
    *peSE_Status = SE_INIT_ERR;
  }
#else
  /*
   * Use PKCS11
   */
  CK_FLAGS session_flags = CKF_SERIAL_SESSION;  /* Read ONLY session */
  CK_MECHANISM smech;
  CK_RV rv;

  if ((CK_SESSION_HANDLE) NULL != session)
  {
    e_ret_status = SE_ERROR; /* session may already be in started */
  }

  if (SE_SUCCESS == e_ret_status)
  {
    C_Initialize(NULL);
  }

  if (SE_SUCCESS == e_ret_status)
  {
    rv = C_OpenSession(0,  session_flags, NULL, 0, &session);
    if (rv == CKR_OK)
    {
      smech.pParameter = NULL;
      smech.ulParameterLen = 0;
      smech.mechanism = CKM_SHA256;
      rv = C_DigestInit(session, &smech);
    }
    if (rv == CKR_OK)
    {
      e_ret_status = SE_SUCCESS;
    }
    else
    {
      e_ret_status = SE_ERROR;
    }
  }
#endif /* SBSFU_X509_USE_PKCS11DIGEST */
  return e_ret_status;
}

/**
  * @brief Firmware Authentication Append function.
  * @param peSE_Status Secure Engine Status.
  *        This parameter can be a value of @ref SE_Status_Structure_definition.
  * @param pInputBuffer pointer to Input Buffer.
  * @param uInputSize Input Size (bytes).
  * @param pOutputBuffer pointer to Output Buffer.
  * @param puOutputSize pointer to Output Size (bytes).
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_AuthenticateFW_Append(SE_StatusTypeDef *peSE_Status, const uint8_t *pInputBuffer, int32_t InputSize,
                                        uint8_t *pOutputBuffer, int32_t *pOutputSize)
{
  SE_ErrorStatus e_ret_status = SE_SUCCESS;

#if !defined(SBSFU_X509_USE_PKCS11DIGEST)
  /*
   * Use mbedTLS digest
   */
  /*
   * check context
   */
  if (NULL == pSHA256_ctx)
  {
    e_ret_status = SE_ERROR; /* Can't append without a context */
  }
  /*
   * Check Parameters
   */
  if ((NULL == pInputBuffer) || (0 == InputSize))
  {
    e_ret_status = SE_ERROR; /* bad params */
  }

  if (SE_SUCCESS == e_ret_status)
  {
    if (0 != mbedtls_sha256_update_ret(pSHA256_ctx, pInputBuffer, InputSize))
    {
      e_ret_status = SE_ERROR; /* Update failed */
    }
  }
#else
  /*
   * Use PKCS11 digest
   */
  CK_RV rv = CKR_OK;
  if ((CK_SESSION_HANDLE) NULL == session)
  {
    rv = CKR_SESSION_CLOSED;
  }

  if (CKR_OK == rv)
  {
    rv = C_DigestUpdate(session, (CK_BYTE_PTR) pInputBuffer, InputSize);
  }

  if (CKR_OK == rv)
  {
    e_ret_status = SE_SUCCESS;
  }
  else
  {
    e_ret_status = SE_ERROR;
  }
#endif /* SBSFU_X509_USE_PKCS11DIGEST */

  if (SE_ERROR == e_ret_status)
  {
#if defined(SFU_X509_VERBOSE_DEBUG_MODE)
    TRACE("\n\r= [SBOOT] SE_AuthenticateFW_Append FAILED");
#endif /* SFU_X509_VERBOSE_DEBUG_MODE */
  }
  return e_ret_status;
}

/**
  * @brief Firmware Authentication Finish function.
  *        It is a wrapper of AuthenticateFW_Finish function included in the Firewall.
  * @param peSE_Status Secure Engine Status.
  *        This parameter can be a value of @ref SE_Status_Structure_definition.
  * @param pOutputBuffer pointer to Output Buffer.
  * @param puOutputSize pointer to Output Size (bytes).
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_AuthenticateFW_Finish(SE_StatusTypeDef *peSE_Status, uint8_t *pOutputBuffer, int32_t *pOutputSize)
{
  SE_ErrorStatus e_ret_status = SE_SUCCESS;
  *peSE_Status = SE_OK;

#if !defined(SBSFU_X509_USE_PKCS11DIGEST)
  /*
   * check context
   */
  if (NULL == pSHA256_ctx)
  {
    e_ret_status = SE_ERROR; /* No valid context */
  }
  /*
   * check parameters
   */
  if ((NULL == pOutputBuffer) || (*pOutputSize != 32))
  {
    e_ret_status = SE_ERROR; /* Bad parameters */
  }

  if (e_ret_status == SE_SUCCESS)
  {
    /*
     * return hash
     */
    if (0 != mbedtls_sha256_finish_ret(pSHA256_ctx, pOutputBuffer))
    {
      e_ret_status = SE_ERROR; /* Hash finish failed */
    }
  }
  /*
   * cleanup context
   */
  if (NULL != pSHA256_ctx)
  {
    mbedtls_sha256_free(pSHA256_ctx);
    free(pSHA256_ctx);
    pSHA256_ctx = NULL;
  }

#else
  CK_RV rv = CKR_OK;
  if ((CK_SESSION_HANDLE) NULL == session)
  {
    rv = CKR_SESSION_CLOSED; /* No session open */
  }
  /*
   * check parameters
   */
  if ((NULL == pOutputBuffer) || (*pOutputSize != 32))
  {
    rv = CKR_ARGUMENTS_BAD; /* Bad parameters */
  }

  if (CKR_OK == rv)
  {
    rv = C_DigestFinal(session, (CK_BYTE_PTR)pOutputBuffer, (CK_ULONG *) pOutputSize);
  }

  if ((CK_SESSION_HANDLE) NULL != session)
  {
    C_CloseSession(session); /* Close the session */
    session = (CK_SESSION_HANDLE) NULL;
  }
  C_Finalize(NULL);

  if (CKR_OK == rv)
  {
    e_ret_status = SE_SUCCESS;
  }
  else
  {
    e_ret_status = SE_ERROR;
  }

#endif /* SBSFU_X509_USE_PKCS11DIGEST */

  if (SE_ERROR == e_ret_status)
  {
#if defined(SFU_X509_VERBOSE_DEBUG_MODE)
    TRACE("\n\r\n\r= [SBOOT] SE_AuthenticateFW_Finish FAILED");
#endif /* SFU_X509_VERBOSE_DEBUG_MODE */
  }

  return e_ret_status;
}



/**
  * @brief Verify Raw Fw Header Tag.
  *        It verifies the signature of a firmware header.
  * @param peSE_Status Secure Engine Status.
  *        This parameter can be a value of @ref SE_Status_Structure_definition.
  * @param pFwRawHeader pointer to RawHeader Buffer.
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_VerifyHeaderSignature(SE_StatusTypeDef *peSE_Status, SE_FwRawHeaderTypeDef *pxFwRawHeader)
{
  /* Certs - may be null if not loaded previously */
  SE_ErrorStatus e_ret_status = SE_SUCCESS;
  *peSE_Status = SE_OK;

#if defined(SFU_VERBOSE_DEBUG_MODE)
  TRACE("\n\r= [SBOOT] SE_VerifyHeaderSignature called");
#endif /* SFU_VERBOSE_DEBUG_MODE */

  if (0 == SchemeInitialized)
  {
#if defined(SFU_VERBOSE_DEBUG_MODE)
    TRACE("\n\r= [SBOOT] Initializing x509 scheme");
#endif /* SFU_VERBOSE_DEBUG_MODE */

    if (SFU_SCHEME_X509_CORE_Init(pxFwRawHeader) == SFU_SUCCESS)
    {
      SchemeInitialized = 1;
#if defined(SFU_VERBOSE_DEBUG_MODE)
      TRACE("\n\r= [SBOOT] Initializing x509 scheme OK");
#endif /* SFU_VERBOSE_DEBUG_MODE */
      e_ret_status = SE_SUCCESS;
    }
    else
    {
      SchemeInitialized = 0;
#if defined(SFU_X509_VERBOSE_DEBUG_MODE)
      TRACE("\n\r= [SBOOT] Initializing x509 scheme FAILED");
#endif /* SFU_X509_VERBOSE_DEBUG_MODE */
      e_ret_status = SE_ERROR;
    }
  }

  /*
   * verify header
   */
#if defined(SFU_VERBOSE_DEBUG_MODE)
  TRACE("\r\n= [SBOOT] Calling SB_VerifyFWHeader");
#endif /* SFU_VERBOSE_DEBUG_MODE */
  if (SFU_SCHEME_X509_CORE_VerifyFWHeader(pxFwRawHeader, p_CertChain_OEM, p_CertChain_RootCA) == SFU_SUCCESS)
  {
    e_ret_status = SE_SUCCESS; /* Verify OK */
  }
  else
  {
    e_ret_status = SE_ERROR; /* Verify Failed */
  }
  return e_ret_status;

}

/**
  * @brief Secure Engine Decrypt Init function.
  *        It is a wrapper of Decrypt_Init function included in the protected area.
  * @param peSE_Status Secure Engine Status.
  *        This parameter can be a value of @ref SE_Status_Structure_definition.
  * @param pxSE_Metadata Metadata that will be used to fill the Crypto Init structure.
  * @param SE_FwType: Type of Fw Image.
  *        This parameter can be SE_FW_IMAGE_COMPLETE or SE_FW_IMAGE_PARTIAL.
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_Decrypt_Init(SE_StatusTypeDef *peSE_Status, SE_FwRawHeaderTypeDef *pxSE_Metadata, uint32_t SE_FwType)
{
  /* Check the pointers allocation */
  if (pxSE_Metadata == NULL)
  {
    return SE_ERROR;
  }
  *peSE_Status = SE_OK;
  return SE_SUCCESS;
}

/**
  * @brief Secure Engine Decrypt Append function.
  *        It is a wrapper of Decrypt_Append function included in the protected area.
  * @param peSE_Status Secure Engine Status.
  *        This parameter can be a value of @ref SE_Status_Structure_definition.
  * @param pInputBuffer pointer to Input Buffer.
  * @param uInputSize Input Size (bytes).
  * @param pOutputBuffe pointer to Output Buffer.
  * @param puOutputSize pointer to Output Size (bytes).
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_Decrypt_Append(SE_StatusTypeDef *peSE_Status, const uint8_t *pInputBuffer, int32_t InputSize,
                                 uint8_t *pOutputBuffer, int32_t *pOutputSize)
{
  /*
   * The firmware is not encrypted.
   * The only thing we need to do is to recopy the input buffer in the output buffer
   */
  (void)memcpy(pOutputBuffer, pInputBuffer, (uint32_t)InputSize);
  *pOutputSize = InputSize;
  /* Return status*/
  *peSE_Status = SE_OK;
  return SE_SUCCESS;
}

/**
  * @brief Secure Engine Decrypt Finish function.
  *        It is a wrapper of Decrypt_Finish function included in the protected area.
  * @param peSE_Status Secure Engine Status.
  *        This parameter can be a value of @ref SE_Status_Structure_definition.
  * @param pOutputBuffer pointer to Output Buffer.
  * @param puOutputSize pointer to Output Size (bytes).
  * @retval SE_ErrorStatus SE_SUCCESS if successful, SE_ERROR otherwise.
  */
SE_ErrorStatus SE_Decrypt_Finish(SE_StatusTypeDef *peSE_Status, uint8_t *pOutputBuffer, int32_t *pOutputSize)
{
  /* Check the pointers allocation */
  if ((pOutputBuffer == NULL) || (pOutputSize == NULL))
  {
    return SE_ERROR;
  }
  /* Return status*/
  *peSE_Status = SE_OK;
  return SE_SUCCESS;
}
#endif /* SECBOOT_CRYPTO_SCHEME */
