/**
  ******************************************************************************
  * @file    sfu_scheme_x509_mbedtls_ecdsa_verify.c
  * @author  MCD Application Team
  * @brief   This file provides an alternative implementation of
  * mbedtls_ecdsa_verify(), handing off the ecdsa verification to the
  * secure engine via PKCS11
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

/* Only include if using SECBOOT_X509_ECDSA_WITHOUT_ENCRYPT_SHA256 scheme */
#include "se_crypto_config.h"
#if (SECBOOT_CRYPTO_SCHEME == SECBOOT_X509_ECDSA_WITHOUT_ENCRYPT_SHA256)


#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif /* MBEDTLS_CONFIG_FILE */


#if defined(MBEDTLS_ECDSA_VERIFY_ALT)

/* Includes ------------------------------------------------------------------*/
#include "mbedtls/ecdsa.h"
#include "mbedtls/asn1write.h"
#include "mbedtls/pk.h"
#include "mbedtls/base64.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "tkms.h"
#include "se_interface_kms.h"
#include "sfu_trace.h"
#include "kms_platf_objects_interface.h"
#if defined(MBEDTLS_ECDSA_DETERMINISTIC)
#include "mbedtls/hmac_drbg.h"
#endif /* MBEDTLS_ECDSA_DETERMINISTIC */

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#define verificationPUBLIC_KEY_TEMPLATE_COUNT 3

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int32_t pkcs11_ecdsa_verify(uint8_t *p_ucPubKey, uint32_t uiPubKeySize, const uint8_t *p_ucHash, \
                                   uint32_t HashSize, uint8_t *p_ucSignature, uint32_t SignatureSize);
static int32_t MbedtlsEcpPointToDerX962(const mbedtls_ecp_point *p_Q, uint8_t *p_Der, uint32_t *p_SizeDer);
static int32_t MbedtlsSigRSToPkcs11Sig(const mbedtls_mpi *p_r, const mbedtls_mpi *p_s, uint8_t *p_Sig,
                                       uint32_t *p_SizeSig);

/* Functions Definition ------------------------------------------------------*/

/**
  * @brief  Perform ECDSA verify using PKCS11
  * @param  p_ucPubKey      pointer to public key for ECDSA verification
  * @param  uiPubKeySize   size of public key in bytes
  * @param  p_ucHash        pointer to hash
  * @param  HashSize       size of hash in bytes
  * @param  p_ucSignature   pointer to signature
  * @param  SignatureSize  size of signature in bytes
  * @retval int            0 for ecdsa verification success, non zero for
  *                        signature verification fail or error
  */

