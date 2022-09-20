/**
  ******************************************************************************
  * @file    kms_ext_token_stsafe.c
  * @author  SMD/AME application teams
  * @brief   Key Management Services module, external token plug-in.
  *          This file includes an implementation of PKCS11 APIs for STSAFE-A1xx.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "string.h"
#include <stdlib.h>
#include "kms.h"

#include "stm32l4xx.h"

/*STSAFE Drivers*/
#include "stsafea_conf.h"
#include "stsafea_types.h"
#include "stsafea_core.h"
#include "stsafea_interface_conf.h"

/* KSM Includes*/
#include "kms_ext_token.h"
#include "kms_platf_objects_interface.h"
#include "stsafe_platf_objects.h"
#include "stsafe_platf_objects_config.h"

/* Private typedef -----------------------------------------------------------*/

typedef struct
{
  CK_SESSION_HANDLE   SessionHandle;                                   /*!< Session Handle */
  CK_SESSION_INFO     SessionInfo;                                     /*!< Session Info */
  CK_BBOOL            OperationActive;                                 /*!< Operation Status */
  CK_OBJECT_HANDLE    OperationObjects[STSAFE_P11_NUMBER_OBJECTS];     /*!< List of operation objects */
  CK_ULONG            ulOperationObjectsCount;                         /*!< N of valid operation object handles*/
} STSAFE_P11_SessionTypeDef;

/* Private defines -----------------------------------------------------------*/
#define STSAFE_P11_NB_SESSIONS_MAX          3UL           /*!< STSAFE P11 Max number of sessions*/
#define STSAFE_P11_SESSION_INDEX_OFFSET     1UL           /*!< STSAFE P11 session index Offset*/
#define STSAFE_RANDOM_MAX_LEN               0xFFU         /*!< STSAFE Max random length */

#define ASN_OCTET_STRING                    0x04U         /*!< ASN OCTET STRING  tag */
#define ASN_SEQUENCE                        0x30U         /*!< ASN SEQUENCE  tag */
#define ASN_LONG_LEN                        0x80U         /*!< ASN LONG length tag */
#define DER_UNCOMPRESSED                    0x04U         /*!< DER Uncompressed tag */
#define MAX_SIZE_BUFFER_I2_FOR_DATAS        0x1F0U        /*!< Max size of buffer STSAFE-A110 */
#define ZERO_OFFSET_ZONE                    0x00U         /*!< Offset 0 in zone STSAFE*/

/* Private macros ------------------------------------------------------------*/
#define __STSAFEA_GET_SLOT_ID_FROM_CK(__CKE_PRVKEY_SLOTx_PTR__) \
  (((*((CK_EXT_TOKEN_PRVKEY_SLOT *)__CKE_PRVKEY_SLOTx_PTR__)) == CKE_PRVKEY_SLOT0) ? STSAFEA_KEY_SLOT_0 :\
   ((*((CK_EXT_TOKEN_PRVKEY_SLOT *)__CKE_PRVKEY_SLOTx_PTR__)) == CKE_PRVKEY_SLOT1) ? STSAFEA_KEY_SLOT_1 : 0xFFU)

#define STS_CHK(ret, f) if (!ret) ret = f;                /*!< It allow to check status of function */

/* Private variables ---------------------------------------------------------*/
static StSafeA_Handle_t   hStSafe;    /*<! STSAFE Handle */
static STSAFE_P11_SessionTypeDef    a_SessionList[STSAFE_P11_NB_SESSIONS_MAX];  /*!< STSAFE P11 Session List */
static CK_ULONG ulSessionNb;    /*!< STSAFE P11 Number of open session. It can't exceed STSAFE_P11_NB_SESSIONS_MAX */
static CK_BBOOL  uInitialized = CK_FALSE;   /*!< STSAFE P11 Initialization Status. It is set to CK_TRUE calling
C_Initialize and to CK_FALSE calling C_Finalize */

/* Sensitive Attribute List - used to filter sensitive attributes when calling C_GetAttributeValue*/
/*CKO_PRIVATE_KEY objects - extractable / not extractable objects*/
static CK_ATTRIBUTE_TYPE a_SensAttList_CkoPrivateKey_NotExtr[] = {CKA_VALUE,
                                                                  CKA_EXT_TOKEN_PRVKEY_SLOT,
                                                                  CKA_VENDOR_SESSION_HANDLE
                                                                 };
static CK_ULONG SensAttList_CkoPrivateKey_NotExtrLen =  sizeof(a_SensAttList_CkoPrivateKey_NotExtr) /
                                                        sizeof(CK_ATTRIBUTE_TYPE);
static CK_ATTRIBUTE_TYPE a_SensAttList_CkoPrivateKey_Extr[] = { CKA_EXT_TOKEN_PRVKEY_SLOT,
                                                                CKA_VENDOR_SESSION_HANDLE
                                                              };
static CK_ULONG SensAttList_CkoPrivateKey_ExtrLen =  sizeof(a_SensAttList_CkoPrivateKey_Extr) /
                                                     sizeof(CK_ATTRIBUTE_TYPE);

/*CKO_SECRETE_KEY objects - extractable / not extractable objects*/
static CK_ATTRIBUTE_TYPE a_SensAttList_CkoSecretKey_NotExtr[] = {CKA_VALUE,
                                                                 CKA_VENDOR_SESSION_HANDLE
                                                                };
static CK_ULONG SensAttList_CkoSecretKey_NotExtr =  sizeof(a_SensAttList_CkoSecretKey_NotExtr) /
                                                    sizeof(CK_ATTRIBUTE_TYPE);
static CK_ATTRIBUTE_TYPE a_SensAttList_CkoSecretKey_Extr[] = {CKA_VENDOR_SESSION_HANDLE};
static CK_ULONG SensAttList_CkoSecretKey_ExtrLen =  sizeof(a_SensAttList_CkoSecretKey_Extr) /
                                                    sizeof(CK_ATTRIBUTE_TYPE);

/*CKO_CERTIFICATE objects - all objects are extractable*/
static CK_ATTRIBUTE_TYPE a_SensitiveAttributesList_CkoCertificate[] = {CKA_EXT_TOKEN_ZONE, CKA_EXT_TOKEN_ZONE_AC,
                                                                       CKA_VENDOR_SESSION_HANDLE
                                                                      };
static CK_ULONG SensitiveAttributesList_CkoCertificateLen = sizeof(a_SensitiveAttributesList_CkoCertificate) /
                                                            sizeof(CK_ATTRIBUTE_TYPE);

/*CKO_PUBLIC_KEY objects - all objects are extractable*/
static CK_ATTRIBUTE_TYPE a_SensitiveAttributesList_CkoPublicKey[] = {CKA_VENDOR_SESSION_HANDLE};
static CK_ATTRIBUTE_TYPE SensitiveAttributesList_CkoPublicKeyLen = sizeof(a_SensitiveAttributesList_CkoPublicKey) /
                                                                   sizeof(CK_ATTRIBUTE_TYPE);

/* Global variables ----------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/** @addtogroup Utils_Private_Functions
  * @{
  */
/* Session */
static CK_RV    SessionInit(void);
static CK_RV    SessionOpen(CK_SLOT_ID slotID, CK_FLAGS flags, CK_VOID_PTR pApplication,  CK_NOTIFY Notif,
                            CK_SESSION_HANDLE_PTR phSession);
static CK_RV    SessionClose(CK_SESSION_HANDLE hSession);
static CK_RV    SessionIsValid(CK_SESSION_HANDLE hSession);
static CK_RV    SessionIsOpen(CK_SESSION_HANDLE hSession);
static CK_RV    SessionOperationIsActive(CK_SESSION_HANDLE hSession);
static CK_RV    SessionOperationSetObjects(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE_PTR pObject, CK_ULONG ulCount);
static CK_RV    SessionOperationGetObjects(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE_PTR pObject,
                                           CK_ULONG ulMaxObjectCount, CK_ULONG_PTR pulObjectCount);
static CK_RV    SessionOperationFinish(CK_ULONG ulSessionIndex);
/* Objects / Attributes */
static CK_RV    ObjectSetSession(CK_OBJECT_HANDLE hObject, CK_SESSION_HANDLE hSession);
static CK_RV    ObjectSetVendorObjectAvailability(CK_OBJECT_HANDLE hObject, CK_BBOOL bValue);

static CK_RV    FindObjectsFromTemplate(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE_PTR phObject, CK_ULONG ulMaxCount,
                                        CK_ULONG_PTR pulObjectCount,  CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount);
static CK_RV    GetAttributeValueFromTemplate(CK_BYTE_PTR pValue,  CK_ULONG_PTR pulValueLen, CK_ATTRIBUTE_TYPE Type,
                                              CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount);
static CK_BBOOL IsMechanismTypeAllowed(CK_MECHANISM_TYPE MechanismIn, CK_MECHANISM_PTR  pMechanism,
                                       CK_ULONG ulValueLen);
static CK_BBOOL IsAttributeSensitive(CK_ATTRIBUTE_TYPE Type, CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount);
static CK_BBOOL IsAttributeTypeInList(CK_ATTRIBUTE_TYPE Type, CK_ATTRIBUTE_TYPE *pAttributeList, CK_ULONG ulCount);
/*StSafe Read*/
static CK_RV StSafeReadZone(uint8_t uZone, uint32_t ulOffset, uint8_t *pucData, uint32_t ulDataSize,
                            uint8_t AccessCondition);
static CK_RV StSafeReadCertificate(uint8_t *pValue, uint32_t *pulValueLen, uint8_t Zone, uint8_t AccessCondition);
/*StSafe Write*/
static CK_RV StSafeUpdateZone(uint8_t zone, StSafeA_LVBuffer_t in_Data, uint16_t offset);
/* Parsing */
static CK_RV DecodeDerASN1OctetString(CK_BYTE_PTR pValueOut, CK_ULONG_PTR pulValueOutLen, CK_BYTE_PTR pValueIn,
                                      CK_ULONG ulValueInLen);
static CK_RV DecodeDerASN1UncompressedEcPoint(CK_BYTE_PTR pValueOut, CK_ULONG_PTR pulValueOutLen, CK_BYTE_PTR pValueIn,
                                              CK_ULONG ulValueInLen);
static CK_RV EncodeRawToDerASN1UncompressedEcPoint(CK_BYTE_PTR pValueOut, CK_ULONG_PTR pulValueOutLen,
                                                   CK_BYTE_PTR pValueIn, CK_ULONG ulValueInLen);
static CK_RV GetLenDerASN1SequenceHeader(CK_ULONG_PTR pulValueOutLen, CK_BYTE_PTR pValueIn, CK_ULONG ulValueInLen);
/**
  * @}
  */

/** @addtogroup P11_Private_Functions
  * @{
  */
static CK_RV STSAFE_P11_Initialize(CK_VOID_PTR pInitArgs);
static CK_RV STSAFE_P11_Finalize(CK_VOID_PTR pReserved);
static CK_RV STSAFE_P11_GetInfo(CK_INFO_PTR pInfo);
/* Slot and token management */
static CK_RV STSAFE_P11_GetSlotList(CK_BBOOL tokenPresent, CK_SLOT_ID_PTR pSlotList, CK_ULONG_PTR pulCount);
static CK_RV STSAFE_P11_GetSlotInfo(CK_SLOT_ID slotID, CK_SLOT_INFO_PTR pInfo);
static CK_RV STSAFE_P11_GetTokenInfo(CK_SLOT_ID slotID, CK_TOKEN_INFO_PTR pInfo);
static CK_RV STSAFE_P11_GetMechanismList(CK_SLOT_ID slotID, CK_MECHANISM_TYPE_PTR pMechanismList,
                                         CK_ULONG_PTR pulCount);
static CK_RV STSAFE_P11_GetMechanismInfo(CK_SLOT_ID slotID, CK_MECHANISM_TYPE type, CK_MECHANISM_INFO_PTR pInfo);
static CK_RV STSAFE_P11_InitToken(CK_SLOT_ID slotID, CK_UTF8CHAR_PTR pPin, CK_ULONG ulPinLen, CK_UTF8CHAR_PTR pLabel);
static CK_RV STSAFE_P11_InitPIN(CK_SESSION_HANDLE hSession, CK_UTF8CHAR_PTR pPin, CK_ULONG ulPinLen);
static CK_RV STSAFE_P11_SetPIN(CK_SESSION_HANDLE hSession, CK_UTF8CHAR_PTR pOldPin, CK_ULONG ulOldLen,
                               CK_UTF8CHAR_PTR pNewPin, CK_ULONG ulNewLen);
/* Session management */
static CK_RV STSAFE_P11_OpenSession(CK_SLOT_ID slotID, CK_FLAGS flags, CK_VOID_PTR pApplication, CK_NOTIFY Notify,
                                    CK_SESSION_HANDLE_PTR phSession);
static CK_RV STSAFE_P11_CloseSession(CK_SESSION_HANDLE hSession);
static CK_RV STSAFE_P11_CloseAllSessions(CK_SLOT_ID slotID);
static CK_RV STSAFE_P11_GetSessionInfo(CK_SESSION_HANDLE hSession, CK_SESSION_INFO_PTR pInfo);
static CK_RV STSAFE_P11_GetOperationState(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pOperationState,
                                          CK_ULONG_PTR pulOperationStateLen);
static CK_RV STSAFE_P11_SetOperationState(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pOperationState,
                                          CK_ULONG ulOperationStateLen, CK_OBJECT_HANDLE hEncryptionKey,
                                          CK_OBJECT_HANDLE hAuthenticationKey);
static CK_RV STSAFE_P11_Login(CK_SESSION_HANDLE hSession, CK_USER_TYPE userType, CK_UTF8CHAR_PTR pPin,
                              CK_ULONG ulPinLen);
static CK_RV STSAFE_P11_Logout(CK_SESSION_HANDLE hSession);
/* Object management */
static CK_RV STSAFE_P11_CreateObject(CK_SESSION_HANDLE hSession, CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount,
                                     CK_OBJECT_HANDLE_PTR phObject);
static CK_RV STSAFE_P11_CopyObject(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hObject, CK_ATTRIBUTE_PTR pTemplate,
                                   CK_ULONG ulCount, CK_OBJECT_HANDLE_PTR phNewObject);
static CK_RV STSAFE_P11_DestroyObject(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hObject);
static CK_RV STSAFE_P11_GetObjectSize(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hObject, CK_ULONG_PTR pulSize);
static CK_RV STSAFE_P11_GetAttributeValue(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hObject,
                                          CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount);
static CK_RV STSAFE_P11_SetAttributeValue(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hObject,
                                          CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount);
static CK_RV STSAFE_P11_FindObjectsInit(CK_SESSION_HANDLE hSession, CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount);
static CK_RV STSAFE_P11_FindObjects(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE_PTR phObject,
                                    CK_ULONG ulMaxObjectCount, CK_ULONG_PTR pulObjectCount);
static CK_RV STSAFE_P11_FindObjectsFinal(CK_SESSION_HANDLE hSession);
/* Encryption and decryption */
static CK_RV STSAFE_P11_EncryptInit(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hKey);
static CK_RV STSAFE_P11_Encrypt(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pData, CK_ULONG ulDataLen,
                                CK_BYTE_PTR pEncryptedData, CK_ULONG_PTR pulEncryptedDataLen);
static CK_RV STSAFE_P11_EncryptUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pPart, CK_ULONG ulPartLen,
                                      CK_BYTE_PTR pEncryptedPart, CK_ULONG_PTR pulEncryptedPartLen);
static CK_RV STSAFE_P11_EncryptFinal(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pLastEncryptedPart,
                                     CK_ULONG_PTR pulLastEncryptedPartLen);
static CK_RV STSAFE_P11_DecryptInit(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hKey);
static CK_RV STSAFE_P11_Decrypt(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pEncryptedData, CK_ULONG ulEncryptedDataLen,
                                CK_BYTE_PTR pData, CK_ULONG_PTR pulDataLen);
static CK_RV STSAFE_P11_DecryptUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pEncryptedPart,
                                      CK_ULONG ulEncryptedPartLen, CK_BYTE_PTR pPart, CK_ULONG_PTR pulPartLen);
static CK_RV STSAFE_P11_DecryptFinal(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pLastPart, CK_ULONG_PTR pulLastPartLen);
/* Message digesting */
static CK_RV STSAFE_P11_DigestInit(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism);
static CK_RV STSAFE_P11_Digest(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pData, CK_ULONG ulDataLen, CK_BYTE_PTR pDigest,
                               CK_ULONG_PTR pulDigestLen);
static CK_RV STSAFE_P11_DigestUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pPart, CK_ULONG ulPartLen);
static CK_RV STSAFE_P11_DigestKey(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hKey);
static CK_RV STSAFE_P11_DigestFinal(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pDigest, CK_ULONG_PTR pulDigestLen);
/* Signing and MACing */
static CK_RV STSAFE_P11_SignInit(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hKey);
static CK_RV STSAFE_P11_Sign(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pData, CK_ULONG ulDataLen, CK_BYTE_PTR pSignature,
                             CK_ULONG_PTR pulSignatureLen);
static CK_RV STSAFE_P11_SignUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pPart, CK_ULONG ulPartLen);
static CK_RV STSAFE_P11_SignFinal(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pSignature, CK_ULONG_PTR pulSignatureLen);
static CK_RV STSAFE_P11_SignRecoverInit(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hKey);
static CK_RV STSAFE_P11_SignRecover(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pData, CK_ULONG ulDataLen,
                                    CK_BYTE_PTR pSignature, CK_ULONG_PTR pulSignatureLen);
/* Verifying signatures and MACs */
static CK_RV STSAFE_P11_VerifyInit(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hKey);
static CK_RV STSAFE_P11_Verify(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pData, CK_ULONG ulDataLen,
                               CK_BYTE_PTR pSignature, CK_ULONG ulSignatureLen);
static CK_RV STSAFE_P11_VerifyUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pPart, CK_ULONG ulPartLen);
static CK_RV STSAFE_P11_VerifyFinal(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pSignature, CK_ULONG ulSignatureLen);
static CK_RV STSAFE_P11_VerifyRecoverInit(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism,
                                          CK_OBJECT_HANDLE hKey);
static CK_RV STSAFE_P11_VerifyRecover(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pSignature, CK_ULONG ulSignatureLen,
                                      CK_BYTE_PTR pData, CK_ULONG_PTR pulDataLen);
/* Dual-function cryptographic operations */
static CK_RV STSAFE_P11_DigestEncryptUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pPart, CK_ULONG ulPartLen,
                                            CK_BYTE_PTR pEncryptedPart, CK_ULONG_PTR pulEncryptedPartLen);
static CK_RV STSAFE_P11_DecryptDigestUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pEncryptedPart,
                                            CK_ULONG ulEncryptedPartLen, CK_BYTE_PTR pPart, CK_ULONG_PTR pulPartLen);
static CK_RV STSAFE_P11_SignEncryptUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pPart, CK_ULONG ulPartLen,
                                          CK_BYTE_PTR pEncryptedPart, CK_ULONG_PTR pulEncryptedPartLen);
static CK_RV STSAFE_P11_DecryptVerifyUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pEncryptedPart,
                                            CK_ULONG ulEncryptedPartLen, CK_BYTE_PTR pPart, CK_ULONG_PTR pulPartLen);
/* Key management */
static CK_RV STSAFE_P11_GenerateKey(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_ATTRIBUTE_PTR pTemplate,
                                    CK_ULONG ulCount, CK_OBJECT_HANDLE_PTR phKey);
static CK_RV STSAFE_P11_GenerateKeyPair(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism,
                                        CK_ATTRIBUTE_PTR pPublicKeyTemplate, CK_ULONG ulPublicKeyAttributeCount,
                                        CK_ATTRIBUTE_PTR pPrivateKeyTemplate,
                                        CK_ULONG ulPrivateKeyAttributeCount, CK_OBJECT_HANDLE_PTR phPublicKey,
                                        CK_OBJECT_HANDLE_PTR phPrivateKey);
static CK_RV STSAFE_P11_WrapKey(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hWrappingKey,
                                CK_OBJECT_HANDLE hKey, CK_BYTE_PTR pWrappedKey, CK_ULONG_PTR pulWrappedKeyLen);
static CK_RV STSAFE_P11_UnwrapKey(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism,
                                  CK_OBJECT_HANDLE hUnwrappingKey, CK_BYTE_PTR pWrappedKey, CK_ULONG ulWrappedKeyLen,
                                  CK_ATTRIBUTE_PTR pTemplate,
                                  CK_ULONG ulAttributeCount, CK_OBJECT_HANDLE_PTR phKey);
static CK_RV STSAFE_P11_DeriveKey(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hBaseKey,
                                  CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulAttributeCount, CK_OBJECT_HANDLE_PTR phKey);
/* Random number generation */
static CK_RV STSAFE_P11_SeedRandom(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pSeed, CK_ULONG ulSeedLen);
static CK_RV STSAFE_P11_GenerateRandom(CK_SESSION_HANDLE hSession, CK_BYTE_PTR RandomData, CK_ULONG ulRandomLen);
/* Parallel function management */
static CK_RV STSAFE_P11_GetFunctionStatus(CK_SESSION_HANDLE hSession);
static CK_RV STSAFE_P11_CancelFunction(CK_SESSION_HANDLE hSession);
static CK_RV STSAFE_P11_WaitForSlotEvent(CK_FLAGS flags, CK_SLOT_ID_PTR pSlot, CK_VOID_PTR pRserved);

/**
  * @}
  */

/* Functions Definition ------------------------------------------------------*/

/**
  * @brief  C_Initialize initializes the Cryptoki library.
  * @param  pInitArgs  if this is not NULL_PTR, it gets cast to CK_C_INITIALIZE_ARGS_PTR and dereferenced
  * @retval CK_RV CKR_ARGUMENTS_BAD, CKR_CRYPTOKI_ALREADY_INITIALIZED, CKR_FUNCTION_FAILED, CKR_OK.
  */
static CK_RV STSAFE_P11_Initialize(CK_VOID_PTR pvInitArgs)
{
  CK_C_INITIALIZE_ARGS_PTR p_init_args;

  /* STSAFE MW requires a data buffer to send/receive bytes over the bus.*/

  static uint8_t a_rx_tx_stsafea_data [STSAFEA_BUFFER_MAX_SIZE];
  uint8_t a_echo_data[3] = {0x01U, 0x02U, 0x03U};
  StSafeA_LVBuffer_t out_echo;
  
  /* Check Input parameters */
  if (pvInitArgs != NULL_PTR)
  {
    p_init_args = (CK_C_INITIALIZE_ARGS_PTR)(pvInitArgs);

    if (p_init_args->pReserved != NULL_PTR)
    {
      return CKR_ARGUMENTS_BAD;
    }
  }

  /* ========== Check PKCS11 status and session  ========== */

  /* Check if PKCS11 module has already been initialized */
  if (uInitialized == CK_TRUE)
  {
    return CKR_CRYPTOKI_ALREADY_INITIALIZED;
  }

  /* ========== Initialize CryptoKi ========== */
  uInitialized = CK_FALSE;

  if (STSAFE_OBJ_Init() == CKR_OK)
  {
    if (StSafeA_Init(&hStSafe, a_rx_tx_stsafea_data) == STSAFEA_OK)
    {
      /* Test STSAFE is configured properly: echo command */
      if (StSafeA_Echo(&hStSafe, a_echo_data, (uint16_t)(sizeof(a_echo_data)), &out_echo, STSAFEA_MAC_NONE)
          == STSAFEA_OK)
      {
        /* Init Session */
        if (SessionInit() == CKR_OK)
        {
          /* PKCS11 Initialized successfully */
          uInitialized = CK_TRUE;
        }
      }
    }
  }

  /*Check if initialization was successful*/
  if (uInitialized == CK_FALSE)
  {
    return CKR_FUNCTION_FAILED;
  }

  return CKR_OK;
}