static int32_t pkcs11_ecdsa_verify(uint8_t *p_ucPubKey, uint32_t uiPubKeySize, const uint8_t *p_ucHash, \
                                   uint32_t HashSize, uint8_t *p_ucSignature, uint32_t SignatureSize)
{
  int32_t verifyResult = -1;

  CK_RV result = CKR_FUNCTION_FAILED;
  CK_FLAGS sessionFlags = CKF_SERIAL_SESSION;  /* Read ONLY session */
  CK_SESSION_HANDLE hSession = (CK_SESSION_HANDLE) NULL;
  CK_MECHANISM sigMechanism;

  /* sig verification public key */
  CK_OBJECT_HANDLE hObject;

  CK_ATTRIBUTE Template[2];

  /* Fill in the template with attribute to retrieve */
  Template[0].type       = CKA_LABEL;
  Template[0].pValue     = P11_OBJ_LABEL_VER_PUB_KEY_256;
  Template[0].ulValueLen = sizeof(P11_OBJ_LABEL_VER_PUB_KEY_256);
  Template[1].type       = CKA_EC_POINT;
  Template[1].pValue     = p_ucPubKey;
  Template[1].ulValueLen = uiPubKeySize;

  result = C_Initialize(NULL);
  if (CKR_OK != result)
  {
    verifyResult = -1;
    goto cleanup;
  }

  result = C_OpenSession(0,  sessionFlags, NULL, 0, &hSession);
  if (CKR_OK != result)
  {
    verifyResult = -1;
    goto cleanup;
  }

  result = C_CreateObject(hSession, Template, (sizeof(Template) / sizeof(Template[0])), &hObject);
  if (CKR_OK != result)
  {
    verifyResult = -1;
    goto cleanup;
  }

  sigMechanism.pParameter = NULL;
  sigMechanism.ulParameterLen = 0;
  sigMechanism.mechanism = CKM_ECDSA;

  result = C_VerifyInit(hSession, &sigMechanism, hObject);
  if (CKR_OK != result)
  {
    verifyResult = -1;
    goto cleanup;
  }

  result = C_Verify(hSession, (CK_BYTE_PTR) p_ucHash, (CK_ULONG)HashSize, (CK_BYTE_PTR)p_ucSignature, \
                    (CK_ULONG)SignatureSize);
  verifyResult = result;


  if (CKR_OK != result)
  {
    /* signature verification failed */
#if defined(SFU_X509_VERBOSE_DEBUG_MODE)
    TRACE("\n\r= [SBOOT] Signature verification FAILED!");
#endif /* SFU_X509_VERBOSE_DEBUG_MODE */
  }
  else
  {
    /* signature verification passed */
#if defined(SFU_VERBOSE_DEBUG_MODE)
    TRACE("\n\r= [SBOOT] Signature verification PASSED!");
#endif /* SFU_VERBOSE_DEBUG_MODE */
  }


cleanup:

  if ((CK_SESSION_HANDLE) NULL != hSession)
  {
    result = C_CloseSession((CK_SESSION_HANDLE) hSession);
  }

  result = C_Finalize(NULL);

  if (CKR_OK != result)
  {
    return result;
  }
  else
  {
    return verifyResult;
  }
}


/**
  * @brief  Convert mbedtls ecp point to der x962 format
  * @param  p_Q              pointer to ecp point
  * @param  p_Der           pointer to buffer for DER encoded point
  * @param  p_SizeDer       pointer to size of der
  * @note                  if function is called with a NULL p_Der, the function
  *                        returns the required der buffer size and 0 for return
  *                        value, otherwise returns size of der written
  * @retval int            0 for success but no DER data output, <0 for error
  *                        or size of der data written if successful
  */