/**
  * @brief  C_Finalize indicates that an application is done with the Cryptoki library
  * @param  pReserved Reserved.  Should be NULL_PTR.
  * @retval CK_RV CKR_ARGUMENTS_BAD, CKR_OK.
  */
static CK_RV STSAFE_P11_Finalize(CK_VOID_PTR pvReserved)
{
  /* Check Input parameters */
  if (pvReserved != NULL_PTR)
  {
    return CKR_ARGUMENTS_BAD;
  }

  /* ========== Check PKCS11 status and session  ========== */

  /* Check if PKCS11 module has already been initialized */
  if (uInitialized == CK_FALSE)
  {
    return CKR_CRYPTOKI_NOT_INITIALIZED;
  }

  /*Reset STSAFE handle*/
  (void)memset(((StSafeA_Handle_t *)(&hStSafe)), 0, sizeof(hStSafe));

  /* Reset PKCS11 Initialization status */
  uInitialized = CK_FALSE;

  return CKR_OK;
}

/**
  * @brief  C_GetInfo returns general information about Cryptoki.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  pInfo     location that receives information
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED
  */
static CK_RV STSAFE_P11_GetInfo(CK_INFO_PTR pInfo)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}

/**
  * @brief  C_GetSlotList obtains a list of slots in the system.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  tokenPresent     only slots with tokens
  * @param  pSlotList     receives array of slot IDs
  * @param  pulCount      receives number of slots
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED
  */
static CK_RV STSAFE_P11_GetSlotList(CK_BBOOL tokenPresent, CK_SLOT_ID_PTR pSlotList, CK_ULONG_PTR pulCount)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}

/**
  * @brief  C_GetSlotInfo obtains information about a particular slot in the system
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  slotID     the ID of the slot
  * @param  pInfo     receives the slot information
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED
  */
static CK_RV STSAFE_P11_GetSlotInfo(CK_SLOT_ID slotID, CK_SLOT_INFO_PTR pInfo)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}

/**
  * @brief  C_GetTokenInfo obtains information about a particular token in the system
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  slotID     ID of the token's slot
  * @param  pInfo     receives the token information
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED
  */
static CK_RV STSAFE_P11_GetTokenInfo(CK_SLOT_ID slotID, CK_TOKEN_INFO_PTR pInfo)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}

/**
  * @brief  C_GetMechanismList obtains a list of mechanism types supported by a token
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  slotID     ID of token's slot
  * @param  pMechanismList     gets mech. array
  * @param  pulCount     gets # of mechs.
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED
  */
static CK_RV STSAFE_P11_GetMechanismList(CK_SLOT_ID slotID, CK_MECHANISM_TYPE_PTR pMechanismList, CK_ULONG_PTR pulCount)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}

/**
  * @brief  C_GetMechanismInfo obtains information about a particular mechanism possibly supported by a token.
  * @param  slotID     ID of the token's slot
  * @param  type     type of mechanism
  * @param  pInfo     receives mechanism info
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED
  */
static CK_RV STSAFE_P11_GetMechanismInfo(CK_SLOT_ID slotID, CK_MECHANISM_TYPE type, CK_MECHANISM_INFO_PTR pInfo)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}

/**
  * @brief  C_InitToken initializes a token.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  slotID     ID of the token's slot
  * @param  pPin     the SO's initial PIN
  * @param  ulPinLen     length in bytes of the PIN
  * @param  pLabel     32-byte token label (blank padded)
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED
  */
static CK_RV STSAFE_P11_InitToken(CK_SLOT_ID slotID, CK_UTF8CHAR_PTR pPin, CK_ULONG ulPinLen, CK_UTF8CHAR_PTR pLabel)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}

/**
  * @brief  C_InitPIN initializes the normal user's PIN.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession     the session's handle
  * @param  pPin     the normal user's PIN
  * @param  ulPinLen     length in bytes of the PIN
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED
  */
static CK_RV STSAFE_P11_InitPIN(CK_SESSION_HANDLE hSession, CK_UTF8CHAR_PTR pPin, CK_ULONG ulPinLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}

/**
  * @brief  C_SetPIN modifies the PIN of the user who is logged in.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession     the session's handle
  * @param  pOldPin     the old PIN
  * @param  ulOldLen     length of the old PIN
  * @param  pNewPin     the new PIN
  * @param  ulNewLen     length of the new PIN
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED
  */
static CK_RV STSAFE_P11_SetPIN(CK_SESSION_HANDLE hSession, CK_UTF8CHAR_PTR pOldPin, CK_ULONG ulOldLen,
                               CK_UTF8CHAR_PTR pNewPin, CK_ULONG ulNewLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}


/**
  * @brief  C_OpenSession opens a session between an application and a token.
  * @param  slotID     the slot's ID
  * @param  flags     from CK_SESSION_INFO
  * @param  pApplication     passed to callback
  * @param  Notify     callback function
  * @param  phSession     gets session handle
  * @retval CK_RV CKR_ARGUMENTS_BAD, CKR_CRYPTOKI_NOT_INITIALIZED, CKR_SLOT_ID_INVALID,
          CKR_SESSION_PARALLEL_NOT_SUPPORTED, CKR_SESSION_COUNT, CKR_OK.
  */
static CK_RV STSAFE_P11_OpenSession(CK_SLOT_ID slotID, CK_FLAGS flags,
                                    CK_VOID_PTR pApplication,  CK_NOTIFY Notify,
                                    CK_SESSION_HANDLE_PTR phSession)
{

  /* Check input parameters */
  if (phSession == NULL_PTR)
  {
    return CKR_ARGUMENTS_BAD;
  }

  /* ========== Check PKCS11 status and session  ========== */

  /* Check if PKCS11 module has already been initialized */
  if (uInitialized == CK_FALSE)
  {
    return CKR_CRYPTOKI_NOT_INITIALIZED;
  }

  /* Check the value of slotID: the only slot supported by this implementation is slotID = 0*/
  if (slotID != 0x0000U)
  {
    return CKR_SLOT_ID_INVALID;
  }

  /* As defined in PKCS11 spec: For legacy reasons, the CKF_SERIAL_SESSION bit MUST
  always be set; if a call to C_OpenSession does not have this bit set, the call
  should return unsuccessfully with the error code CKR_SESSION_PARALLEL_NOT_SUPPORTED.
  */
  if ((flags & CKF_SERIAL_SESSION) == 0x0000U)
  {
    return CKR_SESSION_PARALLEL_NOT_SUPPORTED;
  }

  /* Open Session */
  if (SessionOpen(slotID, flags, pApplication, Notify, phSession) != CKR_OK)
  {
    return CKR_SESSION_COUNT;
  }

  return CKR_OK;

}

/**
  * @brief  C_CloseSession closes a session between an application and a token.
  * @param  hSession     the session's handle
  * @retval CK_RV CKR_CRYPTOKI_NOT_INITIALIZED,CKR_SESSION_HANDLE_INVALID,
  *         CKR_GENERAL_ERROR, CKR_OK
  */
static CK_RV STSAFE_P11_CloseSession(CK_SESSION_HANDLE hSession)
{
  CK_OBJECT_HANDLE hObject, hObjectMin, hObjectMax;
  CK_ATTRIBUTE_PTR  p_template_embedded, p_attribute_embedded;
  CK_ULONG  ul_count_embedded;

  /* ========== Check PKCS11 status and session  ========== */

  /* Check if PKCS11 module has already been initialized */
  if (uInitialized == CK_FALSE)
  {
    return CKR_CRYPTOKI_NOT_INITIALIZED;
  }

  /* Check if Session handle specified in input is valid */
  if (SessionIsValid(hSession) != CKR_OK)
  {
    return CKR_SESSION_HANDLE_INVALID;
  }

  /* Check if Session is open */
  if (SessionIsOpen(hSession) != CKR_OK)
  {
    return CKR_OK;
  }

  /* ========== Reset session handle ========== */

  /*Get Objects handles for all objects allocated (both token objects and session objects)*/
  if (STSAFE_OBJ_GetObjectRange(&hObjectMin, &hObjectMax) != CKR_OK)
  {
    return CKR_GENERAL_ERROR;
  }

  /*Init the loop*/
  hObject = hObjectMin;

  /*Loop on all allocated objects */
  while (hObject < hObjectMax)
  {
    /* Get a pointer to the template of the embedded object corresponding to hObject handle  */
    if (STSAFE_OBJ_GetTemplateFromObjectHandle(&p_template_embedded, &ul_count_embedded, hObject) != CKR_OK)
    {
      return CKR_GENERAL_ERROR;
    }

    /* ========== Check CKA_VENDOR_SESSION_HANDLE attribute ========== */
    /* If the object is a session object and is assigned to this session,
     *  mark it as available (CKA_VENDOR_SESSION_HANDLE = CK_INVALID_HANDLE)*/
    if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_embedded, CKA_VENDOR_SESSION_HANDLE, p_template_embedded,
                                           ul_count_embedded) == CKR_OK)
    {
      /*Check if the object is assigned to this session*/
      if ((*((CK_VENDOR_SESSION_HANDLE *)(p_attribute_embedded->pValue))) == hSession)
      {
        /* Mark the object as available (i.e. CKA_VENDOR_SESSION_HANDLE = CK_INVALID_HANDLE)*/
        if (ObjectSetSession(hObject, CK_INVALID_HANDLE) != CKR_OK)
        {
          return CKR_GENERAL_ERROR;
        }
      }
    }

    /* Move to the next object */
    hObject++;
  }

  /* Close the session */
  if (SessionClose(hSession) != CKR_OK)
  {
    return CKR_GENERAL_ERROR;
  }

  /* Everything is ok, return */
  return CKR_OK;
}

/**
  * @brief  C_CloseAllSessions closes all sessions with a token
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  slotID      the token's slot
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED
  */
static CK_RV STSAFE_P11_CloseAllSessions(CK_SLOT_ID slotID)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_GetSessionInfo obtains information about the session.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession     the session's handle
  * @param  pInfo     receives session info
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED
  */
static CK_RV STSAFE_P11_GetSessionInfo(CK_SESSION_HANDLE hSession, CK_SESSION_INFO_PTR pInfo)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_GetOperationState obtains the state of the cryptographic operation in a session
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession     the session's handle
  * @param  pOperationState     gets state
  * @param  pulOperationStateLen     gets state length
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED
  */
static CK_RV STSAFE_P11_GetOperationState(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pOperationState,
                                          CK_ULONG_PTR pulOperationStateLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_SetOperationState restores the state of the cryptographic operation in a session.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession     session's handle
  * @param  pOperationState     holds state
  * @param  ulOperationStateLen     holds state length
  * @param  hEncryptionKey     en/decryption key
  * @param  hAuthenticationKey     sign/verify key
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED
  */
static CK_RV STSAFE_P11_SetOperationState(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pOperationState,
                                          CK_ULONG ulOperationStateLen, CK_OBJECT_HANDLE hEncryptionKey,
                                          CK_OBJECT_HANDLE hAuthenticationKey)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_Login logs a user into a token.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession    the session's handle
  * @param  userType      the user type
  * @param  pPin      the user's PIN
  * @param  ulPinLen      the length of the PIN
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED
  */
static CK_RV STSAFE_P11_Login(CK_SESSION_HANDLE hSession, CK_USER_TYPE userType, CK_UTF8CHAR_PTR pPin,
                              CK_ULONG ulPinLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief   C_Logout logs a user out from a token.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession    the session's handle
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED
  */
static CK_RV STSAFE_P11_Logout(CK_SESSION_HANDLE hSession)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}

/**
  * @brief  C_CreateObject creates a new object.
  * @note   This implementation supports creation only of CKO_PUBLIC_KEY Objects for signature verification with
  *         C_Verify.
  *         Two types of objects can be created : NIST-P256 and NIST-P384
  *         (ref to P11_OBJ_ID_VER_256 and P11_OBJ_ID_VER_384).
  *         Each call to create object will overwrite the value of CKA_EC_POINT if all the templates parameters are
  *         consistent with the ones specified in the templates.
  *         Created objects are session objects and stored in non-volatile memory.
  *         The content of CKA_EC_POINT attribute is invalidated calling C_CloseSession()
  *         and not retained during power-off.
  * @param  hSession     the session's handle
  * @param  pTemplate     the object's template
  * @param  ulCount     attributes in template
  * @param  phObject     gets new object's handle.
  * @retval CK_RV CKR_ARGUMENTS_BAD, CKR_CRYPTOKI_NOT_INITIALIZED, CKR_SESSION_HANDLE_INVALID,
  *               CKR_SESSION_CLOSED,CKR_ATTRIBUTE_TYPE_INVALID, CKR_FUNCTION_FAILED, CKR_DEVICE_MEMORY, CKR_OK.
  */
static CK_RV STSAFE_P11_CreateObject(CK_SESSION_HANDLE hSession, CK_ATTRIBUTE_PTR pTemplate,
                                     CK_ULONG ulCount,  CK_OBJECT_HANDLE_PTR phObject)
{
  CK_RV e_ret_status;
  CK_OBJECT_HANDLE h_obj,h_obj_cert,h_cert_obj;
  CK_ATTRIBUTE_PTR  p_template_embedded,p_cert_template_embedded;
  CK_ULONG  ul_count_embedded,ul_cert_count_embedded;
  CK_ATTRIBUTE_PTR p_attribute_embedded,p_attribute_tmp;
  CK_ATTRIBUTE_PTR p_attribute_input;
  CK_ATTRIBUTE  p_template_filtered[STSAFE_P11_MAX_TEMPLATE_SIZE];
  CK_ULONG  ul_count_filtered;
  CK_ULONG ul_obj_cnt;
  CK_ATTRIBUTE  template_tmp,template_tmp_cert;
  CK_BBOOL object_found;
  CK_ATTRIBUTE_PTR p_attribute;
  CK_EXT_TOKEN_ZONE zone;


  /*Check input parameters*/
  if ((pTemplate == NULL_PTR) || (ulCount == 0UL) || (ulCount > STSAFE_P11_MAX_TEMPLATE_SIZE) || (phObject == NULL_PTR))
  {
    return CKR_ARGUMENTS_BAD;
  }

  /* ========== Check PKCS11 status and session  ========== */

  /* Check if PKCS11 module has already been initialized */
  if (uInitialized == CK_FALSE)
  {
    return CKR_CRYPTOKI_NOT_INITIALIZED;
  }

  /* Check if the Session handle specified in input is valid */
  if (SessionIsValid(hSession) != CKR_OK)
  {
    return CKR_SESSION_HANDLE_INVALID;
  }

  /* Check if Session is open */
  if (SessionIsOpen(hSession) != CKR_OK)
  {
    return CKR_SESSION_CLOSED;
  }
  /* Use case of Create object once object is a certificate, next release will implement update of the object as PKCS11 object */
  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute, CKO_CERTIFICATE , pTemplate, ulCount) == CKR_OK)
  {

    /* ==========  We should update certificate only if this is a dynamic object ========== */

    /* First we get CKA_LABEL this will allow later to identify required information about STSAFE Zones */

    if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute, CKA_LABEL, pTemplate, ulCount) == CKR_OK)
    {
      if ((!memcmp(p_attribute->pValue, "DeviceCertificate", p_attribute->ulValueLen)) == 0)      {

        (void) memcpy(&template_tmp.pValue, &p_attribute->pValue, p_attribute->ulValueLen);

        /* ==========  We manage a dynamic object, need to get certificate value and size  ========== */

        if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute, CKA_VALUE , pTemplate, ulCount) == CKR_OK)
        {
          /* ==========  We get information from object template like zone number ========== */

          template_tmp.type = CKA_LABEL;

           /*Look for the object corresponding to Label, 1 single object*/
          if (FindObjectsFromTemplate(CKV_SESSION_HANDLE_ANY, &h_cert_obj, 1, &ul_obj_cnt, &template_tmp, 1) != CKR_OK)
          {
            return CKR_FUNCTION_FAILED;
          }

          /* Get a pointer to the object template*/

          if (STSAFE_OBJ_GetTemplateFromObjectHandle(&p_cert_template_embedded, &ul_cert_count_embedded, h_cert_obj) != CKR_OK)
          {
            return CKR_FUNCTION_FAILED;
          }

          /* Extract from the template the CKA_EXT_TOKEN_ZONE */

          if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_EXT_TOKEN_ZONE, p_cert_template_embedded,
                                                 ul_cert_count_embedded) != CKR_OK)
          {
            return CKR_FUNCTION_FAILED;
          }

          zone = (*((CK_EXT_TOKEN_ZONE *)(p_attribute_tmp->pValue)));

          STSAFEA_DYNAMIC_ALLOCATE_LV_BUFFER_VAR(StSafeA_LVBuffer_t, DatatoStore, (p_attribute->ulValueLen) + 2);

          DatatoStore.Length = p_attribute->ulValueLen;

          (void) memcpy(DatatoStore.Data,  p_attribute->pValue, p_attribute->ulValueLen);

          e_ret_status=StSafeUpdateZone(zone, DatatoStore,ZERO_OFFSET_ZONE);

          STSAFEA_DYNAMIC_FREE_LV_BUFFER_VAR(DatatoStore);

          if (e_ret_status != CKR_OK)
            return CKR_FUNCTION_FAILED;
          else
          {
            /* ========== Update attribute : Cert should be enabled for all sessions it's a dynamic object ========== */

            template_tmp_cert.type = CKA_LABEL;
            template_tmp_cert.pValue = P11_OBJ_LABEL_DYN_CRT;

            if (FindObjectsFromTemplate(CKV_SESSION_HANDLE_ANY, &h_obj_cert, 1, &ul_obj_cnt, &template_tmp_cert, 1) != CKR_OK)
            {
              return CKR_FUNCTION_FAILED;
            }

            /* ========== Update attribute : Cert should be enabled for all sessions  ========== */

            ObjectSetVendorObjectAvailability(h_obj_cert, CK_TRUE);

            *phObject = h_obj_cert;

            return CKR_OK;
          }
        }
      }
      else
        /* ========== Certificate is not a dynamic object because this is device certificate ========== */
        return CKR_OK;
    }
    else
    /* ========== We didn't found label for the certificate object  ========== */
      return CKR_FUNCTION_FAILED;
  }

  /* Use case of Create object once object is NOT a certificate */

  /* ======================== Prepare input template to look for pre-allocated object =================== */

  /* Since this implementation doesn't support dynamic memory allocation, the template specified in input will
    * have to match pre-allocated objects The only value that can be different is CKA_EC_POINT, the actual value
    * of the public key, that is unique per key.
    * Filter attributes so that they will not be considered when looking and comparing the template with pre-allocated
    * objects.
    */
  CK_ATTRIBUTE attribute_filter[] =  { {CKA_EC_POINT, NULL_PTR, 0}  } ;

  if (STSAFE_OBJ_FilterTemplate(p_template_filtered, &ul_count_filtered, pTemplate, ulCount, attribute_filter,
                                sizeof(attribute_filter) / sizeof(attribute_filter[0])) != CKR_OK)
  {
    return CKR_FUNCTION_FAILED;
  }

  /* ======================== Check if the object can be created =================== */
  /*  Note: In this implementation we only support creation of 256-bit or 384-bit keys used for C_Verify.
   *  Create Object can be used only to create either one of these two objects. The one selected will be
   *  overwritten.  */

  /*Init*/
  object_found = CK_FALSE;

  /*Point to the object pre-allocated for NISTP-256 bit verification key*/
  template_tmp.type = CKA_LABEL;
  template_tmp.pValue = P11_OBJ_LABEL_VER_PUB_KEY_256;

  /*Look for the object corresponding to 256-bit verification key message, 1 single object, no filter on session*/
  if (FindObjectsFromTemplate(CKV_SESSION_HANDLE_ANY, &h_obj, 1, &ul_obj_cnt, &template_tmp, 1) == CKR_OK)
  {
    /* Get a pointer to the template of the object  */
    if (STSAFE_OBJ_GetTemplateFromObjectHandle(&p_template_embedded, &ul_count_embedded, h_obj) == CKR_OK)
    {
      /*Compare embedded referefence object with input template filtered of 'unique' values (e.g. CKA_EC_POINT).*/
      if (STSAFE_OBJ_CompareTemplates(p_template_filtered, ul_count_filtered, p_template_embedded,
                                      ul_count_embedded) == CKR_OK)
      {
        /*All attributes of input template are consistent with the pre-allocated object*/
        object_found = CK_TRUE;
      }
    }
  }

  /*If object was not found - try with NIST-384 bit verification key*/
  if (object_found == CK_FALSE)
  {
    template_tmp.type = CKA_LABEL;
    template_tmp.pValue = P11_OBJ_LABEL_VER_PUB_KEY_384;

    /*Look for the object corresponding to 384-bit verification key message, 1 single object, no filter on session*/
    if (FindObjectsFromTemplate(CKV_SESSION_HANDLE_ANY, &h_obj, 1, &ul_obj_cnt, &template_tmp, 1) == CKR_OK)
    {
      /* Get a pointer to the template of the object  */
      if (STSAFE_OBJ_GetTemplateFromObjectHandle(&p_template_embedded, &ul_count_embedded, h_obj) == CKR_OK)
      {
        /*Compare embedded referefence object with input template filtered of 'unique' values (e.g. CKA_EC_POINT).*/
        if (STSAFE_OBJ_CompareTemplates(p_template_filtered, ul_count_filtered, p_template_embedded,
                                        ul_count_embedded) == CKR_OK)
        {
          /*All attributes of input template are consistent with the pre-allocated object*/
          object_found = CK_TRUE;
        }
      }
    }
  }

  /* Check if both templates were not ok for the input object*/
  if (object_found == CK_FALSE)
  {
    /* Return: the template specified in input can't be used to create an object for this implementation*/
    return CKR_ATTRIBUTE_TYPE_INVALID;
  }

  /* ======================== Update the pre-allocated object =================== */
  /* Input template is consistent with one of the two pre-allocated objects.
    * Overwrite the CKA_EC_POINT attribute of the pre-allocated object. */

  /* Extract CKA_EC_POINT attribute from input template */
  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_input, CKA_EC_POINT, pTemplate, ulCount) == CKR_OK)
  {
    /* Extract CKA_EC_POINT attribute pointer from pre-allocated object template */
    if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_embedded, CKA_EC_POINT, p_template_embedded,
                                           ul_count_embedded) == CKR_OK)
    {
      /* Check if the pre-allocated object has enough memory to store the new input value*/
      if (p_attribute_input->ulValueLen <= p_attribute_embedded->ulValueLen)
      {
        /* Update the embedded object pValue */
        e_ret_status = STSAFE_OBJ_SetAttributeValue(p_attribute_embedded, p_attribute_input->pValue,
                                                    p_attribute_input->ulValueLen);

      }
      else
      {
        /* The pre-allocated value doesn't have enough memory to store the new value*/
        e_ret_status = CKR_DEVICE_MEMORY;
      }
    }
    else
    {
      /* It means pre-allocated template does not have CKA_EC_POINT attribute - should never get here*/
      e_ret_status = CKR_GENERAL_ERROR;
    }
  }
  else
  {
    /* It means input template does not have CKA_EC_POINT attribute - should never get here*/
    e_ret_status = CKR_ATTRIBUTE_TYPE_INVALID;
  }


  /* if error, return */
  if (e_ret_status != CKR_OK)
  {
    return e_ret_status;
  }

  /* ========== Assign the object to the session ========== */
  if (ObjectSetSession(h_obj, hSession) != CKR_OK)
  {
    return CKR_FUNCTION_FAILED;
  }

  /* ========== Return the new object handle  ========== */



  /*Return the object handle*/
  *phObject = h_obj;

  return CKR_OK;
}