static int32_t MbedtlsEcpPointToDerX962(const mbedtls_ecp_point *p_Q, uint8_t *p_Der, uint32_t *p_SizeDer)
{
  const mbedtls_mpi *pX = &(p_Q->X);
  const mbedtls_mpi *pY = &(p_Q->Y);
  size_t sPubKeyX = 0;
  size_t sPubKeyY = 0;
  uint8_t *pPubKeyX = NULL;
  uint8_t *pPubKeyY = NULL;
  uint8_t SizeLengthField = 0;
  int32_t result = 0;
  int32_t ReturnValue = 0;

  /* get size */
  sPubKeyX = mbedtls_mpi_size(pX);
  sPubKeyY = mbedtls_mpi_size(pY);

  /* Der Size */
  uint32_t RequiredDerSize = sPubKeyX + sPubKeyY + 1; /* 0x04|X|Y */

  if (RequiredDerSize > 0xFFFFFF)
  {
    SizeLengthField = 5; /* 0x84 | Len[3] | Len[2] | Len[1] | Len[0] */
  }
  else if (RequiredDerSize > 0xFFFF)
  {
    SizeLengthField = 4; /* 0x83 | Len[2] | Len[1] | Len[0] */
  }
  else if (RequiredDerSize > 0xFF)
  {
    SizeLengthField = 3; /* 0x82 | Len[1] | Len[0] */
  }
  else if (RequiredDerSize > 0x80)
  {
    SizeLengthField = 2; /* 0x81 | Len[0] */
  }
  else
  {
    SizeLengthField = 1; /* Len[0] */
  }

  RequiredDerSize += SizeLengthField + 1; /* 0x04 | Len[SizeLengthField] | 0x04 | X | Y */

  if ((NULL == p_Der) || (*p_SizeDer < RequiredDerSize))
  {
    *p_SizeDer = RequiredDerSize;
    ReturnValue = MBEDTLS_ERR_ECP_BUFFER_TOO_SMALL;
    goto CleanUp;
  }

  /*  */
  pPubKeyX = calloc(sPubKeyX, sizeof(uint8_t));
  pPubKeyY = calloc(sPubKeyY, sizeof(uint8_t));
  if ((NULL == pPubKeyX) || (NULL == pPubKeyY))
  {
    *p_SizeDer = 0;
    ReturnValue = MBEDTLS_ERR_ECP_ALLOC_FAILED;
    goto CleanUp;
  }

  /* Get Public Key X and Y */
  result = mbedtls_mpi_write_binary(pX, pPubKeyX, sPubKeyX);
  result |= mbedtls_mpi_write_binary(pY, pPubKeyY, sPubKeyY);
  if (0 != result)
  {
    *p_SizeDer = 0;
    ReturnValue =  MBEDTLS_ERR_ECP_BUFFER_TOO_SMALL;
    goto CleanUp;
  }

  /* Output Der */
  p_Der[0] = 0x04;
  if (SizeLengthField > 1)
  {
    uint32_t DataLength = RequiredDerSize - SizeLengthField;
    p_Der[1] = 0x80 | (0x7F & (SizeLengthField - 1));
    for (int32_t i = 0; i < (SizeLengthField - 1); i++)
    {
      p_Der[2 + i] = ((uint8_t *) &DataLength)[SizeLengthField - 1 - i];
    }
  }
  else
  {
    p_Der[1] = RequiredDerSize - 2;
  }
  p_Der[1 + SizeLengthField] = 0x04;
  memcpy(&p_Der[1 + SizeLengthField + 1], pPubKeyX, sPubKeyX);
  memcpy(&p_Der[1 + SizeLengthField + 1 + sPubKeyX], pPubKeyY, sPubKeyY);
  *p_SizeDer = RequiredDerSize;

#if defined(SFU_VERBOSE_DEBUG_MODE)
  TRACE("\n\rPublic Key:\n\r");
  int32_t iLoop = 0;
  for (int32_t i = 0; i < RequiredDerSize; i++)
  {
    TRACE("%02x", p_Der[i]);
    iLoop++;
    if (!((iLoop) % 32))
    {
      TRACE("\n\r");
      iLoop = 0;
    }
    if (i == 2)
    {
      TRACE("\n\r");
      iLoop = 0;
    }
  }
#endif /* SFU_VERBOSE_DEBUG_MODE */

CleanUp:
  if (NULL != pPubKeyX)
  {
    free(pPubKeyX);
  }
  if (NULL != pPubKeyY)
  {
    free(pPubKeyY);
  }
  return ReturnValue;
}

/**
  * @brief  Convert mbedtls signature (r,s) point to pkcs11 format
  * @param  p_r              pointer to r coordinate
  * @param  p_s              pointer to s coordinate
  * @param  p_Sig           pointer to buffer for signature
  * @param  p_SizeSig       pointer to size of signature buffer
  * @note                  if function is called with a NULL p_Sig, the function
  *                        returns the required buffer size and 0 for return
  *                        value, otherwise it returns size of data written
  * @retval int            0 for success but no DER data output, <0 for error
  *                        or size of der data written if successful
  */
static int32_t MbedtlsSigRSToPkcs11Sig(const mbedtls_mpi *p_r, const mbedtls_mpi *p_s, uint8_t *p_Sig,
                                       uint32_t *p_SizeSig)
{
  uint32_t uSizeR = mbedtls_mpi_size(p_r);
  uint32_t uSizeS = mbedtls_mpi_size(p_s);
  int32_t iResult = 0;

  /* Check buffer size is OK */
  if ((uSizeR + uSizeS) > *p_SizeSig)
  {
    *p_SizeSig = uSizeR + uSizeS; /* Return required size */
    return MBEDTLS_ERR_ECP_BUFFER_TOO_SMALL;
  }

  iResult = mbedtls_mpi_write_binary(p_r, &p_Sig[0], uSizeR); /* extract raw values of r */
  if (0 == iResult)
  {
    iResult = mbedtls_mpi_write_binary(p_s, &p_Sig[uSizeR], uSizeS); /* extract raw values of s */
  }

  if (0 != iResult)
  {
    *p_SizeSig = 0;
    return iResult; /* Error */
  }
  else
  {
    *p_SizeSig = (uSizeR + uSizeS);
    return iResult;
  }
}


/**
  * @brief           This function verifies the ECDSA signature of a
  *                  previously-hashed message.
  *
  * @note            If the bitlength of the message hash is larger than the
  *                  bitlength of the group order, then the hash is truncated as
  *                  defined in <em>Standards for Efficient Cryptography Group
  *                  (SECG): SEC1 Elliptic Curve Cryptography</em>, section
  *                  4.1.4, step 3.
  *
  * @param p_grp       The ECP group.
  * @param p_buf       The message hash.
  * @param blen      The length of p_buf.
  * @param p_Q         The public key to use for verification.
  * @param p_r         The first integer of the signature.
  * @param p_s         The second integer of the signature.
  *
  * @retval          0 on success.
  * @retval          #MBEDTLS_ERR_ECP_BAD_INPUT_DATA if the signature
  *                  is invalid.
  * @retval          An MBEDTLS_ERR_ECP_XXX or MBEDTLS_MPI_XXX
  *                  error code on failure for any other reason.
  */
int mbedtls_ecdsa_verify(mbedtls_ecp_group *p_grp,
                         const unsigned char  *p_buf,
                         size_t blen,
                         const mbedtls_ecp_point *p_Q,
                         const mbedtls_mpi *p_r,
                         const mbedtls_mpi *p_s)
{
  CK_BYTE ucSignature[48 * 2] = {0};
  size_t sSigLength = 0;
  int32_t returnValue = MBEDTLS_ERR_ECP_VERIFY_FAILED;
  uint8_t PubKeyDer[128] = {0};
  size_t sPubKeyDer = 0;

  /*---------------------------------------------------------------------------
   * convert signature from mbedtls_mpi r and s to raw for PKCS11
   */
  sSigLength = sizeof(ucSignature);
  returnValue = MbedtlsSigRSToPkcs11Sig(p_r, p_s, ucSignature, (uint32_t *) &sSigLength);

  /*---------------------------------------------------------------------------
   * convert public key from mbedtls to der
   */
  if (0 == returnValue)
  {
    sPubKeyDer = sizeof(PubKeyDer);
    returnValue = MbedtlsEcpPointToDerX962(p_Q, (uint8_t *) PubKeyDer, (uint32_t *) &sPubKeyDer);
  }

  /* --------------------------------------------------------------------------
   * Now we can verify using PKCS11
   */
  if (0 == returnValue)
  {
    returnValue = pkcs11_ecdsa_verify(PubKeyDer, sPubKeyDer, p_buf, blen, ucSignature, sSigLength);
  }
  return returnValue;
}
#else

#warning WARNING:
#warning IF YOU HAVE INCLUDED THIS FILE IN YOUR PROJECT AND ARE EXPECTING TO USE THE ALT VERSION OF
#warning mbedtls_ecdsa_verify
#warning YOU MAY NOT HAVE DEFINED MBEDTLS_ECDSA_VERIFY_ALT IN YOUR MBEDTLS CONFIG FILE (or it is not
#warning being seen by your IDE)
#warning YOU ARE STILL USING THE MBEDTLS VERSION OF mbedtls_ecdsa_verify, NOT THE ONE IN THIS FILE.
#warning IF THAT IS WHAT YOU EXPECT YOU CAN IGNORE THIS WARNING

#endif /* MBEDTLS_ECDSA_VERIFY_ALT */
#endif /* SCHEME_X509_ECDSA_WITHOUT_ENCRYPT_SHA256 */