/**
  * @brief C_CopyObject copies an object, creating a new object for the copy.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession     the session's handle
  * @param  hObject     the object's handle
  * @param  pTemplate     template for the new object
  * @param  ulCount     attributes in template
  * @param  phNewObject     receives handle of copy.
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED
  */
static CK_RV STSAFE_P11_CopyObject(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hObject, CK_ATTRIBUTE_PTR pTemplate,
                                   CK_ULONG ulCount, CK_OBJECT_HANDLE_PTR phNewObject)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_DestroyObject destroys an object.
  * @note   Delete certificate not supported by this release
  * @param  hSession     the session's handle
  * @param  hObject     the object's handle
  * @retval CK_RV CKR_ACTION_PROHIBITED
  */

//CK_RV STSAFE_OBJ_DeleteTemplatePtrFromObjectHandle(CK_OBJECT_HANDLE hObject);

static CK_RV STSAFE_P11_DestroyObject(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hObject)
{
  return CKR_ACTION_PROHIBITED; /* Valid for certificate*/

}
/**
  * @brief  C_GetObjectSize gets the size of an object in bytes.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession     the session's handle
  * @param  hObject     the object's handle
  * @param  pulSize     receives size of object
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED
  */
static CK_RV STSAFE_P11_GetObjectSize(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hObject, CK_ULONG_PTR pulSize)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_GetAttributeValue obtains the value of one or more object attributes
  * @param  hSession     the session's handle
  * @param  hObject     the object's handle
  * @param  pTemplate    specifies attrs; gets vals
  * @param  ulCount     attributes in template
  * @retval CK_RV CKR_ARGUMENTS_BAD, CKR_CRYPTOKI_NOT_INITIALIZED, CKR_SESSION_HANDLE_INVALID,
  *               CKR_SESSION_CLOSED, CKR_OBJECT_HANDLE_INVALID, CKR_FUNCTION_FAILED, CKR_ATTRIBUTE_SENSITIVE,
  *               CKR_ATTRIBUTE_TYPE_INVALID, CKR_BUFFER_TOO_SMALL, CKR_OK.
  */
static CK_RV STSAFE_P11_GetAttributeValue(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE  hObject,
                                          CK_ATTRIBUTE_PTR  pTemplate, CK_ULONG  ulCount)
{
  uint32_t i;
  CK_RV e_ret_status;
  CK_ATTRIBUTE_PTR  p_template_embedded;
  CK_ATTRIBUTE_PTR  p_attribute;
  CK_ULONG  ul_count_embedded;
  CK_ULONG ul_value_len_tmp;

  /* Check input parameters */
  if ((pTemplate == NULL_PTR) || (ulCount == 0UL))
  {
    return CKR_ARGUMENTS_BAD;
  }

  /* ========== Check PKCS11 status and session  ========== */

  /* Check if PKCS11 module has already been initialized */
  if (uInitialized == CK_FALSE)
  {
    return CKR_CRYPTOKI_NOT_INITIALIZED;
  }

  /* Check if the Session handle specified in input is valid */
  if (SessionIsValid(hSession) != CKR_OK)
  {
    return CKR_SESSION_HANDLE_INVALID;
  }

  /* Check if Session is open */
  if (SessionIsOpen(hSession) != CKR_OK)
  {
    return CKR_SESSION_CLOSED;
  }

  /* ========================= Look for the specific object ============================== */

  /* Get a pointer to the template of the embedded object corresponding to hObject handle specified in input */
  if (STSAFE_OBJ_GetTemplateFromObjectHandle(&p_template_embedded, &ul_count_embedded, hObject) != CKR_OK)
  {
    /*Handle specified is not a valid object*/
    return CKR_OBJECT_HANDLE_INVALID;
  }

  /* ========== Copy embedded object attributes to the template provided in input ========== */
  /*
    * For each (type, pValue, ulValueLen) triple in the template, C_GetAttributeValue performs the following algorithm:
    * 1. If the specified attribute (i.e., the attribute specified by the type field) for the object cannot be
    *    revealed because the object is sensitive or unextractable, then the ulValueLen field in that triple
    *    is modified to hold the value CK_UNAVAILABLE_INFORMATION.
    * 2. Otherwise, if the specified value for the object is invalid (the object does not possess such an
    *    attribute), then the ulValueLen field in that triple is modified to hold the value CK_UNAVAILABLE_INFORMATION.
    * 3. Otherwise, if the pValue field has the value NULL_PTR, then the ulValueLen field is modified to hold the
    *    exact length of the specified attribute for the object.
    * 4. Otherwise, if the length specified in ulValueLen is large enough to hold the value of the specified
    *    attribute for the object,then that attribute is copied into the buffer located at pValue, and the ulValueLen
    *    field is modified to hold the exact length of the attribute.
    * 5. Otherwise, the ulValueLen field is modified to hold the value CK_UNAVAILABLE_INFORMATION.
    */

  /*Init*/
  e_ret_status = CKR_OK;

  /*Loop on all the attributes of input Template*/
  for (i = 0; i < ulCount; i++)
  {
    /* Look for the attribute in the corresponding embedded object */
    if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute, pTemplate[i].type, p_template_embedded,
                                           ul_count_embedded) == CKR_OK)
    {
      /* Check if the attribute is sensitive */
      if (IsAttributeSensitive(pTemplate[i].type, p_template_embedded, ul_count_embedded) == CK_TRUE)
      {
        /* The attribute specified by the type field for the object cannot be revealed because the object is sensitive
         * or unextractable, then the ulValueLen field in that triple is modified to hold
         * the value CK_UNAVAILABLE_INFORMATION.*/
        pTemplate[i].ulValueLen = CK_UNAVAILABLE_INFORMATION;

        /* If case 1 applies to any of the requested attributes, then the call should return the value
         * CKR_ATTRIBUTE_SENSITIVE*/
        e_ret_status = CKR_ATTRIBUTE_SENSITIVE;
      }
      else
      {
        /* Get Value Len*/
        if (GetAttributeValueFromTemplate(NULL_PTR, &ul_value_len_tmp, pTemplate[i].type, p_template_embedded,
                                          ul_count_embedded) != CKR_OK)
        {
          /*Return immediately*/
          return CKR_FUNCTION_FAILED;
        }

        /* Check if inpute template provided a pValue pointer to store the content for that attribute */
        if (pTemplate[i].pValue == NULL_PTR)
        {
          /* If the pValue field has the value NULL_PTR, then the ulValueLen field is modified to hold the exact length
           * of the specified attribute for the object */
          pTemplate[i].ulValueLen = ul_value_len_tmp;
        }
        else
        {
          /* Otherwise, if the length specified in ulValueLen is large enough to hold the value of the specified
           * attribute for the object, then that attribute is
           * copied into the buffer located at pValue, and the ulValueLen field is modified to hold the exact
           * length of the attribute*/
          if (pTemplate[i].ulValueLen >= ul_value_len_tmp)
          {
            if (GetAttributeValueFromTemplate(pTemplate[i].pValue, &pTemplate[i].ulValueLen, pTemplate[i].type,
                                              p_template_embedded,
                                              ul_count_embedded) != CKR_OK)
            {
              /*Return immediately*/
              return CKR_FUNCTION_FAILED;
            }
          }
          else
          {
            /* Otherwise, the ulValueLen field is modified to hold the value CK_UNAVAILABLE_INFORMATION
             * return is CKR_BUFFER_TOO_SMALL */
            pTemplate[i].ulValueLen = CK_UNAVAILABLE_INFORMATION;
            e_ret_status = CKR_BUFFER_TOO_SMALL;
          }
        }
      }
    }
    else
    {
      /* The object does not possess such an attribute,then the ulValueLen field in that triple
       * is modified to hold the value CK_UNAVAILABLE_INFORMATION.*/
      pTemplate[i].ulValueLen = CK_UNAVAILABLE_INFORMATION;

      /* If case 2 applies to any of the requested attributes, then the call should return
       * the value CKR_ATTRIBUTE_TYPE_INVALID*/
      e_ret_status = CKR_ATTRIBUTE_TYPE_INVALID;
    }
  }

  /* Only if none of the return errors applies to any of the requested attributes CKR_OK is returned.*/
  if ((e_ret_status != CKR_ATTRIBUTE_SENSITIVE) && (e_ret_status != CKR_ATTRIBUTE_TYPE_INVALID)
      && (e_ret_status != CKR_BUFFER_TOO_SMALL))
  {
    e_ret_status = CKR_OK;
  }

  /*As usual, if more than one of the error codes is applicable, Cryptoki may return any of them*/
  return e_ret_status;
}

/**
  * @brief  C_SetAttributeValue modifies the value of one or more object attributes.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession     the session's handle
  * @param  hObject     the object's handle
  * @param  pTemplate    specifies attrs and values
  * @param  ulCount     attributes in template
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_SetAttributeValue(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE  hObject,
                                          CK_ATTRIBUTE_PTR  pTemplate, CK_ULONG          ulCount)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}



/**
  * @brief  C_FindObjectsInit initializes a search for token and session objects that match a template
  * @param  hSession     the session's handle
  * @param  pTemplate    attribute values to match
  * @param  ulCount     attrs in search template
  * @retval CK_RV CKR_ARGUMENTS_BAD, CKR_CRYPTOKI_NOT_INITIALIZED, CKR_SESSION_HANDLE_INVALID,
  *               CKR_SESSION_CLOSED, CKR_OPERATION_ACTIVE, CKR_FUNCTION_FAILED, CKR_OK.
  */
static CK_RV STSAFE_P11_FindObjectsInit(CK_SESSION_HANDLE hSession, CK_ATTRIBUTE_PTR pTemplate,
                                        CK_ULONG ulCount)
{
  CK_RV e_ret_status;
  CK_OBJECT_HANDLE h_found_object[STSAFE_P11_NUMBER_OBJECTS];
  CK_ULONG ul_object_count;

  /*Check input parameters*/
  /*Note: pTemplate = NULL_PTR is ok only if ulCount = 0*/
  if (((pTemplate == NULL_PTR) && (ulCount > 0U)) || (ulCount > STSAFE_P11_MAX_TEMPLATE_SIZE))
  {
    return CKR_ARGUMENTS_BAD;
  }

  /* ========== Check PKCS11 status and session  ========== */

  /* Check if PKCS11 module has already been initialized */
  if (uInitialized == CK_FALSE)
  {
    return CKR_CRYPTOKI_NOT_INITIALIZED;
  }

  /* Check if the Session handle specified in input is valid */
  if (SessionIsValid(hSession) != CKR_OK)
  {
    return CKR_SESSION_HANDLE_INVALID;
  }

  /* Check if Session is open */
  if (SessionIsOpen(hSession) != CKR_OK)
  {
    return CKR_SESSION_CLOSED;
  }

  /* ========== Check active operation status ========== */

  /* Check if there is already an operation active */
  if (SessionOperationIsActive(hSession) == CKR_OK)
  {
    return CKR_OPERATION_ACTIVE;
  }

  /* ========== Look for the objects  ========== */
  /*Note: if ulCount was 0, phObject will be filled with a list of all objects handles up to STSAFE_P11_NUMBER_OBJECTS*/
  e_ret_status = FindObjectsFromTemplate(hSession, &h_found_object[0U], sizeof(h_found_object) /
                                         sizeof(h_found_object[0]), &ul_object_count, pTemplate, ulCount) ;

  if (e_ret_status != CKR_OK)
  {
    return e_ret_status;
  }

  /* ========== Store the objects found corresponding to input template ========== */
  if (SessionOperationSetObjects(hSession, &h_found_object[0U], ul_object_count) != CKR_OK)
  {
    return CKR_FUNCTION_FAILED;
  }


  return CKR_OK;
}

/**
  * @brief  C_FindObjects continues a search for token and session objects that
  *          match a template, obtaining additional object handles
  * @param  hSession     session's handle
  * @param  phObject    gets obj. handles
  * @param  ulMaxObjectCount    max handles to get
  * @param  pulObjectCount    actual # returned
  * @retval CK_RV CKR_ARGUMENTS_BAD,CKR_CRYPTOKI_NOT_INITIALIZED, CKR_SESSION_HANDLE_INVALID,
          CKR_SESSION_CLOSED, CKR_OPERATION_NOT_INITIALIZED, CKR_FUNCTION_FAILED,
          CKR_ATTRIBUTE_VALUE_INVALID, CKR_OK.
  */
static CK_RV STSAFE_P11_FindObjects(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE_PTR phObject,
                                    CK_ULONG ulMaxObjectCount,  CK_ULONG_PTR pulObjectCount)
{

  /*Check input parameters*/
  if ((phObject == NULL_PTR) || (pulObjectCount == NULL_PTR) || (ulMaxObjectCount <= 0UL))
  {
    return CKR_ARGUMENTS_BAD;
  }

  /* ========== Check PKCS11 status and session  ========== */

  /* Check if PKCS11 module has already been initialized */
  if (uInitialized == CK_FALSE)
  {
    return CKR_CRYPTOKI_NOT_INITIALIZED;
  }

  /* Check if the Session handle specified in input is valid */
  if (SessionIsValid(hSession) != CKR_OK)
  {
    return CKR_SESSION_HANDLE_INVALID;
  }

  /* Check if Session is open */
  if (SessionIsOpen(hSession) != CKR_OK)
  {
    return CKR_SESSION_CLOSED;
  }

  /* ========== Check active operation status ========== */

  /* Check if there is a pending operation: i.e. FindObjectInit was called*/
  if (SessionOperationIsActive(hSession) != CKR_OK)
  {
    return CKR_OPERATION_NOT_INITIALIZED;
  }

  /* ========== Get active operation objects ========== */
  if (SessionOperationGetObjects(hSession, phObject, ulMaxObjectCount, pulObjectCount) != CKR_OK)
  {
    return CKR_FUNCTION_FAILED;
  }


  return CKR_OK;
}

/**
  * @brief  C_FindObjectsFinal finishes a search for token and session objects.
  * @param  hSession     session's handle
  * @retval CK_RV CKR_CRYPTOKI_NOT_INITIALIZED, CKR_SESSION_HANDLE_INVALID, CKR_SESSION_CLOSED,
  *               CKR_OPERATION_NOT_INITIALIZED, CKR_FUNCTION_FAILED, CKR_OK.
  */
static CK_RV STSAFE_P11_FindObjectsFinal(CK_SESSION_HANDLE hSession)
{

  /* ========== Check PKCS11 status and session  ========== */

  /* Check if PKCS11 module has already been initialized */
  if (uInitialized == CK_FALSE)
  {
    return CKR_CRYPTOKI_NOT_INITIALIZED;
  }

  /* Check if the Session handle specified in input is valid */
  if (SessionIsValid(hSession) != CKR_OK)
  {
    return CKR_SESSION_HANDLE_INVALID;
  }

  /* Check if Session is open */
  if (SessionIsOpen(hSession) != CKR_OK)
  {
    return CKR_SESSION_CLOSED;
  }

  /* ========== Check active operation status ========== */

  /* Check if there is FindObjectInit was called*/
  if (SessionOperationIsActive(hSession) != CKR_OK)
  {
    return CKR_OPERATION_NOT_INITIALIZED;
  }

  /* ========== Reset active operation  ========== */

  if (SessionOperationFinish(hSession) != CKR_OK)
  {
    return CKR_FUNCTION_FAILED;
  }

  return CKR_OK;
}
/**
  * @brief  C_EncryptInit initializes an encryption operation.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession     the session's handle
  * @param  pMechanism    the encryption mechanism
  * @param  hKey    handle of encryption key
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_EncryptInit(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hKey)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_Encrypt encrypts single-part data.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession     the session's handle
  * @param  pData    the plaintext data
  * @param  ulDataLen     bytes of plaintext
  * @param  pEncryptedData    gets ciphertext
  * @param  pulEncryptedDataLen   gets c-text size
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_Encrypt(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pData, CK_ULONG ulDataLen,
                                CK_BYTE_PTR pEncryptedData, CK_ULONG_PTR pulEncryptedDataLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_EncryptUpdate continues a multiple-part encryption operation.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession     the session's handle
  * @param  pPart    the plaintext data
  * @param  ulPartLen     bytes of plaintext
  * @param  pEncryptedPart    gets ciphertext
  * @param  pulEncryptedPartLen   gets c-text size
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_EncryptUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pPart, CK_ULONG ulPartLen,
                                      CK_BYTE_PTR pEncryptedPart, CK_ULONG_PTR pulEncryptedPartLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_EncryptFinal finishes a multiple-part encryption operation.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession     the session's handle
  * @param  pLastEncryptedPart    last c-text
  * @param  pulLastEncryptedPartLen   gets last size
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_EncryptFinal(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pLastEncryptedPart,
                                     CK_ULONG_PTR pulLastEncryptedPartLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_DecryptInit initializes a decryption operation.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession     the session's handle
  * @param  pMechanism    the decryption mechanism
  * @param  hKey    handle of decryption key
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_DecryptInit(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hKey)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_Decrypt decrypts encrypted data in a single part.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession     the session's handle
  * @param  pEncryptedData    ciphertext
  * @param  ulEncryptedDataLen     ciphertext length
  * @param  pData    gets plaintext
  * @param  pulDataLen   gets p-text size
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_Decrypt(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pEncryptedData, CK_ULONG ulEncryptedDataLen,
                                CK_BYTE_PTR pData, CK_ULONG_PTR pulDataLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_DecryptUpdate continues a multiple-part decryption operation.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession     the session's handle
  * @param  pEncryptedData    encrypted data
  * @param  ulEncryptedPartLen     input length
  * @param  pPart    gets plaintext
  * @param  pulPartLen   p-text size
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_DecryptUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pEncryptedPart,
                                      CK_ULONG ulEncryptedPartLen, CK_BYTE_PTR pPart, CK_ULONG_PTR pulPartLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_DecryptFinal finishes a multiple-part decryption operation.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession     the session's handle
  * @param  pLastPart   gets plaintext
  * @param  pulLastPartLen     p-text size
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_DecryptFinal(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pLastPart, CK_ULONG_PTR pulLastPartLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_DigestInit initializes a message-digesting operation.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession     the session's handle
  * @param  pMechanism    the digesting mechanism
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_DigestInit(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_Digest digests data in a single part.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession     the session's handle
  * @param  pData     data to be digested
  * @param  ulDataLen     bytes of data to digest
  * @param  pDigest     gets the message digest
  * @param  pulDigestLen     gets digest length
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_Digest(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pData, CK_ULONG ulDataLen, CK_BYTE_PTR pDigest,
                               CK_ULONG_PTR pulDigestLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_DigestUpdate continues a multiple-part message-digesting operation.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession  hSession,   the session's handle
  * @param  pData     pPart,      data to be digested
  * @param  ulDataLen ulPartLen   bytes of data to be digested
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_DigestUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pPart, CK_ULONG ulPartLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_DigestKey continues a multi-part message-digesting
  *         operation, by digesting the value of a secret key as part of
  *         the data already digested.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession  hSession,   the session's handle
  * @param  pData     hKey        secret key to digest
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_DigestKey(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE hKey)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_DigestFinal finishes a multiple-part message-digesting operation.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession,     the session's handle
  * @param  pDigest,      gets the message digest
  * @param  pulDigestLen  gets byte count of digest
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_DigestFinal(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pDigest, CK_ULONG_PTR pulDigestLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}

/**
  * @brief  C_SignInit initializes a signature (private key encryption)
  *         operation, where the signature is (will be) an appendix to
  *         the data, and plaintext cannot be recovered from the
  *         signature.
  * @note   In this implementation only operations on objects with the following attributes are allowed:
  *          - CKA_CLASS = CKO_PRIVATE_KEY
  *          - CKA_KEY_TYPE = CKK_EC
  *          - CKA_SIGN = CK_TRUE
  * @param  hSession,    the session's handle
  * @param  pMechanism,  the signature mechanism
  * @param  hKey         handle of signature key
  * @retval CK_RV CKR_ARGUMENTS_BAD,CKR_CRYPTOKI_NOT_INITIALIZED, CKR_SESSION_HANDLE_INVALID,
  *               CKR_SESSION_CLOSED, CKR_OPERATION_ACTIVE, CKR_KEY_HANDLE_INVALID, CKR_MECHANISM_INVALID,
  *               CKR_KEY_TYPE_INCONSISTENT,CKR_KEY_FUNCTION_NOT_PERMITTED, CKR_FUNCTION_FAILED, CKR_OK.
  */
static CK_RV STSAFE_P11_SignInit(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR  pMechanism,
                                 CK_OBJECT_HANDLE  hObject)
{
  CK_ATTRIBUTE_PTR p_attribute_tmp;
  CK_ATTRIBUTE_PTR  p_template_embedded;
  CK_ULONG  ul_count_embedded;

  /*Check input parameters*/
  if (pMechanism == NULL_PTR)
  {
    return CKR_ARGUMENTS_BAD;
  }

  /* ========== Check PKCS11 status and session  ========== */

  /* Check if PKCS11 module has already been initialized */
  if (uInitialized == CK_FALSE)
  {
    return CKR_CRYPTOKI_NOT_INITIALIZED;
  }

  /* Check if the Session handle specified in input is valid */
  if (SessionIsValid(hSession) != CKR_OK)
  {
    return CKR_SESSION_HANDLE_INVALID;
  }

  /* Check if Session is open */
  if (SessionIsOpen(hSession) != CKR_OK)
  {
    return CKR_SESSION_CLOSED;
  }

  /* ========== Check active operation status ========== */

  /* Check if there is already an operation active */
  if (SessionOperationIsActive(hSession) == CKR_OK)
  {
    return CKR_OPERATION_ACTIVE;
  }

  /* ========== Get the template of the object handle specified in input ========== */

  /* Get a pointer to the template of the embedded object corresponding to hObject handle specified in input */
  if (STSAFE_OBJ_GetTemplateFromObjectHandle(&p_template_embedded, &ul_count_embedded, hObject) != CKR_OK)
  {
    return CKR_KEY_HANDLE_INVALID;
  }

  /* ========== Check CKA_ALLOWED_MECHANISMS attribute ========== */

  /* Extract CKA_ALLOWED_MECHANISMS attribute from the template */
  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_ALLOWED_MECHANISMS, p_template_embedded,
                                         ul_count_embedded) != CKR_OK)
  {
    return CKR_KEY_HANDLE_INVALID;
  }

  /*Check if the mechanism type specified in input is allowed, if not return CKR_MECHANISM_INVALID*/
  if (IsMechanismTypeAllowed((CK_MECHANISM_TYPE)(pMechanism->mechanism), (CK_MECHANISM_PTR)p_attribute_tmp->pValue,
                             p_attribute_tmp->ulValueLen) != CK_TRUE)
  {
    return CKR_MECHANISM_INVALID;
  }

  /* ========== Check CKA_CLASS attribute ========== */

  /* Extract CKA_CLASS attribute from the template */
  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_CLASS, p_template_embedded, ul_count_embedded) != CKR_OK)
  {
    return CKR_KEY_HANDLE_INVALID;
  }

  /*Check if CKA_CLASS == CKO_PRIVATE_KEY, if not return CKR_KEY_TYPE_INCONSISTENT*/
  if (*((CK_OBJECT_CLASS *)(p_attribute_tmp->pValue)) != CKO_PRIVATE_KEY)
  {
    return CKR_KEY_TYPE_INCONSISTENT;
  }

  /* ========== Check CKA_KEY_TYPE attribute ========== */

  /* Extract CKA_KEY_TYPE attribute from the template */
  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_KEY_TYPE, p_template_embedded,
                                         ul_count_embedded) != CKR_OK)
  {
    return CKR_KEY_HANDLE_INVALID;
  }

  /*Check if CKA_KEY_TYPE == CKK_EC, if not return CKR_KEY_TYPE_INCONSISTENT*/
  if (*((CK_KEY_TYPE *)(p_attribute_tmp->pValue)) != CKK_EC)
  {
    return CKR_KEY_TYPE_INCONSISTENT;
  }

  /* ========== Check CKA_SIGN attribute ========== */

  /* Extract CKA_SIGN attribute from the template */
  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_SIGN, p_template_embedded, ul_count_embedded) != CKR_OK)
  {
    return CKR_KEY_HANDLE_INVALID;
  }

  /*Check if CKA_SIGN == CK_TRUE, if not return CKR_KEY_FUNCTION_NOT_PERMITTED*/
  if (*((CK_BBOOL *)(p_attribute_tmp->pValue)) != CK_TRUE)
  {
    return CKR_KEY_FUNCTION_NOT_PERMITTED;
  }

  /* ========== Store the object selected for the operation ========== */
  if (SessionOperationSetObjects(hSession, &hObject, 1U) != CKR_OK)
  {
    return CKR_FUNCTION_FAILED;
  }

  return CKR_OK;
}


/**
  * @brief  C_Sign signs (encrypts with private key) data in a single
  *         part, where the signature is (will be) an appendix to the
  *         data, and plaintext cannot be recovered from the signature.
  * @note   This function uses the convention for functions returning output in a variable-length buffer in producing
  *         output.
  *         Two of the arguments to the function are a pointer to the output buffer (say pBuf) and a pointer to a
  *         location which will hold the length of the output produced (say pulBufLen).
  *         There are two ways for an application to call such a function
  *         1.  If pBuf is NULL_PTR, then all that the function does is return (in *pulBufLen) a number of bytes which
  *             would suffice to hold the cryptographic output produced from the input to the function.
  *             This number may somewhat exceed the precise number of bytes needed,
  *             but should not exceed it by a large amount.  CKR_OK is returned by the function.
  *         2.  If pBuf is not NULL_PTR, then *pulBufLen MUST contain the size in bytes of the buffer pointed to by
  *             pBuf.
  *             If that buffer is large enough to hold the cryptographic output produced from the input to the
  *             function, then that cryptographic output is placed there,
  *             and CKR_OK is returned by the function.  If the buffer is not large enough, then CKR_BUFFER_TOO_SMALL
  *             is returned.
  *         In either case, *pulBufLen is set to hold the exact number of bytes needed to hold the cryptographic output
  *         produced from the input to the function.
  * @param  hSession,        the session's handle
  * @param  pData,           the data to sign
  * @param  ulDataLen,       count of bytes to sign
  * @param  pSignature,      gets the signature
  * @param  pulSignatureLen  gets signature length
  * @retval CK_RV CKR_ARGUMENTS_BAD,CKR_CRYPTOKI_NOT_INITIALIZED, CKR_SESSION_HANDLE_INVALID,
  *               CKR_SESSION_CLOSED, CKR_OPERATION_NOT_INITIALIZED, CKR_FUNCTION_FAILED, CKR_DATA_LEN_RANGE,
  *               CKR_BUFFER_TOO_SMALL, CKR_OK.
  */
static CK_RV STSAFE_P11_Sign(CK_SESSION_HANDLE     hSession, CK_BYTE_PTR pData,
                             CK_ULONG  ulDataLen, CK_BYTE_PTR  pSignature, CK_ULONG_PTR pulSignatureLen)
{
  CK_ATTRIBUTE_PTR p_attribute_tmp, p_template;
  CK_OBJECT_HANDLE h_object;
  CK_ULONG ul_count;
  uint32_t ul_data_len;
  uint32_t ul_sign_out_len;
  uint32_t ul_r_length;
  uint32_t ul_s_length;
  uint8_t u_sign_in_len;  
  StSafeA_HashTypes_t e_hash_type;
   
  StSafeA_LVBuffer_t SignR, SignS;
#if (!STSAFEA_USE_OPTIMIZATION_SHARED_RAM)
  uint8_t data_SignR [STSAFEA_XYRS_ECDSA_SHA256_LENGTH];
  SignR.Length = STSAFEA_XYRS_ECDSA_SHA256_LENGTH;
  SignR.Data = data_SignR;
  uint8_t data_SignS [STSAFEA_XYRS_ECDSA_SHA256_LENGTH];
  SignS.Length = STSAFEA_XYRS_ECDSA_SHA256_LENGTH;
  SignS.Data = data_SignS;
#endif
  
  /*Check input parameters*/
  if ((pData == NULL_PTR) || (ulDataLen == 0UL) || (pulSignatureLen == NULL_PTR))
  {
    return CKR_ARGUMENTS_BAD;
  }

  /* ========== Check PKCS11 status and session  ========== */

  /* Check if PKCS11 module has already been initialized */
  if (uInitialized == CK_FALSE)
  {
    return CKR_CRYPTOKI_NOT_INITIALIZED;
  }

  /* Check if the Session handle specified in input is valid */
  if (SessionIsValid(hSession) != CKR_OK)
  {
    SessionOperationFinish(hSession);
    return CKR_SESSION_HANDLE_INVALID;
  }

  /* Check if Session is open */
  if (SessionIsOpen(hSession) != CKR_OK)
  {
    return CKR_SESSION_CLOSED;
  }

  /* Check if there is already an operation active */
  if (SessionOperationIsActive(hSession) != CKR_OK)
  {
    return CKR_OPERATION_NOT_INITIALIZED;
  }

  /* ========== Get active operation object ========== */
  if (SessionOperationGetObjects(hSession, &h_object, 1, &ul_count) != CKR_OK)
  {
    SessionOperationFinish(hSession);
    return CKR_FUNCTION_FAILED;
  }

  /* Get a pointer to the object using hobject specified in input */
  if (STSAFE_OBJ_GetTemplateFromObjectHandle(&p_template, &ul_count, h_object) != CKR_OK)
  {
    SessionOperationFinish(hSession);
    return CKR_FUNCTION_FAILED;
  }

  /* ========== Check CKA_EC_PARAMS attribute ========== */

  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_EC_PARAMS, p_template, ul_count) != CKR_OK)
  {
    SessionOperationFinish(hSession);
    return CKR_FUNCTION_FAILED;
  }

  /*Extract curve and data len from key OID */
  if (memcmp(p_attribute_tmp->pValue, STSAFEA_ECC_CURVE_OID_NIST_P_256_DER, p_attribute_tmp->ulValueLen) == 0)
  {
    /* NIST P-256*/
    e_hash_type = STSAFEA_SHA_256;
    ul_data_len =  STSAFEA_SHA_256_LENGTH;
    ul_sign_out_len = (STSAFEA_SHA_256_LENGTH * 2);
    u_sign_in_len = STSAFEA_LENGTH_CMD_RESP_GENERATE_SIGNATURE_SHA_256;

  }
  else if (memcmp(p_attribute_tmp->pValue, STSAFEA_ECC_CURVE_OID_NIST_P_384_DER, p_attribute_tmp->ulValueLen) == 0)
  {
    /* NIST P-384*/
    e_hash_type = STSAFEA_SHA_384;
    ul_data_len =  STSAFEA_SHA_384_LENGTH;
    ul_sign_out_len = (STSAFEA_SHA_384_LENGTH * 2);
    u_sign_in_len = STSAFEA_LENGTH_CMD_RESP_GENERATE_SIGNATURE_SHA_384;
  }
  else
  {
    /* It should never get here*/
    SessionOperationFinish(hSession);
    return CKR_FUNCTION_FAILED;
  }

  /* ========== Check input data len ========== */
  if (ulDataLen != ul_data_len)
  {
    SessionOperationFinish(hSession);
    return CKR_DATA_LEN_RANGE;
  }

  /* ========== Check signature buffer========== */
  if (pSignature != NULL_PTR)
  {
    /* ========== Check CKA_EXT_TOKEN_PRVKEY_SLOT attribute========== */
    /* This vendor defined attribute links one PKCS11 object to a specific feature of STSAFE*/

    /* Extract from the template the external token slot*/
    if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_EXT_TOKEN_PRVKEY_SLOT, p_template, ul_count) != CKR_OK)
    {
      return CKR_FUNCTION_FAILED;
    }

    /* ========== Sign with STSAFE  ========== */

    /* Generate Signature */
    if (StSafeA_GenerateSignature(&hStSafe, __STSAFEA_GET_SLOT_ID_FROM_CK(p_attribute_tmp->pValue), pData, e_hash_type,
                                  u_sign_in_len, &SignR, &SignS, STSAFEA_MAC_NONE, STSAFEA_ENCRYPTION_NONE) != STSAFEA_OK)    
    {
      /*Reset Signature Session */
      SessionOperationFinish(hSession);
      return CKR_FUNCTION_FAILED;
    }

    /* ========== Prepare output data  ========== */

    /* STSAFE output is formatted as per the following: ( R_LENGHT (2 bytes) | R | S_LENGHT(2bytes) | S) */

    /* Extract Signature R length */
    
    ul_r_length = (uint32_t) SignR.Length; 

    /* Extract Signature S length */
    
    ul_s_length = (uint32_t) SignS.Length;     

    /* ========== Check Buffer size  ========== */
    /* Note: parameter pulSignatureLen is used in input/output mode - the one specified in input must specify a size
     * big enough to store the output*/
    if (*pulSignatureLen < (ul_r_length + ul_s_length))
    {
      /* If the buffer is not large enough, then CKR_BUFFER_TOO_SMALL is returned.*/
      *pulSignatureLen = ul_r_length + ul_s_length;

      /*Return and not close active operation*/
      return CKR_BUFFER_TOO_SMALL;
    }

    /* ========== Copy the signature in output  ========== */

    /* Copy Signature R */
    (void)memcpy(&pSignature[0U], &SignR.Data[0], ul_r_length);

    /* Copy Signature S */    
    (void)memcpy(&pSignature[ul_r_length], &SignS.Data[0], ul_s_length);

    /* Copy signature len */
    *pulSignatureLen = ul_r_length + ul_s_length;

    /* ========== Close Signature operation  ========== */
    /* Close Operation Session */
    SessionOperationFinish(hSession);
  }
  else
  {
    /* If pSignature is NULL_PTR, then all that the function does is return (in *pulBufLen) a number of bytes
    which would suffice to hold the cryptographic output produced from the input to the function */
    *pulSignatureLen = ul_sign_out_len;
  }

  return CKR_OK;
}

/**
  * @brief  C_SignUpdate continues a multiple-part signature operation,
  *         where the signature is (will be) an appendix to the data,
  *         and plaintext cannot be recovered from the signature.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param hSession, the session's handle
  * @param pPart,    the data to sign
  * @param ulPartLen count of bytes to sign
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_SignUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pPart, CK_ULONG ulPartLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}

/**
  * @brief  C_SignFinal finishes a multiple-part signature operation,
  *         returning the signature.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param hSession,        the session's handle
  * @param pSignature,      gets the signature
  * @param pulSignatureLen  gets signature length
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_SignFinal(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pSignature, CK_ULONG_PTR pulSignatureLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_SignRecoverInit initializes a signature operation, where
  * the data can be recovered from the signature.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param hSession,   the session's handle
  * @param pMechanism, the signature mechanism
  * @param hKey        handle of the signature key
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_SignRecoverInit(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hKey)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_SignRecover signs data in a single operation, where the
  * data can be recovered from the signature.
  * @param hSession,        the session's handle
  * @param pData,           the data to sign
  * @param ulDataLen,       count of bytes to sign
  * @param pSignature,      gets the signature
  * @param pulSignatureLen  gets signature length
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_SignRecover(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pData, CK_ULONG ulDataLen,
                                    CK_BYTE_PTR pSignature, CK_ULONG_PTR pulSignatureLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}

/**
  * @brief  C_VerifyInit initializes a verification operation, where the
  *         signature is an appendix to the data, and plaintext cannot
  *         cannot be recovered from the signature (e.g. DSA).
  * @note   In this implementation only operations on objects with the following attributes are allowed:
  *          - CKA_CLASS = CKO_PUBLIC_KEY
  *          - CKA_KEY_TYPE = CKK_EC
  *          - CKA_VERIFY = CK_TRUE
  * @param hSession    the session's handle
  * @param pMechanism  the verification mechanism
  * @param hKey        verification key
  * @retval CK_RV CKR_ARGUMENTS_BAD,CKR_CRYPTOKI_NOT_INITIALIZED, CKR_SESSION_HANDLE_INVALID,CKR_SESSION_CLOSED,
  *               CKR_OPERATION_ACTIVE, CKR_KEY_HANDLE_INVALID, CKR_KEY_TYPE_INCONSISTENT, CKR_MECHANISM_INVALID,
  *               CKR_KEY_FUNCTION_NOT_PERMITTED, CKR_FUNCTION_FAILED, CKR_OK.
  */
static CK_RV STSAFE_P11_VerifyInit(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR  pMechanism,
                                   CK_OBJECT_HANDLE  hObject)

{

  CK_ATTRIBUTE_PTR p_attribute_tmp;
  CK_ATTRIBUTE_PTR  p_template_embedded;
  CK_ULONG  ul_count_embedded;

  /*Check input parameters*/
  if (pMechanism == NULL_PTR)
  {
    return CKR_ARGUMENTS_BAD;
  }

  /* ========== Check PKCS11 status and session  ========== */

  /* Check if PKCS11 module has already been initialized */
  if (uInitialized == CK_FALSE)
  {
    return CKR_CRYPTOKI_NOT_INITIALIZED;
  }

  /* Check if the Session handle specified in input is valid */
  if (SessionIsValid(hSession) != CKR_OK)
  {
    return CKR_SESSION_HANDLE_INVALID;
  }

  /* Check if Session is open */
  if (SessionIsOpen(hSession) != CKR_OK)
  {
    return CKR_SESSION_CLOSED;
  }

  /* ========== Check active operation status ========== */

  /* Check that session not already used for another processing */
  if (SessionOperationIsActive(hSession) == CKR_OK)
  {
    return CKR_OPERATION_ACTIVE;
  }

  /* ========== Get the template of the object handle specified in input ========== */

  /* Get a pointer to the object template using hobject specified in input */
  if (STSAFE_OBJ_GetTemplateFromObjectHandle(&p_template_embedded, &ul_count_embedded, hObject) != CKR_OK)
  {
    return CKR_KEY_HANDLE_INVALID;
  }

  /* ========== Check CKA_ALLOWED_MECHANISMS attribute  ========== */

  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_ALLOWED_MECHANISMS, p_template_embedded,
                                         ul_count_embedded) != CKR_OK)
  {
    return CKR_KEY_HANDLE_INVALID;
  }

  /*Check if the mechanism type specified in input is allowed, if not return CKR_MECHANISM_INVALID*/
  if (IsMechanismTypeAllowed(pMechanism->mechanism, p_attribute_tmp->pValue, p_attribute_tmp->ulValueLen) != CK_TRUE)
  {
    return CKR_MECHANISM_INVALID;
  }

  /* ========== Check CKA_CLASS attribute  ========== */

  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_CLASS, p_template_embedded, ul_count_embedded) != CKR_OK)
  {
    return CKR_KEY_HANDLE_INVALID;
  }

  /*Check if CKA_CLASS == CKO_PUBLIC_KEY, if not return CKR_KEY_TYPE_INCONSISTENT*/
  if (*((CK_OBJECT_CLASS *)(p_attribute_tmp->pValue)) != CKO_PUBLIC_KEY)
  {
    return CKR_KEY_TYPE_INCONSISTENT;
  }

  /* ========== Check CKA_KEY_TYPE attribute  ========== */

  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_KEY_TYPE, p_template_embedded,
                                         ul_count_embedded) != CKR_OK)
  {
    return CKR_KEY_HANDLE_INVALID;
  }

  /*Check if CKA_KEY_TYPE == CKK_EC, if not return CKR_KEY_TYPE_INCONSISTENT*/
  if (*((CK_KEY_TYPE *)(p_attribute_tmp->pValue)) != CKK_EC)
  {
    return CKR_KEY_TYPE_INCONSISTENT;
  }

  /* ========== Check CKA_VERIFY attribute  ========== */

  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_VERIFY, p_template_embedded, ul_count_embedded)
      != CKR_OK)
  {
    return CKR_KEY_HANDLE_INVALID;
  }

  /*Check if CKA_VERIFY == CK_TRUE, if not return CKR_KEY_FUNCTION_NOT_PERMITTED*/
  if (*((CK_BBOOL *)(p_attribute_tmp->pValue)) != CK_TRUE)
  {
    return CKR_KEY_FUNCTION_NOT_PERMITTED;
  }

  /* ========== Store the object selected for the operation ========== */
  if (SessionOperationSetObjects(hSession, &hObject, 1) != CKR_OK)
  {
    return CKR_FUNCTION_FAILED;
  }

  return CKR_OK;
}


/**
  * @brief  C_Verify verifies a signature in a single-part operation,
  *         where the signature is an appendix to the data, and plaintext
  *         cannot be recovered from the signature.
  * @param hSession        the session's handle
  * @param pData           signed data
  * @param ulDataLen       length of signed data
  * @param pSignature      signature
  * @param ulSignatureLen  signature length
  * @retval CK_RV CKR_ARGUMENTS_BAD,CKR_CRYPTOKI_NOT_INITIALIZED, CKR_SESSION_HANDLE_INVALID,CKR_SESSION_CLOSED,
  *         CKR_OPERATION_NOT_INITIALIZED, CKR_SIGNATURE_LEN_RANGE, CKR_FUNCTION_FAILED, CKR_DATA_INVALID,
  *         CKR_DATA_LEN_RANGE, CKR_SIGNATURE_INVALID, CKR_OK.
  */
static CK_RV STSAFE_P11_Verify(CK_SESSION_HANDLE     hSession,  CK_BYTE_PTR   pData,
                               CK_ULONG ulDataLen, CK_BYTE_PTR  pSignature,
                               CK_ULONG ulSignatureLen)
{

  CK_OBJECT_HANDLE  h_object;
  CK_ATTRIBUTE_PTR p_attribute_tmp, p_template;
  CK_ULONG  ul_count, ul_object_count, ul_public_data_raw_len;;
  CK_BYTE  p_public_data_raw[2UL * STSAFEA_SHA_384_LENGTH];        /* Using 384 max len. */

  /*Allocate variables to communicate with STSAFE*/
  STSAFEA_STATIC_ALLOCATE_LV_BUFFER_VAR(StSafeA_LVBuffer_t, hash,
                                        STSAFEA_GET_HASH_SIZE((uint16_t)STSAFEA_SHA_384));  /* Using 384 max len. */
  STSAFEA_STATIC_ALLOCATE_LV_BUFFER_VAR(StSafeA_LVBuffer_t, sig_r,
                                        STSAFEA_XYRS_ECDSA_SHA384_LENGTH);  /* Using 384 max len. */
  STSAFEA_STATIC_ALLOCATE_LV_BUFFER_VAR(StSafeA_LVBuffer_t, sig_s,
                                        STSAFEA_XYRS_ECDSA_SHA384_LENGTH);  /* Using 384 max len. */
  STSAFEA_STATIC_ALLOCATE_LV_BUFFER_VAR(StSafeA_LVBuffer_t, pub_x,
                                        STSAFEA_XYRS_ECDSA_SHA384_LENGTH);  /* Using 384 max len. */
  STSAFEA_STATIC_ALLOCATE_LV_BUFFER_VAR(StSafeA_LVBuffer_t, pub_y,
                                        STSAFEA_XYRS_ECDSA_SHA384_LENGTH);  /* Using 384 max len. */

  StSafeA_VerifySignatureBuffer_t verif;
  StSafeA_CurveId_t curve;
  uint32_t ul_data_len;

  /*Check input parameters*/
  if ((pData == NULL_PTR) || (pSignature == NULL_PTR))
  {
    return CKR_ARGUMENTS_BAD;
  }

  /* ========== Check PKCS11 status and session  ========== */

  /* Check if PKCS11 module has already been initialized */
  if (uInitialized == CK_FALSE)
  {
    return CKR_CRYPTOKI_NOT_INITIALIZED;
  }

  /* Check if the Session handle specified in input is valid */
  if (SessionIsValid(hSession) != CKR_OK)
  {
    return CKR_SESSION_HANDLE_INVALID;
  }

  /* Check if Session is open */
  if (SessionIsOpen(hSession) != CKR_OK)
  {
    return CKR_SESSION_CLOSED;
  }

  /* Check if there is VerifyInit was called*/
  if (SessionOperationIsActive(hSession) != CKR_OK)
  {
    return CKR_OPERATION_NOT_INITIALIZED;
  }

  /* ========== Get active operation object ========== */
  if (SessionOperationGetObjects(hSession, &h_object, 1, &ul_object_count) != CKR_OK)
  {
    SessionOperationFinish(hSession);
    return CKR_FUNCTION_FAILED;
  }

  /* Get a pointer to the object using hobject specified in input */
  if (STSAFE_OBJ_GetTemplateFromObjectHandle(&p_template, &ul_count, h_object) != CKR_OK)
  {
    SessionOperationFinish(hSession);
    return CKR_FUNCTION_FAILED;
  }

  /* ========== Check CKA_EC_PARAMS attribute ========== */

  /* Prepare the curve parameters  */
  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_EC_PARAMS, p_template, ul_count) != CKR_OK)
  {
    SessionOperationFinish(hSession);
    return CKR_FUNCTION_FAILED;
  }

  /*Extract curve and data len from key OID */
  if (memcmp(p_attribute_tmp->pValue, STSAFEA_ECC_CURVE_OID_NIST_P_256_DER, p_attribute_tmp->ulValueLen) == 0)
  {
    /* NIST P-256*/
    curve = STSAFEA_NIST_P_256;
    ul_data_len =  STSAFEA_SHA_256_LENGTH;

  }
  else if (memcmp(p_attribute_tmp->pValue, STSAFEA_ECC_CURVE_OID_NIST_P_384_DER, p_attribute_tmp->ulValueLen) == 0)
  {
    /* NIST P-384*/
    curve = STSAFEA_NIST_P_384;
    ul_data_len = STSAFEA_SHA_384_LENGTH;
  }
  else
  {
    /* It should never get here */
    SessionOperationFinish(hSession);
    return CKR_FUNCTION_FAILED;
  }


  /* ========== Check signature len ========== */
  if (ulSignatureLen != (ul_data_len * 2UL))
  {
    SessionOperationFinish(hSession);
    return CKR_SIGNATURE_LEN_RANGE;
  }


  /* ========== Extract CKA_EC_POINT attribute  ========== */


  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_EC_POINT, p_template, ul_count) != CKR_OK)
  {
    SessionOperationFinish(hSession);
    return CKR_FUNCTION_FAILED;
  }

  /* Decode and check data format of embedded object CKE_EC_POINT - if not ok return CKR_DATA_INVALID*/
  /* Note: EC Point formatting for Verify is only DER-encoding of ASN1 X9.62 EC Point value Q*/
  if (DecodeDerASN1UncompressedEcPoint(p_public_data_raw, &ul_public_data_raw_len, p_attribute_tmp->pValue,
                                       p_attribute_tmp->ulValueLen) != CKR_OK)
  {
    SessionOperationFinish(hSession);
    return CKR_DATA_INVALID;
  }

  /* ========== Check data len ========== */
  if ((ulDataLen != ul_data_len) || (ulDataLen != (ul_public_data_raw_len / 2UL)))
  {
    SessionOperationFinish(hSession);
    return CKR_DATA_LEN_RANGE;
  }

  /* ========== Convert Data to STSAFE types : Public Key  ========== */

  /* Prepare Public Key X Coordinate Value */
  pub_x.Length = (uint16_t)(ul_public_data_raw_len / 2UL);
  (void)memcpy(pub_x.Data, &p_public_data_raw[0], pub_x.Length);

  /* Prepare Public Key Y Coordinate Value */
  pub_y.Length = (uint16_t)(ul_public_data_raw_len / 2UL);
  (void)memcpy(pub_y.Data, &p_public_data_raw[pub_x.Length], pub_y.Length);

  /* ========== Convert Data to STSAFE types : Signature  ========== */

  /* Prepare Signature R Value */
  sig_r.Length = (uint16_t)(ulSignatureLen / 2UL);
  (void)memcpy(sig_r.Data, &pSignature[0], sig_r.Length);

  /* Prepare Signature S Value */
  sig_s.Length = (uint16_t)(ulSignatureLen / 2UL);
  (void)memcpy(sig_s.Data, &pSignature[sig_r.Length], sig_s.Length);

  /* ========== Convert Data to STSAFE types : Hash  ========== */

  /* Prepare HASH Value */
  hash.Length = (uint16_t)ul_data_len;
  (void)memcpy(hash.Data, pData, hash.Length);

  /* ========== Verify Signature using STSAFE-A1xx ========== */

  /* Verify with STSAFE-A1xx */
  if (StSafeA_VerifyMessageSignature(&hStSafe, curve, &pub_x, &pub_y, &sig_r, &sig_s, &hash, &verif,
		  STSAFEA_MAC_HOST_RMAC) != STSAFEA_OK)
  {
    SessionOperationFinish(hSession);
    return CKR_FUNCTION_FAILED;
  }

  /* Check Signature validity */
  if (verif.SignatureValidity == 0U)
  {
    SessionOperationFinish(hSession);
    return CKR_SIGNATURE_INVALID;
  }

  /* ========== Mark operation as finished ========== */
  SessionOperationFinish(hSession);

  return CKR_OK;
}

/**
  * @brief  C_VerifyUpdate continues a multiple-part verification
  *         operation, where the signature is an appendix to the data,
  *         and plaintext cannot be recovered from the signature.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession,  the session's handle
  * @param  pPart,     signed data
  * @param  ulPartLen  length of signed data
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_VerifyUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pPart, CK_ULONG ulPartLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_VerifyFinal finishes a multiple-part verification
  *         operation, checking the signature.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession,       the session's handle
  * @param  pSignature,     signature to verify
  * @param  ulSignatureLen  signature length
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_VerifyFinal(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pSignature, CK_ULONG ulSignatureLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief   C_VerifyRecoverInit initializes a signature verification
  * operation, where the data is recovered from the signature.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param hSession,    the session's handle
  * @param pMechanism,  the verification mechanism
  * @param hKey         verification key
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_VerifyRecoverInit(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism,
                                          CK_OBJECT_HANDLE hKey)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_VerifyRecover verifies a signature in a single-part
  * operation, where the data is recovered from the signature.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param hSession,        the session's handle
  * @param pSignature,      signature to verify
  * @param ulSignatureLen,  signature length
  * @param pData,           gets signed data
  * @param pulDataLen       gets signed data len
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_VerifyRecover(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pSignature, CK_ULONG ulSignatureLen,
                                      CK_BYTE_PTR pData, CK_ULONG_PTR pulDataLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_DigestEncryptUpdate continues a multiple-part digesting
  * and encryption operation.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param hSession,            session's handle
  * @param pPart,               the plaintext data
  * @param ulPartLen,           plaintext length
  * @param pEncryptedPart,      gets ciphertext
  * @param pulEncryptedPartLen  gets c-text length
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_DigestEncryptUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pPart, CK_ULONG ulPartLen,
                                            CK_BYTE_PTR pEncryptedPart, CK_ULONG_PTR pulEncryptedPartLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief   C_DecryptDigestUpdate continues a multiple-part decryption and
  * digesting operation.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param hSession,            session's handle
  * @param pEncryptedPart,      ciphertext
  * @param ulEncryptedPartLen,  ciphertext length
  * @param pPart,               gets plaintext
  * @param pulPartLen           gets plaintext len
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_DecryptDigestUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pEncryptedPart,
                                            CK_ULONG ulEncryptedPartLen, CK_BYTE_PTR pPart, CK_ULONG_PTR pulPartLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_SignEncryptUpdate continues a multiple-part signing and
  * encryption operation.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param hSession,            session's handle
  * @param pPart,               the plaintext data
  * @param ulPartLen,           plaintext length
  * @param pEncryptedPart,      gets ciphertext
  * @param pulEncryptedPartLen  gets c-text length
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_SignEncryptUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pPart, CK_ULONG ulPartLen,
                                          CK_BYTE_PTR pEncryptedPart, CK_ULONG_PTR pulEncryptedPartLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief   C_DecryptVerifyUpdate continues a multiple-part decryption and
  * verify operation.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param hSession,            session's handle
  * @param pEncryptedPart,      ciphertext
  * @param ulEncryptedPartLen,  ciphertext length
  * @param pPart,               gets plaintext
  * @param pulPartLen           gets p-text length
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_DecryptVerifyUpdate(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pEncryptedPart,
                                            CK_ULONG ulEncryptedPartLen, CK_BYTE_PTR pPart, CK_ULONG_PTR pulPartLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_GenerateKey generates a secret key, creating a new key
  * object.
  * @param    hSession,    the session's handle
  * @param    pMechanism,  key generation mech.
  * @param    pTemplate,   template for new key
  * @param    ulCount,     # of attrs in template
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_GenerateKey(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_ATTRIBUTE_PTR pTemplate,
                                    CK_ULONG ulCount, CK_OBJECT_HANDLE_PTR phKey)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}


/**
  * @brief  C_GenerateKeyPair generates a public-key/private-key pair,
  *         creating new key objects.
  * @note   This implementation supports only CKM_EC_KEY_PAIR_GEN generation mechanism.
  * @param    hSession,                    session handle
  * @param    pMechanism,                  key-gen mech.
  * @param    pPublicKeyTemplate,          template for pub. key
  * @param    ulPublicKeyAttributeCount,   # pub. attrs.
  * @param    pPrivateKeyTemplate,         template for priv. key
  * @param    ulPrivateKeyAttributeCount,  # priv.  attrs.
  * @retval CK_RV CKR_ARGUMENTS_BAD,CKR_CRYPTOKI_NOT_INITIALIZED, CKR_SESSION_HANDLE_INVALID,CKR_SESSION_CLOSED,
  *               CKR_MECHANISM_INVALID, CKR_ATTRIBUTE_VALUE_INVALID, CKR_ATTRIBUTE_TYPE_INVALID, CKR_FUNCTION_FAILED,
  *               CKR_TEMPLATE_INCONSISTENT, CKR_DEVICE_MEMORY,CKR_OK.
  */
static CK_RV STSAFE_P11_GenerateKeyPair(CK_SESSION_HANDLE    hSession,
                                        CK_MECHANISM_PTR     pMechanism,
                                        CK_ATTRIBUTE_PTR     pPublicKeyTemplate,
                                        CK_ULONG             ulPublicKeyAttributeCount,
                                        CK_ATTRIBUTE_PTR     pPrivateKeyTemplate,
                                        CK_ULONG             ulPrivateKeyAttributeCount,
                                        CK_OBJECT_HANDLE_PTR phPublicKey,
                                        CK_OBJECT_HANDLE_PTR phPrivateKey)
{
  CK_RV e_ret_status;
  CK_OBJECT_HANDLE h_pub_obj, h_prv_obj;
  CK_ULONG ul_obj_cnt, ul_pub_count_embedded, ul_count_filtered, u_len_tmp, ul_prv_count_embedded, \
  ul_public_data_raw_len;
  CK_ATTRIBUTE_PTR  p_pub_template_embedded, p_prv_template_embedded, p_attribute_tmp,p_attribute;
  CK_ATTRIBUTE  p_template_filtered[STSAFE_P11_MAX_TEMPLATE_SIZE];
  CK_BYTE  p_public_data_raw[2UL * STSAFEA_SHA_384_LENGTH];
  StSafeA_CurveId_t curve;
  StSafeA_LVBuffer_t pub_x;
  StSafeA_LVBuffer_t pub_y;
  uint8_t point_reprensentation_id,slot_id,in_authorization_flags = 0;

  /*Check input parameters*/
  if ((pMechanism == NULL_PTR) || (pPublicKeyTemplate == NULL_PTR) || (pPrivateKeyTemplate == NULL_PTR)
      || (phPublicKey == NULL_PTR) || (phPrivateKey == NULL_PTR) || (ulPublicKeyAttributeCount == 0UL)
      || (ulPrivateKeyAttributeCount == 0UL))
  {
    return CKR_ARGUMENTS_BAD;
  }

  /* ========== Check PKCS11 status and session  ========== */

  /* Check if PKCS11 module has already been initialized */
  if (uInitialized == CK_FALSE)
  {
    return CKR_CRYPTOKI_NOT_INITIALIZED;
  }

  /* Check if the Session handle specified in input is valid */
  if (SessionIsValid(hSession) != CKR_OK)
  {
    return CKR_SESSION_HANDLE_INVALID;
  }

  /* Check if Session is open */
  if (SessionIsOpen(hSession) != CKR_OK)
  {
    return CKR_SESSION_CLOSED;
  }

  /* ========== Check mechanism ========== */
  if (pMechanism->mechanism != CKM_EC_KEY_PAIR_GEN)
  {
    return CKR_MECHANISM_INVALID;
  }

  /* ========== Private Key  ========== */
  /* Note: In this implementation we only support key generation on a specific object.
           Each call to GenerateKeyPair will overwrite this object.
           CKA_LABEL is used to refer uniquely to a specific pre-allocated object*/

  /* We need to detect which private key is required for processing so we get CKA_LABEL from provided template */

  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute, CKA_LABEL , pPrivateKeyTemplate, ulPrivateKeyAttributeCount) != CKR_OK)
  {
     return CKR_FUNCTION_FAILED;
  }

  /*Look for the object corresponding to Label, 1 single object*/
  if (FindObjectsFromTemplate(CKV_SESSION_HANDLE_ANY, &h_prv_obj, 1, &ul_obj_cnt, p_attribute, 1) != CKR_OK)
  {
    return CKR_FUNCTION_FAILED;
  }

  /* Get a pointer to the object template*/
  if (STSAFE_OBJ_GetTemplateFromObjectHandle(&p_prv_template_embedded, &ul_prv_count_embedded, h_prv_obj) != CKR_OK)
  {
    return CKR_FUNCTION_FAILED;
  }

  /* ========== Public Key  ========== */
  /* Note: In this implementation we only support key generation on a specific object.
           Each call to GenerateKeyPair will overwrite this object.
           CKA_LABEL is used to refer uniquely to a specific pre-allocated object*/

  /* We need to detect which public key is required for processing so we get CKA_LABEL from provided template */

  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute, CKA_LABEL , pPublicKeyTemplate, ulPublicKeyAttributeCount) != CKR_OK)
  {
     return CKR_FUNCTION_FAILED;
  }

  /*Look for the object corresponding to label, 1 single object*/
  if (FindObjectsFromTemplate(CKV_SESSION_HANDLE_ANY, &h_pub_obj, 1, &ul_obj_cnt, p_attribute, 1) != CKR_OK)
  {
    return CKR_FUNCTION_FAILED;
  }

  /* Get a pointer to the object template */
  if (STSAFE_OBJ_GetTemplateFromObjectHandle(&p_pub_template_embedded, &ul_pub_count_embedded, h_pub_obj) != CKR_OK)
  {
    return CKR_FUNCTION_FAILED;
  }

  /* ========== Check selected Private object against pre-allocated template  ========== */

  /* Note: Since this implementation doesn't support dynamic memory allocation, the template specified in input will
    *       have to match pre-allocated objects. The only value that can be different is CKA_EC_POINT, the actual value
    *       of the public key, that is unique per key.
    *       Filter attributes so that they will not be considered when looking and comparing the template with
    *       pre-allocated objects.
    *       Private Key should not have CKA_EC_POINT attributes because the request is to generate a new key pair.
    */
  CK_ATTRIBUTE attribute_filter[] =  { {CKA_EC_POINT, NULL_PTR, 0}  } ;

  /*Filter template*/
  if (STSAFE_OBJ_FilterTemplate(p_template_filtered, &ul_count_filtered, pPrivateKeyTemplate,
                                ulPrivateKeyAttributeCount,
                                attribute_filter, sizeof(attribute_filter) / sizeof(attribute_filter[0])) != CKR_OK)
  {
    return CKR_FUNCTION_FAILED;
  }

  /* Compare embedded referefence object with input template filtered of 'unique' values (e.g. CKA_EC_POINT).*/
  e_ret_status = STSAFE_OBJ_CompareTemplates(p_template_filtered, ul_count_filtered, p_prv_template_embedded,
                                             ul_prv_count_embedded);

  if (e_ret_status != CKR_OK)
  {
    return e_ret_status;
  }

  /* ========== Check selected Public object against pre-allocated template  ========== */

  /*Filter template*/
  if (STSAFE_OBJ_FilterTemplate(p_template_filtered, &ul_count_filtered, pPublicKeyTemplate, ulPublicKeyAttributeCount,
                                attribute_filter, sizeof(attribute_filter) / sizeof(attribute_filter[0])) != CKR_OK)
  {
    return CKR_FUNCTION_FAILED;
  }

  /* Compare embedded referefence object with input template filtered of 'unique' values (e.g. CKA_EC_POINT).*/
  e_ret_status = STSAFE_OBJ_CompareTemplates(p_template_filtered, ul_count_filtered, p_pub_template_embedded,
                                             ul_pub_count_embedded);

  if (e_ret_status != CKR_OK)
  {
    return e_ret_status;
  }

  /* Note: at this stage both objects are compatible with pre-allocated templates,
     actual key generation process can start*/

  /* ========== Select the right curve using CKA_EC_PARAMS ========== */

  /* Extract from the private key pre-allocated template the CKA_EC_PARAMS attribute*/
  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_EC_PARAMS, p_prv_template_embedded,
                                         ul_prv_count_embedded) != CKR_OK)
  {
    return CKR_FUNCTION_FAILED;
  }


  /* Extract curve and data len from key OID */
  /* Depending on this parameter, a different keypair can be generated - actual type available depends on the
   * configuration profile*/
  if (memcmp(p_attribute_tmp->pValue, STSAFEA_ECC_CURVE_OID_NIST_P_256_DER, p_attribute_tmp->ulValueLen) == 0)
  {
    /* NIST P-256*/
    curve = STSAFEA_NIST_P_256;
  }
  else if (memcmp(p_attribute_tmp->pValue, STSAFEA_ECC_CURVE_OID_NIST_P_384_DER, p_attribute_tmp->ulValueLen) == 0)
  {
    /* NIST P-384*/
    curve = STSAFEA_NIST_P_384;
  }
  else
  {
    /* Should never get here */
    return CKR_FUNCTION_FAILED;
  }

  /* ========== Extract CKA_EXT_TOKEN_PRVKEY_SLOT  ========== */
  /* This vendor defined attribute links one PKCS11 object to a specific feature of STSAFE*/

  /* Extract from the template the external token slot*/
  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_EXT_TOKEN_PRVKEY_SLOT, p_prv_template_embedded,
                                         ul_prv_count_embedded) != CKR_OK)
  {
    return CKR_FUNCTION_FAILED;
  }

  /* ========== Generate Key Pair with STSAFE  ========== */
  /* Note:  Public Key will be returned to the application, while Private Key is stored securely and never
    *       leaves STSAFE. Curve type depends on CKA_EC_PARAM attributes specified in the configuration */

  slot_id=(*((CK_EXT_TOKEN_PRVKEY_SLOT *) (p_attribute_tmp->pValue)));

  if (slot_id == 0xFF)
  {
    in_authorization_flags=(STSAFEA_PRVKEY_MODOPER_AUTHFLAG_KEY_ESTABLISHEN);
  }
  else
  {
    in_authorization_flags=(STSAFEA_PRVKEY_MODOPER_AUTHFLAG_MSG_DGST_SIGNEN);
  }
  
  if (StSafeA_GenerateKeyPair(
                              &hStSafe, /* pStSafeA*/ 
                              slot_id, /* InKeySlotNum */ 
                              0xFFFFU, /* InUseLimit */ 
                              1U, /* InChangeAuthFlagsRight */ 
                              in_authorization_flags, /* InAuthorizationFlags */ 
                              curve, /* InCurveId */ 
                              STSAFEA_GET_XYRS_LEN_FROM_CURVE(curve), /* InPubXYLen */
                              &point_reprensentation_id, /* pOutPointReprensentationId */
                              &pub_x, /* pOutPubX */
                              &pub_y, /* *pOutPubY */ 
                              STSAFEA_MAC_HOST_CMAC /* InMAC */                               
                              ) != STSAFEA_OK)    
    
  {
    return CKR_FUNCTION_FAILED;
  }

  /* We enable private key for all sessions */

  /* Check if object had CKA_VENDOR_OBJECT_AVAILABILITY, then we need to activate the object has available   */
  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_VENDOR_OBJECT_AVAILABILITY, p_prv_template_embedded,
                                         ul_prv_count_embedded) == CKR_OK)
  {
    CK_BBOOL bValue = CK_TRUE;
    e_ret_status = STSAFE_OBJ_SetAttributeValue(p_attribute_tmp, (CK_BYTE*)&bValue, sizeof(CK_BBOOL));
  }

  /* ========== Prepare output data  ========== */

  /* Comput data len  */
  ul_public_data_raw_len = (uint32_t)pub_x.Length;
  ul_public_data_raw_len += (uint32_t) pub_y.Length;

  /* Copy Public Key X coordinate */
  (void)memcpy(&p_public_data_raw[0U], pub_x.Data, pub_x.Length);

  /* Copy Public Key Y coordinate */
  (void)memcpy(&p_public_data_raw[pub_x.Length], pub_y.Data, pub_y.Length);

  /* ========== Update CKA_EC_POINT attribute of the pre-allocated object ========== */

  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_EC_POINT, p_pub_template_embedded,
                                         ul_pub_count_embedded) != CKR_OK)
  {
    return CKR_ATTRIBUTE_TYPE_INVALID;
  }

  /* ========== Check if pre-allocated space is enough to store the new public key  ========== */


  if (ul_public_data_raw_len > p_attribute_tmp->ulValueLen)
  {
    return CKR_DEVICE_MEMORY;
  }

  /* ========== Encode and update CKA_EC_POINT  ========== */

  /*Encode raw data into DER x962 EC Point format expected by PKCS11 spec*/
  /*Note: len value is local only - can't overwrite the ulValueLen constant value in flash for this object*/
  if (EncodeRawToDerASN1UncompressedEcPoint(p_attribute_tmp->pValue, &u_len_tmp, p_public_data_raw,
                                            ul_public_data_raw_len) != CKR_OK)
  {
    return CKR_ATTRIBUTE_VALUE_INVALID;
  }

  /* ========== Update attribute : Pub key can be enabled for all sessions  ========== */

  ObjectSetVendorObjectAvailability(h_pub_obj, CK_TRUE);
  ObjectSetVendorObjectAvailability(h_prv_obj, CK_TRUE);

  /* ========== Return object handles ========== */

  /*return object handles*/
  *phPublicKey = h_pub_obj;
  *phPrivateKey = h_prv_obj;

  return CKR_OK;
}

/**
  * @brief  C_WrapKey wraps (i.e., encrypts) a key.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param hSession,        the session's handle
  * @param pMechanism,      the wrapping mechanism
  * @param hWrappingKey,    wrapping key
  * @param hKey,            key to be wrapped
  * @param pWrappedKey,     gets wrapped key
  * @param pulWrappedKeyLen gets wrapped key size
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_WrapKey(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism, CK_OBJECT_HANDLE hWrappingKey,
                                CK_OBJECT_HANDLE hKey, CK_BYTE_PTR pWrappedKey, CK_ULONG_PTR pulWrappedKeyLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}

/**
  * @brief   C_UnwrapKey unwraps (decrypts) a wrapped key, creating a new key object.
  * @param    hSession          session's handle
  * @param    pMechanism        unwrapping mech.
  * @param    hUnwrappingKey    unwrapping key
  * @param    pWrappedKey       the wrapped key
  * @param    ulWrappedKeyLen   wrapped key len
  * @param    pTemplate         new key template
  * @param    ulAttributeCount  template length
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_UnwrapKey(CK_SESSION_HANDLE hSession, CK_MECHANISM_PTR pMechanism,
                                  CK_OBJECT_HANDLE hUnwrappingKey, CK_BYTE_PTR pWrappedKey, CK_ULONG ulWrappedKeyLen,
                                  CK_ATTRIBUTE_PTR pTemplate,
                                  CK_ULONG ulAttributeCount, CK_OBJECT_HANDLE_PTR phKey)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}


/**
  * @brief  C_DeriveKey derives a key from a base key, creating a new key object.
  * @note   This implementation only supports ECDH1 with CKD_NULL parameter derivation.
  *         The key derivation function CKD_NULL produces a raw shared secret value.
  *         Diffie-Hellman derivation will be computed using the ephemeral private key stored in
  *         STSAFE and generated with C_GenerateKeyPair in combination with a public key provided
  *         in input through pMechanism.
  * @param  hSession          session's handle
  * @param  pMechanism        key deriv. mech.
  * @param  hBaseKey          base key
  * @param  pTemplate         new key template
  * @param  ulAttributeCount  template length
  * @param  phKey             gets derived key object
  * @retval CK_RV CKR_ARGUMENTS_BAD,CKR_CRYPTOKI_NOT_INITIALIZED, CKR_SESSION_HANDLE_INVALID,CKR_SESSION_CLOSED,
  *               CKR_MECHANISM_INVALID, CKR_MECHANISM_PARAM_INVALID, CKR_DOMAIN_PARAMS_INVALID,
  *               CKR_ATTRIBUTE_VALUE_INVALID,CKR_ATTRIBUTE_TYPE_INVALID, CKR_TEMPLATE_INCONSISTENT,
  *               CKR_ATTRIBUTE_READ_ONLY, CKR_FUNCTION_FAILED, CKR_DEVICE_MEMORY, CKR_OK.
  */
static CK_RV STSAFE_P11_DeriveKey(CK_SESSION_HANDLE  hSession, CK_MECHANISM_PTR  pMechanism,
                                  CK_OBJECT_HANDLE   hBaseKey, CK_ATTRIBUTE_PTR     pTemplate,
                                  CK_ULONG   ulAttributeCount, CK_OBJECT_HANDLE_PTR phKey)
{
  CK_RV e_ret_status;
  CK_ECDH1_DERIVE_PARAMS_PTR p_params;
  CK_OBJECT_HANDLE h_obj_shrd_secret_x;
  CK_ATTRIBUTE  template_tmp, p_template_filtered[STSAFE_P11_MAX_TEMPLATE_SIZE];
  CK_ATTRIBUTE_PTR p_attribute_tmp, p_template_base, p_template_secret;
  CK_ULONG  ul_count_base, ul_count_secret, ul_public_data_raw_len, prv_key_len, ul_obj_cnt, ul_count_filtered;
  CK_BYTE  p_public_data_raw[2UL * STSAFEA_SHA_384_LENGTH];

  STSAFEA_STATIC_ALLOCATE_LV_BUFFER_VAR(StSafeA_LVBuffer_t, pub_x,
                                        STSAFEA_XYRS_ECDSA_SHA384_LENGTH);  /* Using 384 max len. */
  STSAFEA_STATIC_ALLOCATE_LV_BUFFER_VAR(StSafeA_LVBuffer_t, pub_y,
                                        STSAFEA_XYRS_ECDSA_SHA384_LENGTH);  /* Using 384 max len. */

  StSafeA_SharedSecretBuffer_t shared_secret;
  uint8_t SharedKey_Data[STSAFEA_XYRS_ECDSA_SHA384_LENGTH];
  shared_secret.SharedKey.Data   = SharedKey_Data;

  /*Check input parameters*/
  if ((pMechanism == NULL_PTR) || (pTemplate == NULL_PTR) || (phKey == NULL_PTR) || (ulAttributeCount == 0UL))
  {
    return CKR_ARGUMENTS_BAD;
  }

  /* ========== Check PKCS11 status and session  ========== */

  /* Check if PKCS11 module has already been initialized */
  if (uInitialized == CK_FALSE)
  {
    return CKR_CRYPTOKI_NOT_INITIALIZED;
  }

  /* Check if the Session handle specified in input is valid */
  if (SessionIsValid(hSession) != CKR_OK)
  {
    return CKR_SESSION_HANDLE_INVALID;
  }

  /* Check if Session is open */
  if (SessionIsOpen(hSession) != CKR_OK)
  {
    return CKR_SESSION_CLOSED;
  }

  /* ========== Check Mechanism Params specified in input ========== */
  if (pMechanism->pParameter == NULL_PTR)
  {
    return CKR_MECHANISM_INVALID;
  }

  /* Assign mechanism params */
  p_params = (CK_ECDH1_DERIVE_PARAMS_PTR)pMechanism->pParameter;

  /* Check KDF parameter*/
  /* Note: In this implementation only CKD_NULL (KDF = NULL_PTR) is supported */
  if (p_params->kdf != CKD_NULL)
  {
    return CKR_MECHANISM_PARAM_INVALID;
  }

  /* Check if a public key was specified in input */
  /* With the key derivation function CKD_NULL, pSharedData must be NULL and ulSharedDataLen must be zero*/
  if ((p_params->pPublicData == NULL_PTR) || (p_params->pSharedData != NULL_PTR) || (p_params->ulSharedDataLen != 0UL))
  {
    return CKR_DOMAIN_PARAMS_INVALID;
  }

  /* ========== Parse of the other party's EC public key value  ========== */

  /* A token MAY support accepting this value as a DER-encoded ECPoint (as per section E.6 of [ANSI X9.62]) i.e. the
   * same as a CKA_EC_POINT encoding. */
  if (DecodeDerASN1UncompressedEcPoint(p_public_data_raw, &ul_public_data_raw_len, p_params->pPublicData,
                                       p_params->ulPublicDataLen) != CKR_OK)
  {
    /*A token MUST be able to accept this value encoded as a raw octet string (as per section A.5.2 of [ANSI X9.62]).*/
    if (DecodeDerASN1OctetString(p_public_data_raw, &ul_public_data_raw_len, p_params->pPublicData,
                                 p_params->ulPublicDataLen) != CKR_OK)
    {
      return CKR_DOMAIN_PARAMS_INVALID;
    }
  }


  /* ========== Parse Base Key ========== */
  /* Note: In this implementation, base key is the private key used for Diffie-Hellman shared secret computation*/

  /* Get a pointer to the object template using hBaseKey specified in input */
  if (STSAFE_OBJ_GetTemplateFromObjectHandle(&p_template_base, &ul_count_base, hBaseKey) != CKR_OK)
  {
    return CKR_KEY_HANDLE_INVALID;
  }

  /* ========== Extract CKA_ALLOWED_MECHANISMS attribute  ========== */

  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_ALLOWED_MECHANISMS, p_template_base,
                                         ul_count_base) != CKR_OK)
  {
    return CKR_KEY_HANDLE_INVALID;
  }

  /* Check if the mechanism specified in input is valid for the Base key  */
  if (IsMechanismTypeAllowed(pMechanism->mechanism, p_attribute_tmp->pValue, p_attribute_tmp->ulValueLen) != CK_TRUE)
  {
    return CKR_MECHANISM_INVALID;
  }

  /* ========== Check CKA_EC_PARAMS attribute ========== */

  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_EC_PARAMS, p_template_base, ul_count_base) != CKR_OK)
  {
    return CKR_FUNCTION_FAILED;
  }

  /*Extract curve and data len from key OID */
  if (memcmp(p_attribute_tmp->pValue, STSAFEA_ECC_CURVE_OID_NIST_P_256_DER, p_attribute_tmp->ulValueLen) == 0)
  {
    /* NIST P-256*/
    prv_key_len = STSAFEA_SHA_256_LENGTH * 2UL;

  }
  else if (memcmp(p_attribute_tmp->pValue, STSAFEA_ECC_CURVE_OID_NIST_P_384_DER, p_attribute_tmp->ulValueLen) == 0)
  {
    /* NIST P-384*/
    prv_key_len = STSAFEA_SHA_384_LENGTH * 2UL;

  }
  else
  {
    /* It should never get here */
    return CKR_KEY_HANDLE_INVALID;
  }

  /* ========== Check Key len consistency ========== */
  if (prv_key_len != ul_public_data_raw_len)
  {
    return CKR_DOMAIN_PARAMS_INVALID;
  }


  /* ========== Secret Key  ========== */
  /* Note: In this implementation we only support key derivation of a shared secret on a specific pre-allocated object.
    *      Each call to DeriveKey will overwrite this object.
    */

  template_tmp.type = CKA_LABEL;
  template_tmp.pValue = P11_OBJ_LABEL_DH_SECRET_X;

  /*Look for the object corresponding to P11_OBJ_LABEL_DH_SECRET_X, 1 single object*/
  if (FindObjectsFromTemplate(CKV_SESSION_HANDLE_ANY, &h_obj_shrd_secret_x, 1, &ul_obj_cnt, &template_tmp, 1) != CKR_OK)
  {
    return CKR_ATTRIBUTE_VALUE_INVALID;
  }

  /* Get a pointer to the object  */
  if (STSAFE_OBJ_GetTemplateFromObjectHandle(&p_template_secret, &ul_count_secret, h_obj_shrd_secret_x) != CKR_OK)
  {
    return CKR_ATTRIBUTE_VALUE_INVALID;
  }

  /* ========== Check selected object against reference template  ========== */
  /* Note: Since this implementation doesn't support dynamic memory allocation, the template specified in input will
    *       have to match pre-allocated objects. The only value that can be different is CKA_VALUE, the actual value
    *       of the shared secret key, that is unique per key.
    *       Filter attributes so that they will not be considered when looking and comparing the template with
    *       pre-allocated objects.
    */
  CK_ATTRIBUTE attribute_filter[] =  { {CKA_VALUE, NULL_PTR, 0UL}  } ;

  /*Filter template*/
  if (STSAFE_OBJ_FilterTemplate(p_template_filtered, &ul_count_filtered, pTemplate, ulAttributeCount, attribute_filter,
                                sizeof(attribute_filter) / sizeof(attribute_filter[0])) != CKR_OK)
  {
    return CKR_FUNCTION_FAILED;
  }

  /* Compare embedded referefence object with input template filtered of 'unique' values (e.g. CKA_VALUE).*/
  e_ret_status = STSAFE_OBJ_CompareTemplates(p_template_filtered, ul_count_filtered, p_template_secret,
                                             ul_count_secret);

  if (e_ret_status != CKR_OK)
  {
    return e_ret_status;
  }

  /* ========== Convert Data to STSAFE types : Other Party's Public Key ========== */

  /* Extract X coordinate from public key specified as mechanism param*/
  pub_x.Length = (uint16_t)(ul_public_data_raw_len / 2UL);
  (void)memcpy(pub_x.Data, p_public_data_raw, pub_x.Length);

  /* Extract Y coordinate from public key specified as mechanism param*/
  pub_y.Length = (uint16_t)(ul_public_data_raw_len / 2UL);
  (void)memcpy(pub_y.Data, &p_public_data_raw[pub_x.Length], pub_y.Length);

  /* ========== Check CKA_EXT_TOKEN_PRVKEY_SLOT attribute========== */
  /* This vendor defined attribute links one PKCS11 object to a specific feature of STSAFE*/

  /* Extract from the template the external token slot*/
  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_EXT_TOKEN_PRVKEY_SLOT, p_template_base,
                                         ul_count_base) != CKR_OK)
  {
    return CKR_FUNCTION_FAILED;
  }

  /* ========== Compute DH shared secret X coordinate  ========== */

  /* STSAFE Establish Key */
#ifdef STSAFE_A100
  if (StSafeA_EstablishKey(&hStSafe, __STSAFEA_GET_SLOT_ID_FROM_CK(p_attribute_tmp->pValue), &pub_x, &pub_y,
                           STSAFEA_XYRS_ECDSA_SHA256_LENGTH, &shared_secret, STSAFEA_MAC_HOST_CMAC,
                           STSAFEA_ENCRYPTION_RESPONSE) != STSAFEA_OK)
#else
  if (StSafeA_EstablishKey(&hStSafe, __STSAFEA_GET_SLOT_ID_FROM_CK(p_attribute_tmp->pValue), &pub_x, &pub_y,
                           STSAFEA_XYRS_ECDSA_SHA256_LENGTH, &shared_secret, STSAFEA_MAC_HOST_CMAC,
                           STSAFEA_ENCRYPTION_NONE) != STSAFEA_OK)
#endif
  {
    return CKR_FUNCTION_FAILED;
  }

  /* ========== Update CKA_VALUE attribute of the embedded object ========== */

  /* Extract CKA_VALUE attribute from the template of the embedded object*/
  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_VALUE, p_template_secret, ul_count_secret) != CKR_OK)
  {
    return CKR_FUNCTION_FAILED;
  }

  /* Check if pre-allocated space is enough to store the new public key */
  if (shared_secret.SharedKey.Length > p_attribute_tmp->ulValueLen)
  {
    return CKR_DEVICE_MEMORY;
  }

  /* Copy STSAFE generated SharedKey coordinate to local variable */
  if (STSAFE_OBJ_SetAttributeValue(p_attribute_tmp, (CK_BYTE_PTR)shared_secret.SharedKey.Data,
                                   shared_secret.SharedKey.Length) != CKR_OK)
  {
    return CKR_FUNCTION_FAILED;
  }

  /* ========== Assign the object to the session ========== */
  if (ObjectSetSession(h_obj_shrd_secret_x, hSession) != CKR_OK)
  {
    return CKR_FUNCTION_FAILED;
  }

  /* ========== Return object value ========== */

  /* Return object id */
  *phKey = h_obj_shrd_secret_x;

  return CKR_OK;
}

/**
  * @brief  C_SeedRandom mixes additional seed material into the token's
  * random number generator
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param hSession,  the session's handle
  * @param pSeed,     the seed material
  * @param ulSeedLen  length of seed material
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_SeedRandom(CK_SESSION_HANDLE hSession, CK_BYTE_PTR pSeed, CK_ULONG ulSeedLen)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}

/**
  * @brief  C_GenerateRandom generates random data.
  * @note   In this implementation, STSAFE_RANDOM_MAX_LEN max size is allowed.
  * @param hSession    the session's handle
  * @param RandomData  receives the random data
  * @param ulRandomLen  # of bytes to generate
  * @retval CK_RV CKR_ARGUMENTS_BAD,CKR_CRYPTOKI_NOT_INITIALIZED, CKR_SESSION_HANDLE_INVALID,CKR_SESSION_CLOSED,
  *         CKR_FUNCTION_FAILED, CKR_OK.
  */
static CK_RV STSAFE_P11_GenerateRandom(CK_SESSION_HANDLE hSession, CK_BYTE_PTR  pRandomData,
                                       CK_ULONG          ulRandomLen)
{
  CK_RV e_ret_status;

  StSafeA_LVBuffer_t out_generate_random;

  /*Check input parameters*/
  if ((pRandomData == NULL_PTR) || (ulRandomLen == 0UL) || (ulRandomLen > STSAFE_RANDOM_MAX_LEN))
  {
    return CKR_ARGUMENTS_BAD;
  }

  /* ========== Check PKCS11 status and session  ========== */

  /* Check if PKCS11 module has already been initialized */
  if (uInitialized == CK_FALSE)
  {
    return CKR_CRYPTOKI_NOT_INITIALIZED;
  }

  /* Check if the Session handle specified in input is valid */
  if (SessionIsValid(hSession) != CKR_OK)
  {
    return CKR_SESSION_HANDLE_INVALID;
  }

  /* Check if Session is open */
  if (SessionIsOpen(hSession) != CKR_OK)
  {
    return CKR_SESSION_CLOSED;
  }

  /* ========== Generate Random Number with STSAFE ========== */
  if ((StSafeA_GenerateRandom(&hStSafe, STSAFEA_EPHEMERAL_RND, (uint8_t)ulRandomLen, &out_generate_random,
                              STSAFEA_MAC_HOST_RMAC) != STSAFEA_OK) || (out_generate_random.Length != ulRandomLen))
  {
    e_ret_status = CKR_FUNCTION_FAILED;
  }
  else
  {
    /*(void)memcpy(pRandomData, out_generate_random.Data, out_generate_random.Length);*/
    
     for (int i = 0, j = (rand() & (out_generate_random.Length-1)); i < out_generate_random.Length; i++, j = ((j+1) 
                                                                           & (out_generate_random.Length-1)))
     {
       pRandomData[j] = out_generate_random.Data[j];
     }
    
     (void)memset(out_generate_random.Data, 0, out_generate_random.Length);  
    
     e_ret_status = CKR_OK;
  }

  return e_ret_status;
}

/**
  * @brief  C_GetFunctionStatus is a legacy function; it obtains an
  *         updated status of a function running in parallel with an application.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  hSession  the session's handle
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_GetFunctionStatus(CK_SESSION_HANDLE hSession)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief C_CancelFunction is a legacy function; it cancels a function
  *        running in parallel
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param hSession  the session's handle
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_CancelFunction(CK_SESSION_HANDLE hSession)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}
/**
  * @brief  C_WaitForSlotEvent waits for a slot event (token insertion, removal, etc.) to occur.
  * @note   NOT SUPPORTED IN THIS IMPLEMENTATION.
  * @param  flags,        blocking/nonblocking flag
  * @param  pSlot,  location that receives the slot ID
  * @param  pRserved   reserved.  Should be NULL_PTR
  * @retval CK_RV CKR_FUNCTION_NOT_SUPPORTED.
  */
static CK_RV STSAFE_P11_WaitForSlotEvent(CK_FLAGS flags, CK_SLOT_ID_PTR pSlot, CK_VOID_PTR pRserved)
{
  return CKR_FUNCTION_NOT_SUPPORTED;
}


/**
  * @brief KMS GetFunctionList function.
  *   This function return the function list supported by the api.
  * @param ppFunctionList point to structure listing the functions.
  * @retval CK_RV CKR_OK if successful, see PKCS11 return values for more information.
  */
CK_RV KMS_EXT_TOKEN_GetFunctionList(CK_FUNCTION_LIST_PTR_PTR ppFunctionList)
{
  CK_RV ck_rv_ret_status = CKR_OK;
  CK_FUNCTION_LIST_PTR pFunctionList = *ppFunctionList;

  pFunctionList->version.major = 2;
  pFunctionList->version.minor = 40;

  pFunctionList->C_Initialize = STSAFE_P11_Initialize;
  pFunctionList->C_Finalize = STSAFE_P11_Finalize;
  pFunctionList->C_GetInfo  = STSAFE_P11_GetInfo;
  pFunctionList->C_GetFunctionList = KMS_EXT_TOKEN_GetFunctionList;
  pFunctionList->C_GetSlotList = STSAFE_P11_GetSlotList;
  pFunctionList->C_GetSlotInfo = STSAFE_P11_GetSlotInfo;
  pFunctionList->C_GetTokenInfo = STSAFE_P11_GetTokenInfo;
  pFunctionList->C_GetMechanismList  = STSAFE_P11_GetMechanismList  ;
  pFunctionList->C_GetMechanismInfo  = STSAFE_P11_GetMechanismInfo;
  pFunctionList->C_InitToken  = STSAFE_P11_InitToken;
  pFunctionList->C_InitPIN  = STSAFE_P11_InitPIN;
  pFunctionList->C_SetPIN  = STSAFE_P11_SetPIN;
  pFunctionList->C_OpenSession = STSAFE_P11_OpenSession;
  pFunctionList->C_CloseSession = STSAFE_P11_CloseSession;
  pFunctionList->C_CloseAllSessions  = STSAFE_P11_CloseAllSessions;
  pFunctionList->C_GetSessionInfo  = STSAFE_P11_GetSessionInfo;
  pFunctionList->C_GetOperationState  = STSAFE_P11_GetOperationState;
  pFunctionList->C_SetOperationState  = STSAFE_P11_SetOperationState;
  pFunctionList->C_Login  = STSAFE_P11_Login;
  pFunctionList->C_Logout  = STSAFE_P11_Logout;
  pFunctionList->C_CreateObject = STSAFE_P11_CreateObject;
  pFunctionList->C_CopyObject  = STSAFE_P11_CopyObject;
  pFunctionList->C_DestroyObject  = STSAFE_P11_DestroyObject;
  pFunctionList->C_GetObjectSize  = STSAFE_P11_GetObjectSize;
  pFunctionList->C_GetAttributeValue = STSAFE_P11_GetAttributeValue;
  pFunctionList->C_SetAttributeValue = STSAFE_P11_SetAttributeValue;
  pFunctionList->C_FindObjectsInit = STSAFE_P11_FindObjectsInit;
  pFunctionList->C_FindObjects = STSAFE_P11_FindObjects;
  pFunctionList->C_FindObjectsFinal = STSAFE_P11_FindObjectsFinal;
  pFunctionList->C_EncryptInit  = STSAFE_P11_EncryptInit;
  pFunctionList->C_Encrypt  = STSAFE_P11_Encrypt;
  pFunctionList->C_EncryptUpdate  = STSAFE_P11_EncryptUpdate;
  pFunctionList->C_EncryptFinal  = STSAFE_P11_EncryptFinal;
  pFunctionList->C_DecryptInit  = STSAFE_P11_DecryptInit;
  pFunctionList->C_Decrypt  = STSAFE_P11_Decrypt;
  pFunctionList->C_DecryptUpdate  = STSAFE_P11_DecryptUpdate;
  pFunctionList->C_DecryptFinal  = STSAFE_P11_DecryptFinal;
  pFunctionList->C_DigestInit  = STSAFE_P11_DigestInit;
  pFunctionList->C_Digest  = STSAFE_P11_Digest;
  pFunctionList->C_DigestUpdate  = STSAFE_P11_DigestUpdate;
  pFunctionList->C_DigestKey  = STSAFE_P11_DigestKey;
  pFunctionList->C_DigestFinal  = STSAFE_P11_DigestFinal;
  pFunctionList->C_SignInit = STSAFE_P11_SignInit;
  pFunctionList->C_Sign = STSAFE_P11_Sign;
  pFunctionList->C_SignUpdate  = STSAFE_P11_SignUpdate;
  pFunctionList->C_SignFinal  = STSAFE_P11_SignFinal;
  pFunctionList->C_SignRecoverInit  = STSAFE_P11_SignRecoverInit;
  pFunctionList->C_SignRecover  = STSAFE_P11_SignRecover;
  pFunctionList->C_VerifyInit = STSAFE_P11_VerifyInit;
  pFunctionList->C_Verify = STSAFE_P11_Verify;
  pFunctionList->C_VerifyUpdate  = STSAFE_P11_VerifyUpdate;
  pFunctionList->C_VerifyFinal  = STSAFE_P11_VerifyFinal;
  pFunctionList->C_VerifyRecoverInit  = STSAFE_P11_VerifyRecoverInit;
  pFunctionList->C_VerifyRecover  = STSAFE_P11_VerifyRecover;
  pFunctionList->C_DigestEncryptUpdate  = STSAFE_P11_DigestEncryptUpdate;
  pFunctionList->C_DecryptDigestUpdate  = STSAFE_P11_DecryptDigestUpdate;
  pFunctionList->C_SignEncryptUpdate  = STSAFE_P11_SignEncryptUpdate;
  pFunctionList->C_DecryptVerifyUpdate  = STSAFE_P11_DecryptVerifyUpdate;
  pFunctionList->C_GenerateKey  = STSAFE_P11_GenerateKey;
  pFunctionList->C_GenerateKeyPair = STSAFE_P11_GenerateKeyPair;
  pFunctionList->C_WrapKey  = STSAFE_P11_WrapKey;
  pFunctionList->C_UnwrapKey  = STSAFE_P11_UnwrapKey;
  pFunctionList->C_DeriveKey = STSAFE_P11_DeriveKey;
  pFunctionList->C_SeedRandom  = STSAFE_P11_SeedRandom;
  pFunctionList->C_GenerateRandom = STSAFE_P11_GenerateRandom;
  pFunctionList->C_GetFunctionStatus  = STSAFE_P11_GetFunctionStatus;
  pFunctionList->C_CancelFunction  = STSAFE_P11_CancelFunction;
  pFunctionList->C_WaitForSlotEvent  = STSAFE_P11_WaitForSlotEvent;

  return ck_rv_ret_status;
}

/**
  * @brief  SessionInit initialize all the sessions.
  * @retval CK_RV CKR_OK.
  */
static CK_RV    SessionInit(void)
{
  uint32_t i;

  ulSessionNb = 0;

  /* Initialize the Session List */
  for (i = 0 ; i < STSAFE_P11_NB_SESSIONS_MAX; i++)
  {
    a_SessionList[i].SessionHandle = CK_INVALID_HANDLE;
  }

  return CKR_OK;
}


/**
  * @brief  Open a session.
  * @param  slotID     the slot's ID
  * @param  flags     from CK_SESSION_INFO
  * @param  pApplication     passed to callback
  * @param  Notify     callback function
  * @param  phSession     gets session handle
  * @retval CK_RV CKR_SESSION_COUNT, CKR_OK.
  */
static CK_RV    SessionOpen(CK_SLOT_ID slotID, CK_FLAGS flags, CK_VOID_PTR pApplication,  CK_NOTIFY Notif,
                            CK_SESSION_HANDLE_PTR phSession)
{

  CK_ULONG ul_session_index;

  /* Prevent unused argument(s) compilation warning */
  UNUSED(pApplication);
  UNUSED(Notif);

  /* Check if the max number of sessions open at the same time was reached.  */
  if (ulSessionNb >=  STSAFE_P11_NB_SESSIONS_MAX)
  {
    return CKR_SESSION_COUNT;
  }

  /* Find the next available slot for a Session */
  ul_session_index = 0;

  do
  {
    if (a_SessionList[ul_session_index].SessionHandle == CK_INVALID_HANDLE)
    {
      break;
    }
    ul_session_index++;
  } while (ul_session_index < STSAFE_P11_NB_SESSIONS_MAX);

  /* Slot is found */
  if ((ul_session_index < STSAFE_P11_NB_SESSIONS_MAX) &&
      (a_SessionList[ul_session_index].SessionHandle == CK_INVALID_HANDLE))
  {
    a_SessionList[ul_session_index].SessionInfo.slotID = slotID;
    a_SessionList[ul_session_index].SessionHandle = ul_session_index + STSAFE_P11_SESSION_INDEX_OFFSET;
    /* A session can have only one crypto mechanism on going at a time. */
    a_SessionList[ul_session_index].OperationActive  = CK_FALSE;
    /* Reset all operations data */
    (void)memset(a_SessionList[ul_session_index].OperationObjects, CK_INVALID_HANDLE,
                 sizeof(CK_OBJECT_HANDLE) * sizeof(a_SessionList[ul_session_index].OperationObjects) /
                 (sizeof(a_SessionList[ul_session_index].OperationObjects[0U])));
    a_SessionList[ul_session_index].ulOperationObjectsCount = 0UL;

    /* Increment the session counter */
    ulSessionNb++;

    /*Prepare Session Handle*/
    *phSession = a_SessionList[ul_session_index].SessionHandle;

    return CKR_OK;
  }
  else
  {
    return CKR_SESSION_COUNT;
  }
}

/**
  * @brief  Closes a session.
  * @param  hSession     the session handle
  * @retval CK_RV CKR_OK
  */

static CK_RV    SessionClose(CK_SESSION_HANDLE hSession)
{

  CK_ULONG ul_session_index;

  /*Adapt Session handle to session index*/
  ul_session_index = hSession - STSAFE_P11_SESSION_INDEX_OFFSET;

  /* Close the Session */
  a_SessionList[ul_session_index].SessionHandle = CK_INVALID_HANDLE;
  (void)memset(&(a_SessionList[ul_session_index].SessionInfo), 0U, sizeof(CK_SESSION_INFO));
  a_SessionList[ul_session_index].OperationActive = CK_FALSE;
  /* Reset all operations data */
  (void)memset(a_SessionList[ul_session_index].OperationObjects, CK_INVALID_HANDLE,
               sizeof(CK_OBJECT_HANDLE) * sizeof(a_SessionList[ul_session_index].OperationObjects) /
               (sizeof(a_SessionList[ul_session_index].OperationObjects[0U])));
  a_SessionList[ul_session_index].ulOperationObjectsCount = 0UL;

  /* Decrement the session counter */
  ulSessionNb--;

  return CKR_OK;

}

/**
  * @brief  Checks if the session is valid.
  * @param  hSession     the session handle
  * @retval CK_RV CKR_SESSION_HANDLE_INVALID, CKR_OK.
  */
static CK_RV    SessionIsValid(CK_SESSION_HANDLE hSession)
{
  CK_ULONG ul_session_index;

  /*Check input parameters*/
  if ((hSession == CK_INVALID_HANDLE) || (hSession < STSAFE_P11_SESSION_INDEX_OFFSET))
  {
    return CKR_SESSION_HANDLE_INVALID;
  }

  /*Adapt Session handle to session index*/
  ul_session_index = hSession - STSAFE_P11_SESSION_INDEX_OFFSET;

  /* Check if session handle is valid*/
  if ((ul_session_index > ulSessionNb) ||
      (ul_session_index > STSAFE_P11_NB_SESSIONS_MAX))
  {
    return   CKR_SESSION_HANDLE_INVALID;
  }

  return   CKR_OK;
}

/**
  * @brief  Checks if the session is open.
  * @param  hSession     the session handle
  * @retval CK_RV CKR_SESSION_HANDLE_INVALID, CKR_GENERAL_ERROR, CKR_OK.
  */
static CK_RV    SessionIsOpen(CK_SESSION_HANDLE hSession)
{
  CK_ULONG ul_session_index;

  /*Check input parameters*/
  if (SessionIsValid(hSession) != CKR_OK)
  {
    return CKR_SESSION_HANDLE_INVALID;
  }

  /*Adapt Session handle to session index*/
  ul_session_index = hSession - STSAFE_P11_SESSION_INDEX_OFFSET;

  /* Check if session handle is valid and open*/
  if ((a_SessionList[ul_session_index].SessionHandle == CK_INVALID_HANDLE)
      || (a_SessionList[ul_session_index].SessionHandle != hSession))
  {
    return   CKR_GENERAL_ERROR;
  }

  return   CKR_OK;

}

/**
  * @brief  Checks if the session has an active operation.
  * @param  hSession     the session handle
  * @retval CK_RV CKR_SESSION_HANDLE_INVALID, CKR_GENERAL_ERROR, CKR_OK.
  */
static CK_RV    SessionOperationIsActive(CK_SESSION_HANDLE hSession)
{
  CK_ULONG ul_session_index;

  /*Check input parameters*/
  if (SessionIsValid(hSession) != CKR_OK)
  {
    return CKR_SESSION_HANDLE_INVALID;
  }

  /*Adapt Session handle to session index*/
  ul_session_index = hSession - STSAFE_P11_SESSION_INDEX_OFFSET;

  /*Check if operation is tagged as active*/
  if (a_SessionList[ul_session_index].OperationActive == CK_FALSE)
  {
    return CKR_GENERAL_ERROR;
  }

  return CKR_OK;
}


/**
  * @brief  Initialize an operation for a session.
  * @param  hSession     the session handle
  * @param  pTemplate     the operation template
  * @param  ulCount     attributes in template
  * @param  Mechanism     the operation mechanism
  * @retval CK_RV CKR_ARGUMENTS_BAD, CKR_SESSION_HANDLE_INVALID, CKR_OK.
  */
static CK_RV    SessionOperationSetObjects(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE_PTR pObject, CK_ULONG ulCount)
{
  CK_ULONG ul_session_index;

  /*Check input parameters*/
  if (SessionIsValid(hSession) != CKR_OK)
  {
    return CKR_SESSION_HANDLE_INVALID;
  }

  /*Adapt Session handle to session index*/
  ul_session_index = hSession - STSAFE_P11_SESSION_INDEX_OFFSET;

  /* Init operations data */
  if (pObject != NULL_PTR)
  {
    (void)memcpy(a_SessionList[ul_session_index].OperationObjects, pObject, ulCount * sizeof(CK_OBJECT_HANDLE));

    a_SessionList[ul_session_index].ulOperationObjectsCount = ulCount;
  }

  /* Mark operation as active */
  a_SessionList[ul_session_index].OperationActive = CK_TRUE;

  return CKR_OK;
}

/**
  * @brief  Get params of active operation.
  * @param  hSession     the session handle
  * @param  pTemplate     gets the operation template
  * @param  pulCount      gets the number of attributes in the template
  * @param  Mechanism     gets the operation mechanism
  * @retval CK_RV CKR_ARGUMENTS_BAD, CKR_OK.
  */
static CK_RV    SessionOperationGetObjects(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE_PTR pObject,
                                           CK_ULONG ulMaxObjectCount, CK_ULONG_PTR pulObjectCount)
{
  CK_ULONG ul_session_index;
  uint32_t i;

  /*Check input parameters*/
  if (pulObjectCount == NULL)
  {
    return CKR_ARGUMENTS_BAD;
  }

  /*Check input parameters*/
  if (SessionIsValid(hSession) != CKR_OK)
  {
    return CKR_SESSION_HANDLE_INVALID;
  }

  /*Adapt Session handle to session index*/
  ul_session_index = hSession - STSAFE_P11_SESSION_INDEX_OFFSET;

  /*Go through the list of stored objects*/
  i = 0U;

  while ((i < a_SessionList[ul_session_index].ulOperationObjectsCount) && (i < ulMaxObjectCount))
  {
    /* Retrieve stored object handles */
    if (pObject != NULL_PTR)
    {
      pObject[i] = a_SessionList[ul_session_index].OperationObjects[i];
    }

    i++;
  }

  /* Assingn the number of objects*/
  *pulObjectCount = i;


  return CKR_OK;

}

/**
  * @brief  Finalize an operation for a session.
  * @param  hSession     the session handle
  * @retval CK_RV CKR_OK.
  */
static CK_RV    SessionOperationFinish(CK_SESSION_HANDLE hSession)
{
  CK_ULONG ul_session_index;

  /*Check input parameters*/
  if (SessionIsValid(hSession) != CKR_OK)
  {
    return CKR_SESSION_HANDLE_INVALID;
  }

  /*Adapt Session handle to session index*/
  ul_session_index = hSession - STSAFE_P11_SESSION_INDEX_OFFSET;

  /* Reset all operations data */
  (void)memset(a_SessionList[ul_session_index].OperationObjects, 0U,
               sizeof(CK_OBJECT_HANDLE) * sizeof(a_SessionList[ul_session_index].OperationObjects) /
               (sizeof(a_SessionList[ul_session_index].OperationObjects[0U])));
  a_SessionList[ul_session_index].ulOperationObjectsCount = 0UL;

  /* Mark operation as not active */
  a_SessionList[ul_session_index].OperationActive = CK_FALSE;

  return CKR_OK;
}

/**
  * @brief  Assingn an object to a session.
  * @param  hObject    the object handle
  * @param  hSession   the session handle
  * @retval CK_RV CKR_SESSION_HANDLE_INVALID, CKR_GENERAL_ERROR, CKR_OK.
  */
static CK_RV    ObjectSetSession(CK_OBJECT_HANDLE hObject, CK_SESSION_HANDLE hSession)
{
  CK_RV e_ret_status;
  CK_ATTRIBUTE_PTR p_template_embedded;
  CK_ULONG ul_count_embedded;
  CK_ATTRIBUTE_PTR p_attribute_embedded;


  /*Init default*/
  e_ret_status = CKR_GENERAL_ERROR;

  /* Get the pointer to the template of a specific object  */
  if (STSAFE_OBJ_GetTemplateFromObjectHandle(&p_template_embedded, &ul_count_embedded, hObject) == CKR_OK)
  {
    /* ========== Check CKA_VENDOR_SESSION_HANDLE attribute ========== */
    /* Check for the specific attribute  */
    if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_embedded, CKA_VENDOR_SESSION_HANDLE, p_template_embedded,
                                           ul_count_embedded) == CKR_OK)
    {
      /* Set attribute  */
      e_ret_status = STSAFE_OBJ_SetAttributeValue(p_attribute_embedded, (CK_BYTE_PTR)&hSession,
                                                  sizeof(CK_SESSION_HANDLE));
    }
  }

  return e_ret_status;
}


/**
  * @brief  Set attribute for Vendor Object Availability .
  * @param  hObject    the object handle
  * @param  hSession   the session handle
  * @param  bValue     value to set
  * @retval CK_RV, CKR_GENERAL_ERROR, CKR_OK.
  */
static CK_RV ObjectSetVendorObjectAvailability(CK_OBJECT_HANDLE hObject, CK_BBOOL bValue)
{
  CK_RV e_ret_status;
  CK_ATTRIBUTE_PTR p_template_embedded;
  CK_ULONG ul_count_embedded;
  CK_ATTRIBUTE_PTR p_attribute_embedded;


  /*Init default*/
  e_ret_status = CKR_GENERAL_ERROR;

  /* Get the pointer to the template of a specific object  */
  if (STSAFE_OBJ_GetTemplateFromObjectHandle(&p_template_embedded, &ul_count_embedded, hObject) == CKR_OK)
  {
    /* ========== Check CKA_VENDOR_OBJECT_AVAILABILITY attribute ========== */
    /* Check for the specific attribute  */
    if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_embedded, CKA_VENDOR_OBJECT_AVAILABILITY, p_template_embedded,
                                           ul_count_embedded) == CKR_OK)
    {
      /* Set attribute  */
      e_ret_status = STSAFE_OBJ_SetAttributeValue(p_attribute_embedded,(CK_BYTE*)&bValue, sizeof(CK_BBOOL));
    }
  }

  return e_ret_status;
}



/**
  * @brief  Checks if a mechanism type is allowed.
  * @param  MechanismIn    mechanism
  * @param  pMechanism     mech. array
  * @param  ulValueLen     length of the mechanism array
  * @retval CK_BBOOL CK_TRUE, CK_FALSE.
  */
static CK_BBOOL IsMechanismTypeAllowed(CK_MECHANISM_TYPE MechanismIn, CK_MECHANISM_PTR  pMechanism, CK_ULONG ulValueLen)
{
  CK_ULONG i;

  if ((pMechanism == NULL_PTR) || (ulValueLen == 0))
  {
    return CK_FALSE;
  }

  /* Look in the mechanism array for MechanismIn*/
  for (i = 0; i < ulValueLen; i++)
  {
    if (pMechanism->mechanism == MechanismIn)
    {
      return CK_TRUE;
    }

    pMechanism++;
  }
  return CK_FALSE;
}

/**
  * @brief  Get attribute value from a template.
  * @param  pValue     gets the attribute value.
  * @param  pulValueLen     gets the attribute value len.
  * @param  Type    attribute to extract from the template.
  * @param  pTemplate     template to look for the attribute
  * @param  ulCount    number of attributes in the template.
  * @retval CK_RV CKR_ATTRIBUTE_TYPE_INVALID, CKR_FUNCTION_FAILED, CKR_GENERAL_ERROR, CKR_OK.
  */
static CK_RV GetAttributeValueFromTemplate(CK_BYTE_PTR pValue,  CK_ULONG_PTR pulValueLen, CK_ATTRIBUTE_TYPE Type,
                                           CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount)
{
  CK_RV e_ret_status;
  CK_ATTRIBUTE_PTR p_attribute;
  CK_EXT_TOKEN_ZONE zone;
  CK_EXT_TOKEN_ZONE_AC access_condition;

  /* ========== Look for the specific attribute type  ========== */

  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute, Type, pTemplate, ulCount) != CKR_OK)
  {
    return CKR_ATTRIBUTE_TYPE_INVALID;
  }

  /* ================ Check if the value is available on STSAFE ==============*/
  /* Note: if VALUE is stored in External Token, CKA_VALUE has pValue == NULL_PTR and ulValueLen ==0UL */

  if ((Type == CKA_VALUE) && (p_attribute->pValue == NULL_PTR) && (p_attribute->ulValueLen == 0UL))
  {
    /* Check if this attribute is stored in any STSAFE Zone: i.e. if CKA_EXT_TOKEN_ZONE is present*/
    if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute, CKA_EXT_TOKEN_ZONE, pTemplate, ulCount) == CKR_OK)
    {
      zone = (*((CK_EXT_TOKEN_ZONE *)(p_attribute->pValue)));

      /* Check what is STSAFE access condition required to access this zone */
      if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute, CKA_EXT_TOKEN_ZONE_AC, pTemplate, ulCount) == CKR_OK)
      {
        access_condition = (*((CK_EXT_TOKEN_ZONE_AC *)(p_attribute->pValue)));

        /* Read a certificate from STSAFE zone */
        /* Note: in this implementation the assumption is that STSAFE zones contain only certificates, one
                certificate per zone. */
        if (StSafeReadCertificate(pValue, (uint32_t *)pulValueLen, zone, access_condition) != STSAFEA_OK)
        {
          e_ret_status = CKR_FUNCTION_FAILED;
        }
        else
        {
          e_ret_status = CKR_OK;
        }
      }
      else
      {
        e_ret_status = CKR_ATTRIBUTE_TYPE_INVALID;
      }
    }
    else
    {
      e_ret_status = CKR_ATTRIBUTE_TYPE_INVALID;
    }
  }
  else
  {
    /* Attribute value is stored locally : Retrieve the value from embedded object */
    e_ret_status = STSAFE_OBJ_GetAttributeValue(pValue, pulValueLen, p_attribute);
  }

  return e_ret_status;
}

/**
  * @brief  Check if the attribute is sensitive.
  * @param  Type    attribute to check.
  * @param  pTemplate  template to look for the attribute
  * @param  ulCount    number of attributes in the template.
  * @retval CK_BBOOL CK_TRUE, CK_FALSE.
  */
static CK_BBOOL IsAttributeSensitive(CK_ATTRIBUTE_TYPE Type, CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount)
{
  CK_BBOOL sensitive, cka_extractable, cka_sensitive;
  CK_OBJECT_CLASS obj_class;
  CK_ATTRIBUTE_PTR p_attribute_tmp;

  /* Extract CKA_CLASS attribute from the template */
  if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_CLASS, pTemplate, ulCount) != CKR_OK)
  {
    /*If object class is not recognized, the object is considered sensitive*/
    sensitive = CK_TRUE;
  }
  else
  {
    obj_class = (*((CK_OBJECT_CLASS *)(p_attribute_tmp->pValue)));

    /* Check sensitive depending on the object class */
    switch (obj_class)
    {
      case CKO_PRIVATE_KEY:

        /*set default values*/
        cka_sensitive = CK_TRUE;
        cka_extractable = CK_FALSE;

        /* Check CKA_SENSITIVE attribute */
        if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_SENSITIVE, pTemplate, ulCount) == CKR_OK)
        {
          cka_sensitive = (*((CK_BBOOL *)(p_attribute_tmp->pValue)));
        }

        /* Check CKA_EXTRACTABLE attribute */
        if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_EXTRACTABLE, pTemplate, ulCount) == CKR_OK)
        {
          cka_extractable = (*((CK_BBOOL *)(p_attribute_tmp->pValue)));
        }

        /* Check the attributes with two configuration lists depending on the attributes of
         * CKA_SENSITIVE or CKA_EXTRACTABLE*/
        if ((cka_sensitive == CK_TRUE) || (cka_extractable == CK_FALSE))
        {
          sensitive = IsAttributeTypeInList(Type, a_SensAttList_CkoPrivateKey_NotExtr,
                                            SensAttList_CkoPrivateKey_NotExtrLen);
        }
        else
        {
          sensitive = IsAttributeTypeInList(Type, a_SensAttList_CkoPrivateKey_Extr,
                                            SensAttList_CkoPrivateKey_ExtrLen);
        }

        break;

      case CKO_SECRET_KEY:
        /*set default values*/
        cka_sensitive = CK_TRUE;
        cka_extractable = CK_FALSE;

        /* Check CKA_SENSITIVE attribute */
        if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_SENSITIVE, pTemplate, ulCount) == CKR_OK)
        {
          cka_sensitive = (*((CK_BBOOL *)(p_attribute_tmp->pValue)));
        }

        /* Check CKA_EXTRACTABLE attribute */
        if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_tmp, CKA_EXTRACTABLE, pTemplate, ulCount) == CKR_OK)
        {
          cka_extractable = (*((CK_BBOOL *)(p_attribute_tmp->pValue)));
        }

        /*Check the attributes with two configuration lists depending on the attributes of CKA_SENSITIVE or
        CKA_EXTRACTABLE*/
        if ((cka_sensitive == CK_TRUE) || (cka_extractable == CK_FALSE))
        {
          sensitive = IsAttributeTypeInList(Type, a_SensAttList_CkoSecretKey_NotExtr,
                                            SensAttList_CkoSecretKey_NotExtr);
        }
        else
        {
          sensitive = IsAttributeTypeInList(Type, a_SensAttList_CkoSecretKey_Extr,
                                            SensAttList_CkoSecretKey_ExtrLen);
        }
        break;

      case CKO_PUBLIC_KEY:
        /* CKO_PUBLIC_KEY objects are extractable by definition, they don't have CKA_EXTRACTABLE attribute*/
        sensitive = IsAttributeTypeInList(Type, a_SensitiveAttributesList_CkoPublicKey,
                                          SensitiveAttributesList_CkoPublicKeyLen);
        break;

      case CKO_CERTIFICATE:
        /* CKO_CERTIFICATE objects are extractable by definition, they don't have CKA_EXTRACTABLE attribute*/
        sensitive = IsAttributeTypeInList(Type, a_SensitiveAttributesList_CkoCertificate,
                                          SensitiveAttributesList_CkoCertificateLen);
        break;

      default:
        /*If object is not known - sensitive is true */
        sensitive = CK_TRUE;
        break;
    }
  }


  return sensitive;
}

/**
  * @brief  Read the content of a zone from STSAFE.
  * @param  ulZone     zone number.
  * @param  ulOffset     offset from where to start reading.
  * @param  pucData     gets the Data value.
  * @param  ulDataSize     data len.
  * @param  AccessCondition     type of access condition.
  * @retval CK_RV CKR_GENERAL_ERROR, CKR_OK.
  */
static CK_RV StSafeReadZone(uint8_t uZone, uint32_t ulOffset, uint8_t *pucData, uint32_t ulDataSize,
                            uint8_t AccessCondition)
{
  StSafeA_LVBuffer_t read_buffer;  
  uint32_t index;
  uint32_t tmp_len;
  uint32_t tmp_offset;

  tmp_len = ulDataSize;
  tmp_offset = ulOffset;

  index = 0UL;
  
  uint8_t data_sts_read [ulDataSize];
  read_buffer.Length = ulDataSize;
  read_buffer.Data = data_sts_read;

  /*Split the read in multiple chunks considering the limit of ST-SAFE DATA CONTENT size buffer */
  while (tmp_len >= STSAFEA_BUFFER_DATA_CONTENT_SIZE)
  {
    /* Read from STSAFE partition */
    if (StSafeA_Read(&hStSafe,
                     0U,
                     0U,
                     STSAFEA_AC_ALWAYS,
                     uZone,
                     (uint16_t)tmp_offset,
                     (uint16_t)STSAFEA_BUFFER_DATA_CONTENT_SIZE, /* in_length */ 
                     (uint16_t)STSAFEA_BUFFER_DATA_CONTENT_SIZE, /* InRespDataLen*/
                     &read_buffer, 
                     AccessCondition)!= STSAFEA_OK)
    {
      return CKR_GENERAL_ERROR;
    }

    /* Assign to output buffer if !=NULL*/
    if (pucData != NULL_PTR)
    {
      (void)memcpy(&pucData[index], read_buffer.Data, read_buffer.Length);
      index += STSAFEA_BUFFER_DATA_CONTENT_SIZE;
    }

    /* Update offset and len */
    tmp_offset += STSAFEA_BUFFER_DATA_CONTENT_SIZE;
    tmp_len -= STSAFEA_BUFFER_DATA_CONTENT_SIZE;
  }

  /*Update the last chunk*/
  if (tmp_len != 0UL)
  {
    /* Read from STSAFE partition */
    if (StSafeA_Read(&hStSafe,
                     0U,
                     0U,
                     STSAFEA_AC_ALWAYS,
                     uZone,
                     (uint16_t)tmp_offset,
                     (uint16_t)tmp_len,
                     (uint16_t)tmp_len,
                     &read_buffer,
                     AccessCondition) != STSAFEA_OK)
    {
      return CKR_GENERAL_ERROR;
    }

    /* Assign to output buffer if !=NULL*/
    if (pucData != NULL_PTR)
    {
      (void)memcpy(&pucData[index], read_buffer.Data, tmp_len);
    }
  }

  return CKR_OK;
}

/**
  * @brief  Read Certificate from STSAFE.
  * @param  pValue     gets the Data value.
  * @param  pulValueLen     data len.
  * @param  Zone     gets the Data value.
  * @param  AccessCondition     type of access condition.
  * @retval CK_RV CKR_GENERAL_ERROR, CKR_OK.
  */
static CK_RV StSafeReadCertificate(uint8_t *pValue, uint32_t *pulValueLen, uint8_t Zone, uint8_t AccessCondition)
{
  CK_BYTE a_sequence_header[4U];
  CK_ULONG ul_cert_len = 0UL;

  /* Check input parameters */
  if (pulValueLen == NULL_PTR)
  {
    return CKR_GENERAL_ERROR;
  }

  /* Read Certificate header */
  if (StSafeReadZone(Zone, 0U, a_sequence_header, sizeof(a_sequence_header), AccessCondition) != STSAFEA_OK)
  {
    return CKR_GENERAL_ERROR;
  }

  /* Parse the header and extract certificate len*/
  if (GetLenDerASN1SequenceHeader(&ul_cert_len, a_sequence_header, sizeof(a_sequence_header)) != CKR_OK)
  {
    return CKR_GENERAL_ERROR;
  }

  /*Copy value out only if pointer is != NULL_PTR*/
  /*No need to read the actual certificate if pointer is null*/
  if (pValue != NULL_PTR)
  {
    if (StSafeReadZone(Zone, 0U, pValue, ul_cert_len, AccessCondition) != STSAFEA_OK)
    {
      return CKR_GENERAL_ERROR;
    }
  }

  /* Copy valueLen out */
  *pulValueLen = ul_cert_len;

  return CKR_OK;
}

/* Public function prototypes -----------------------------------------------*/

/*!
 * \brief Update data through zone partition
 *
 * \param zone                   : Zone number
 * \param in_Data                : Data to copy inside zone
 * \param offset                 : Offset in selected zone
 * \return OK if no error
 */
static CK_RV StSafeUpdateZone(uint8_t zone, StSafeA_LVBuffer_t in_Data, uint16_t offset)
{
  uint8_t StatusCode = STSAFEA_OK;
  uint16_t Remaining_Data_To_Copy = 0;

  if (in_Data.Length <= MAX_SIZE_BUFFER_I2_FOR_DATAS)
  {

    STS_CHK(StatusCode, StSafeA_Update(&hStSafe, 0U, 0U, 0U, STSAFEA_AC_ALWAYS, zone, offset, &in_Data, STSAFEA_MAC_HOST_CMAC));

    if (StatusCode != STSAFEA_OK)
      return CKR_GENERAL_ERROR;

  }
  else /* Size of data to be send exceed max size of STSAFE-Axx buffer, We have to split data's in blocks  */
  {
    uint16_t size = in_Data.Length;

    /* First Write of data*/

    in_Data.Length = MAX_SIZE_BUFFER_I2_FOR_DATAS;

    STS_CHK(StatusCode, StSafeA_Update(&hStSafe, 0U, 0U, 0U, STSAFEA_AC_ALWAYS, zone, offset, &in_Data, STSAFEA_MAC_HOST_CMAC));

    if (StatusCode != STSAFEA_OK)
      return CKR_GENERAL_ERROR;

    /* Management of 2nd block of data with max data of 1000 bytes*/
    /* Check if it's needed to split in 2 */

    Remaining_Data_To_Copy = size - MAX_SIZE_BUFFER_I2_FOR_DATAS;

    if (Remaining_Data_To_Copy <= MAX_SIZE_BUFFER_I2_FOR_DATAS)
    {
      STSAFEA_DYNAMIC_ALLOCATE_LV_BUFFER_VAR(StSafeA_LVBuffer_t, DataUpdate, Remaining_Data_To_Copy + 2);
      DataUpdate.Length = Remaining_Data_To_Copy;

      for (uint8_t i = 0; i < DataUpdate.Length; i++)
      {
        DataUpdate.Data[i] = in_Data.Data[i + MAX_SIZE_BUFFER_I2_FOR_DATAS];
      }

      /* 2nd Write of data*/

      STS_CHK(StatusCode, StSafeA_Update(&hStSafe, 0U, 0U, 0U, STSAFEA_AC_ALWAYS, zone, MAX_SIZE_BUFFER_I2_FOR_DATAS + offset, &DataUpdate,
                                         STSAFEA_MAC_HOST_CMAC));

      if (StatusCode != STSAFEA_OK)
        return CKR_GENERAL_ERROR;

      STSAFEA_DYNAMIC_FREE_LV_BUFFER_VAR(DataUpdate);
    }
    else /*  it's needed to split in 3 blocks*/
    {

      STSAFEA_DYNAMIC_ALLOCATE_LV_BUFFER_VAR(StSafeA_LVBuffer_t, DataUpdate, MAX_SIZE_BUFFER_I2_FOR_DATAS + 2);

      DataUpdate.Length = MAX_SIZE_BUFFER_I2_FOR_DATAS;

      for (uint8_t i = 0; i < DataUpdate.Length; i++)
      {
        DataUpdate.Data[i] = in_Data.Data[i + MAX_SIZE_BUFFER_I2_FOR_DATAS];
      }

      /* 2nd Write of data*/

      STS_CHK(StatusCode, StSafeA_Update(&hStSafe, 0U, 0U, 0U, STSAFEA_AC_ALWAYS, zone, MAX_SIZE_BUFFER_I2_FOR_DATAS + offset, &DataUpdate,
                                         STSAFEA_MAC_HOST_CMAC));

      STSAFEA_DYNAMIC_FREE_LV_BUFFER_VAR(DataUpdate);

      if (StatusCode != STSAFEA_OK)
        return CKR_GENERAL_ERROR;
      Remaining_Data_To_Copy = size - 2 * MAX_SIZE_BUFFER_I2_FOR_DATAS;

      STSAFEA_DYNAMIC_ALLOCATE_LV_BUFFER_VAR(StSafeA_LVBuffer_t, DataUpdate2, Remaining_Data_To_Copy + 2);
      DataUpdate2.Length = Remaining_Data_To_Copy;

      for (uint8_t i = 0; i < DataUpdate2.Length; i++)
      {
        DataUpdate2.Data[i] = in_Data.Data[i + 2 * MAX_SIZE_BUFFER_I2_FOR_DATAS];
      }

      /* third Write of data*/

      STS_CHK(StatusCode, StSafeA_Update(&hStSafe, 0U, 0U, 0U, STSAFEA_AC_ALWAYS, zone, 2 * MAX_SIZE_BUFFER_I2_FOR_DATAS + offset, &DataUpdate2, STSAFEA_MAC_HOST_CMAC));

      if (StatusCode != STSAFEA_OK)
        return CKR_GENERAL_ERROR;

      STSAFEA_DYNAMIC_FREE_LV_BUFFER_VAR(DataUpdate2);
    }
  }

  return CKR_OK;
}

/**
  * @brief  Get Len value from ASN1 Sequence header.
  * @note: this implementation supports only Sequence with Len field <=0x82
  * @param  pulValueOutLen     sequence len.
  * @param  pValueIn     ASN1 Sequence header.
  * @param  ulValueInLen     data len.
  * @retval CK_RV CKR_GENERAL_ERROR, CKR_OK.
  */
static CK_RV GetLenDerASN1SequenceHeader(CK_ULONG_PTR pulValueOutLen, CK_BYTE_PTR pValueIn, CK_ULONG ulValueInLen)
{
  CK_RV e_ret_status;

  /*Check input parameters*/
  if ((pValueIn == NULL_PTR) || (ulValueInLen == 0UL) || (pulValueOutLen == NULL_PTR))
  {
    return CKR_GENERAL_ERROR;
  }

  /* Check ASN SEQUENCE TAG */
  if (pValueIn[0U] != ASN_SEQUENCE)
  {
    return CKR_GENERAL_ERROR;
  }

  /*Init*/
  e_ret_status = CKR_OK;

  /* Check Length */
  if ((pValueIn[1U] <= ASN_LONG_LEN) && (ulValueInLen >= 2U))
  {
    *pulValueOutLen = pValueIn[1U];

  }
  else if (((pValueIn[1U] == (ASN_LONG_LEN + 1U)) && (ulValueInLen >= 3U)))
  {
    *pulValueOutLen = pValueIn[2U] + 3UL;

  }
  else if (((pValueIn[1U] == (ASN_LONG_LEN + 2U)) && (ulValueInLen >= 4U)))
  {
    *pulValueOutLen = ((uint32_t)pValueIn[2U] << 8UL);
    *pulValueOutLen += pValueIn[3U];
    *pulValueOutLen += 4UL;

  }
  else
  {
    /*Note: this implementation supports only Sequence with Len field <=0x82*/
    e_ret_status = CKR_GENERAL_ERROR;
  }

  return e_ret_status;
}

/**
  * @brief  Decodes a raw octet string.
  * @note:  Raw octet string format: 0x04 | Len | Data.
  *         Example:
  *           |  04 0a                              ; OCTET_STRING (a Bytes)
  *           |     1e 08 00 55 00 73 00 65  00 72  ;   ...U.s.e.r
  *           If the byte array contains fewer than 128 bytes, the Length field
  *           of the TLV triplet requires only one byte to specify the content length.
  *           If it is more than 127 bytes, bit 7 of the Length field is set to 1 and
  *           bits 6 through 0 specify the number of additional bytes used to identify
  *           the content length.
  *           This implementation does NOT support long length octects
  *           ( i.e. Length field > 128 (0x80)).
  * @param  pValueOut        gets the raw data.
  * @param  pulValueOutLen   gets the raw data len.
  * @param  pValueIn         Raw OCTET String.
  * @param  ulValueInLen     Raw OCTET String Len.
  * @retval CKR_GENERAL_ERROR, CKR_OK.
  */
static CK_RV DecodeDerASN1OctetString(CK_BYTE_PTR pValueOut, CK_ULONG_PTR pulValueOutLen, CK_BYTE_PTR pValueIn,
                                      CK_ULONG ulValueInLen)
{
  uint32_t i = 0;
  uint32_t header_len = 0;

  /* Check input parameters */
  if ((pValueIn == NULL_PTR) || (ulValueInLen == 0))
  {
    return CKR_GENERAL_ERROR;
  }

  /* Check ASN OCTET STRING TAG */
  if (pValueIn[i++] != ASN_OCTET_STRING)
  {
    return CKR_GENERAL_ERROR;
  }

  header_len++;

  /* Check Length*/
  /* Note: This implementation does NOT support long length octects ( i.e. Length field > 128 (0x80)).*/
  if (pValueIn[i] >= ASN_LONG_LEN)
  {
    return CKR_GENERAL_ERROR;
  }

  header_len++;

  /* Check len field is consistent with the actual length */
  if (pValueIn[i++] != (ulValueInLen - header_len))
  {
    return CKR_GENERAL_ERROR;
  }

  /* Assign output len*/
  *pulValueOutLen = ulValueInLen - header_len;

  /* Assign output*/
  if (pValueOut != NULL_PTR)
  {
    (void)memcpy(pValueOut, &pValueIn[header_len], *pulValueOutLen) ;
  }

  return CKR_OK;
}

/**
  * @brief  Decodes a DER x9.62 EC Point to raw data X and Y coordinates.
  * @note:  DER-encoded x9.62 EC Point format: 0x04 | Len | 0x04 | X | Y.
  *         Example:
  *           |  04 41                              ; OCTET_STRING (a Bytes)
  *           |     04                              ; UNCOMPRESSED DER
  *           |     0x93, 0x5B, 0x79, 0xC5, 0xC3,   ; X and Y coordinates
  *           |     ....                            ;
  *           |     0xE6, 0xC7, 0x5C, 0x04, 0x96    ;
  *           This implementation does NOT support long length octects
  *           ( i.e. Len field > 128 (0x80)).
  * @param  pValueOut     gets the raw data.
  * @param  pulValueOutLen     gets the raw data len.
  * @param  pValueIn     DER x962 point.
  * @param  ulValueInLen     DER x962 point len.
  * @retval CKR_GENERAL_ERROR, CKR_OK.
  */
static CK_RV DecodeDerASN1UncompressedEcPoint(CK_BYTE_PTR pValueOut, CK_ULONG_PTR pulValueOutLen, CK_BYTE_PTR pValueIn,
                                              CK_ULONG ulValueInLen)
{
  uint32_t i = 0;
  CK_ULONG header_len = 0;

  /* Decode DER Encoded ASN1 Octet String */
  /* Check input parameters */
  if ((pValueIn == NULL_PTR) || (ulValueInLen == 0))
  {
    return CKR_GENERAL_ERROR;
  }

  /* Check ASN OCTET STRING TAG */
  if (pValueIn[i++] != ASN_OCTET_STRING)
  {
    return CKR_GENERAL_ERROR;
  }

  header_len++;

  /* Check Length*/
  /* Note: This implementation does NOT support long length octects ( i.e. Length field > 128 (0x80)).*/
  if (pValueIn[i] >= ASN_LONG_LEN)
  {
    return CKR_GENERAL_ERROR;
  }

  header_len++;

  /* Check len field is consistent with the actual length */
  if (pValueIn[i++] != (ulValueInLen - header_len))
  {
    return CKR_GENERAL_ERROR;
  }

  /* DER_UNCOMPRESSED OCTET TAG  */
  if (pValueIn[i] != DER_UNCOMPRESSED)
  {
    return CKR_GENERAL_ERROR;
  }

  header_len++;

  /* Assign output len - Remove DER UNCOMPRESSED TAG (0x04)*/
  *pulValueOutLen = ulValueInLen - header_len;

  /* Assign output*/
  if (pValueOut != NULL_PTR)
  {
    (void)memcpy(pValueOut, &pValueIn[header_len], *pulValueOutLen) ;
  }

  return CKR_OK;
}


/**
  * @brief  Encodes raw EC Point data to DER x9.62 EC point.
  * @note:  DER-encoded x9.62 EC Point format: 0x04 | Len | 0x04 | X | Y.
  *         Example:
  *           |  04 41                              ; OCTET_STRING (a Bytes)
  *           |     04                              ; UNCOMPRESSED DER
  *           |     0x93, 0x5B, 0x79, 0xC5, 0xC3,   ; X and Y coordinates
  *           |     ....                            ;
  *           |     0xE6, 0xC7, 0x5C, 0x04, 0x96    ;
  * @param  pValueOut     gets the DER x962 point.
  * @param  pulValueOutLen     gets the DER x962 point len.
  * @param  pValueIn     raw data X and Y coordinates.
  * @param  ulValueInLen     raw data X and Y coordinates len.
  * @retval CKR_GENERAL_ERROR, CKR_OK.
  */
static CK_RV EncodeRawToDerASN1UncompressedEcPoint(CK_BYTE_PTR pValueOut, CK_ULONG_PTR pulValueOutLen,
                                                   CK_BYTE_PTR pValueIn, CK_ULONG ulValueInLen)
{
  uint32_t i = 0;

  /* Check input parameters */
  if ((pulValueOutLen == NULL_PTR) || (pValueIn == NULL_PTR) || (ulValueInLen == 0))
  {
    return CKR_GENERAL_ERROR;
  }

  if (ulValueInLen >= ASN_LONG_LEN)
  {
    return CKR_GENERAL_ERROR;
  }

  /*output len*/
  *pulValueOutLen = ulValueInLen + 2UL + 1UL;

  /*Copy value out only if pointer !=NULL_PTR*/
  if (pValueOut != NULL_PTR)
  {
    /* Set the header */
    pValueOut[i] = ASN_OCTET_STRING;
    i++;
    pValueOut[i] = (uint8_t) ulValueInLen;
    pValueOut[i] += 1U;
    i++;

    /*Set the value*/
    pValueOut[i] = DER_UNCOMPRESSED;
    i++;
    (void)memcpy(&pValueOut[i], pValueIn, ulValueInLen + 1UL) ;
  }

  return CKR_OK;
}



/**
  * @brief  Check if an attribute is included in a list.
  * @param  Type     attribute type to look in the list.
  * @param  pAttributeList     list of attributes to look into.
  * @param  ulCount list length.
  * @retval CK_FALSE, CK_TRUE.
  */
static CK_BBOOL IsAttributeTypeInList(CK_ATTRIBUTE_TYPE Type, CK_ATTRIBUTE_TYPE *pAttributeList, CK_ULONG ulCount)
{
  uint32_t i = 0;

  /* Check input parameters */
  if ((pAttributeList == NULL_PTR) || (ulCount == 0))
  {
    return CK_FALSE;
  }

  /*Look the whole list for the type*/
  for (i = 0; i < ulCount; i++)
  {
    if (pAttributeList[i] == Type)
    {
      return CK_TRUE;
    }
  }

  /* Attribute not found in the list*/
  return CK_FALSE;
}

/**
  * @brief  Get the object handle of a specific template.
  * @note   look for a byte-to-byte correspondence of the template.
  * @param  hSession     session handle.
  * @param  phObject     gets the object handles.
  * @param  ulMaxCount     max number of objects to return.
  * @param  pulObjectCount     gets the number of objects found.
  * @note   only = 1 is supported if ulCount!=0.
  * @param  pTemplate    the template
  * @param  ulCount    size of the template.
  * @note   if ulCount == 0, retrieve all objects.
  * @retval CK_RV CKR_GENERAL_ERROR, CKR_OK.
  */
CK_RV FindObjectsFromTemplate(CK_SESSION_HANDLE hSession, CK_OBJECT_HANDLE_PTR phObject, CK_ULONG ulMaxCount,
                              CK_ULONG_PTR pulObjectCount,  CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount)
{
  CK_ATTRIBUTE_PTR p_template_embedded;
  CK_ATTRIBUTE_PTR p_attribute_embedded;
  CK_ULONG ul_count_embedded;
  CK_ULONG ul_attributes_found_count;
  CK_BYTE object_found;
  CK_OBJECT_HANDLE hObject, hObjectMin, hObjectMax;
  uint32_t i;

  /*Init*/
  object_found = 0;
  ul_attributes_found_count = 0;

  /*Get Objects handles for all objects allocated (token and session)*/
  if (STSAFE_OBJ_GetObjectRange(&hObjectMin, &hObjectMax) != CKR_OK)
  {
    return CKR_GENERAL_ERROR;
  }

  /*Init the loop*/
  hObject = hObjectMin;

  /*Loop on all local objects */
  while ((object_found < ulMaxCount) && (hObject < hObjectMax))
  {
    /* Get the pointer to the template of a specific object  */
    if (STSAFE_OBJ_GetTemplateFromObjectHandle(&p_template_embedded, &ul_count_embedded, hObject) != CKR_OK)
    {
      return CKR_GENERAL_ERROR;
    }

    /*Process only allocated objects */
    if ((p_template_embedded != NULL_PTR) && (ul_count_embedded != 0UL))
    {
      /* User is looking for objects with specific templates*/
      if (ulCount > 0UL)
      {
        /*Look for a specific sessions objects*/
        if (hSession != CKV_SESSION_HANDLE_ANY)
        {
          /* Check for the specific CKA_VENDOR_SESSION_HANDLE attribute  */
          if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_embedded, CKA_VENDOR_SESSION_HANDLE, p_template_embedded,
                                                 ul_count_embedded) == CKR_OK)
          {
            /*Check if CKA_VENDOR_SESSION_HANDLE == hSession*/
            if (*((CK_ULONG *)(p_attribute_embedded->pValue)) != hSession)
            {
              /*Skip this object - it is associated to another session*/
              hObject++;
              continue;
            }
          }

          if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_embedded, CKA_VENDOR_OBJECT_AVAILABILITY, p_template_embedded,
                                                  ul_count_embedded) == CKR_OK)
          {
            if (*((CK_BBOOL *)(p_attribute_embedded->pValue)) == FALSE)
            {
              /*Skip this object - it is destroyed */
              hObject++;
              continue;
            }
          }
        }

        /* Loop on all the attributes of the input template and
         * look for the corresponding attribute in the embedded template */
        for (i = 0; i < ulCount; i++)
        {
          /* Check for the specific attribute  */
          if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_embedded, pTemplate[i].type, p_template_embedded,
                                                 ul_count_embedded) == CKR_OK)
          {
            /* CKA ATTRIBUTE attribute found in the embedded object - now compare the two values */
            if (memcmp(p_attribute_embedded->pValue, pTemplate[i].pValue, p_attribute_embedded->ulValueLen) == 0)
            {
              /* Attribute found and the value is identical - increase the number of attributes found*/
              ul_attributes_found_count++;
            }
          }
        }

        /* Check if all the attributes were found */
        if (ul_attributes_found_count == ulCount)
        {
          /* Increment number of objects found*/
          phObject[object_found] = hObject;
          object_found++;
        }

        /* Reset the counter */
        ul_attributes_found_count = 0;

      }
      else
      {
        /* User is looking for all objects for this session(ulCount =0) */
        /* Check for the specific CKA_VENDOR_SESSION_HANDLE attribute  */
        if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute_embedded, CKA_VENDOR_SESSION_HANDLE, p_template_embedded,
                                               ul_count_embedded) == CKR_OK)
        {
          /*Check if CKA_VENDOR_SESSION_HANDLE == hSession*/
          if (*((CK_ULONG *)(p_attribute_embedded->pValue)) == hSession)
          {
            /* If it is a session object, return the object only if assigned to this session*/
            phObject[object_found] = hObject;
            object_found++;
          }
        }
        else
        {
          /* Not a session object, return the object*/
          phObject[object_found] = hObject;
          object_found++;
        }
      }
    }

    hObject++;
  }

  /* Assign the output */
  *pulObjectCount = object_found;

  return CKR_OK;
}
