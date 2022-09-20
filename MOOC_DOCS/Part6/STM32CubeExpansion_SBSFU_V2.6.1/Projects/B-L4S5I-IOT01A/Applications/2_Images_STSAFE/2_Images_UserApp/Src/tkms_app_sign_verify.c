/**
  ******************************************************************************
  * @file    .c
  * @author  MCD Application Team
  * @brief   KMS application examples module.
  *          This file provides examples of KMS API usage to sign & verify
  *          messages.
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

/* Includes ------------------------------------------------------------------*/
#include "tkms.h"
#include "tkms_app.h"
#include "com.h"
#include "common.h"
#include "kms_platf_objects_interface.h"
#include "mbedtls/x509_crt.h"

/* Private defines -----------------------------------------------------------*/
#define CERT_MAX_LEN              (600U)              /* Max certificate Length */
#define EC_POINT_MAX_LEN          (3U + (2U * 32U))   /* EC Point Max Length : X962 header + DER header +
                                                         NIST-P256 *2 - > (0x04|LEN|0x04|X|Y) */

/* Private variables ---------------------------------------------------------*/
static uint8_t a_string[1024] = {0};
static uint8_t signature[2048 / 8] __attribute__((aligned(8))) = {0};
static CK_BYTE a_Hash[] =  /*Hash to be signed*/
{
  0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U,
  0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U, 0x11U
};
static CK_BYTE  a_DeviceCert[CERT_MAX_LEN];                     /* Array to store device certificate*/
static CK_ULONG DeviceCertLen;                                  /*Device certificate Len*/
static CK_BYTE a_DevicePubKeyEcPoint[EC_POINT_MAX_LEN];         /*Device Pub Key EC Point */
static CK_ULONG DevicePubKeyEcPointLen;                         /*Device Pub Key EC Point Len*/
static CK_BYTE a_DevicePubKeyEcPointDER[EC_POINT_MAX_LEN + 2U]; /*Device Pub Key EC Point DER - including header */
static CK_ULONG DevicePubKeyEcPointDERLen;                      /*Device Pub Key EC Point DER Len*/


/**
  * @brief  RSA 1024 Bits
  *         This example is showing how to use RSA signature to
  *         authenticate a message.
  * @note
  *     RSA shared information between emitter and receiver
  *       - Secret signature/verification Key: stored in KMS under
  *         KMS_KEY_RSA2048_OBJECT_HANDLE object handle
  *       - Message to authenticate: transmitted from emitter to sender
  *       - Signature: transmitted from emitter to sender
  *             (calculated by emitter, verified by receiver)
  * @param  length  Length of the clear text for signature/verification tests
  * @param  pMessage  Clear text for signature/verification tests
  * @retval CK_RV return value.
  */
CK_RV tkms_app_sign_verify_rsa(uint32_t length, uint8_t *pMessage)
{

  CK_RV rv;
  CK_SESSION_HANDLE session;
  CK_FLAGS session_flags = CKF_SERIAL_SESSION;  /* Read ONLY session */
  uint32_t signature_length = 0UL;

  /* Prepare RSA mechanism */
  CK_MECHANISM rsa_mechanism = { CKM_SHA1_RSA_PKCS, (CK_VOID_PTR *) NULL, 0};

  /* Open session with KMS */
  rv = C_OpenSession(0,  session_flags, NULL, 0, &session);

  /* Signature --------------------------------------------------------------*/
  (void)printf("--- Signing -----------------------------------------------------\r\n");
  (void)printf("--- RSA 1024 bits\r\n");
  (void)printf("--- Message [%s]\r\n", pMessage);
  tkms_buff2str(pMessage, a_string, length);
  (void)printf("---         [0x%s]\r\n", a_string);
  (void)printf("--- Length  [%d]\r\n", length);

  /* Configure session to sign message using RSA with settings included into the mechanism */
  if (rv == CKR_OK)
  {
    rv = C_SignInit(session, &rsa_mechanism, (CK_OBJECT_HANDLE)KMS_KEY_RSA2048_OBJECT_HANDLE);
  }

  /* Sign message */
  if (rv == CKR_OK)
  {
    signature_length = sizeof(signature);
    rv = C_Sign(session, pMessage, length,
                signature, &signature_length);
  }

  (void)printf("--- Signed ------------------------------------------------------\r\n");
  tkms_buff2str(signature, a_string, signature_length);
  (void)printf("--- Signature [0x%s]\r\n", a_string);
  (void)printf("--- Length    [%d]\r\n", signature_length);

  /* Verification ------------------------------------------------------------*/

  /* Configure session to verify message using RSA with settings included into the mechanism */
  if (rv == CKR_OK)
  {
    rv = C_VerifyInit(session, &rsa_mechanism, (CK_OBJECT_HANDLE)KMS_KEY_RSA2048_OBJECT_HANDLE);
  }

  /* Verify message */
  if (rv == CKR_OK)
  {
    rv = C_Verify(session, pMessage, length,
                  signature, signature_length);
  }

  if (rv == CKR_OK)
  {
    (void)printf(">>> Message authenticated\r\n");
  }
  else
  {
    (void)printf("XXX Message not authenticated\r\n");
  }

  /* Close session with KMS */
  (void)C_CloseSession(session);

  return rv;
}

/**
  * @brief ECDSA NIST-P256.
  *        This example is showing how to use external token to sign a message using
  *        the unique device private key and how to authenticate the signature using the
  *        corresponding public key extracted from the device certificate.
  * @note
  *        - Unique Device Private Key label: label: P11_OBJ_LABEL_DEV_PRV_KEY
  *        - Unique Device Certificate label: label: P11_OBJ_LABEL_DEV_CRT
  *        - Message to authenticate: transmitted from emitter to sender
  *        - Signature: transmitted from emitter to sender
  *          (calculated by emitter, verified by receiver)
  * @param  None.
  * @retval CK_RV return value.
  */
CK_RV tkms_app_ext_token_sign_verify_ecdsa(void)
{

  CK_RV rv;
  CK_SESSION_HANDLE session;
  CK_FLAGS session_flags = CKF_SERIAL_SESSION;  /* Read ONLY session */
  CK_ATTRIBUTE template, template_read[5U], template_ver_key[2U];
  CK_ULONG ul_count;
  CK_ULONG signature_length = 0UL;
  CK_OBJECT_HANDLE h_obj_dev_prv_key, h_obj_dev_cert, h_obj_ver_key;
  CK_BYTE a_object_label[100];  /* Array to store Label values */
  CK_OBJECT_CLASS object_certificate_class;
  CK_CERTIFICATE_TYPE object_certificate_type;
  CK_CERTIFICATE_CATEGORY object_certificate_category;
  CK_MECHANISM ecdsa_mechanism;

  /* Open session with KMS */
  rv = C_OpenSession(0, session_flags, NULL, 0, &session);

  /* Find Unique Device Private Key Object ----------------------------------------------*/

  /* Fill in the template with the attribute to retrieve the specific certificate
  (CKA_LABEL is used to refer to a specific object) */
  template.type       = CKA_LABEL;
  template.pValue     = P11_OBJ_LABEL_DEV_PRV_KEY;
  template.ulValueLen = sizeof(P11_OBJ_LABEL_DEV_PRV_KEY);

  (void)printf("--- Find Object -----------------------------------------------------\r\n");
  (void)printf("--- Label [%s]\r\n", (uint8_t *)template.pValue);

  /* Find Objects Init : template of the object to retrieve is passed to KMS */
  if (rv == CKR_OK)
  {
    rv = C_FindObjectsInit(session, &template, 1);
  }

  /* Find Objects : Find object corresponding to the template specified in FindObjectsInit */
  if (rv == CKR_OK)
  {
    ul_count = 1;
    rv = C_FindObjects(session,  &h_obj_dev_prv_key, 1, &ul_count);
  }

  /* Find Objects Final: Finalize the operation */
  if (rv == CKR_OK)
  {
    rv = C_FindObjectsFinal(session);
  }

  (void)printf("--- Object Found ------------------------------------------------\r\n");
  (void)printf("--- Object Handle [0x%d]\r\n", h_obj_dev_prv_key);

  /* Signature --------------------------------------------------------------*/
  (void)printf("--- Signing -----------------------------------------------------\r\n");
  (void)printf("--- ECDSA NIST-P256\r\n");
  tkms_buff2str(a_Hash, a_string, sizeof(a_Hash));
  (void)printf("--- Hash    [%s]\r\n", a_string);
  (void)printf("--- Length  [%d]\r\n", sizeof(a_Hash));


  /* Configure session to sign message using ECDSA object found with FindObject
  with settings included into the mechanism */

  if (rv == CKR_OK)
  {
    /* Initialize mechanism parameters for ECDSA signature */
    ecdsa_mechanism.pParameter = NULL;
    ecdsa_mechanism.ulParameterLen = 0UL;
    ecdsa_mechanism.mechanism = CKM_ECDSA;

    rv = C_SignInit(session, &ecdsa_mechanism, h_obj_dev_prv_key);
  }

  /* Sign message */
  if (rv == CKR_OK)
  {
    /* Notify KMS there is enough space to store the signature */
    signature_length = sizeof(signature);
    rv = C_Sign(session, a_Hash, sizeof(a_Hash),
                signature, &signature_length);
  }

  (void)printf("--- Signed ------------------------------------------------------\r\n");
  tkms_buff2str(signature, a_string, signature_length);
  (void)printf("--- Signature [%s]\r\n", a_string);
  (void)printf("--- Length    [%d]\r\n", signature_length);

  /* Find Unique Device Certificate Object ----------------------------------------------*/

  /* Fill in the template with the attribute to retrieve the specific certificate
  (CKA_LABEL is used to refer to a specific object) */
  template.type       = CKA_LABEL;
  template.pValue     = P11_OBJ_LABEL_DEV_CRT;
  template.ulValueLen = sizeof(P11_OBJ_LABEL_DEV_CRT);

  (void)printf("--- Find Object -----------------------------------------------------\r\n");
  (void)printf("--- Label [%s]\r\n", (uint8_t *)template.pValue);

  /* Find Objects Init : template of the object to retrieve is passed to KMS */
  if (rv == CKR_OK)
  {
    rv = C_FindObjectsInit(session, &template, 1);
  }

  /* Find Objects : Find object corresponding to the template specified in FindObjectsInit */
  if (rv == CKR_OK)
  {
    ul_count = 1;
    rv = C_FindObjects(session,  &h_obj_dev_cert, 1, &ul_count);
  }

  /* Find Objects Final: Finalize the operation */
  if (rv == CKR_OK)
  {
    rv = C_FindObjectsFinal(session);
  }

  (void)printf("--- Object Found ------------------------------------------------\r\n");
  (void)printf("--- Object Handle [0x%d]\r\n", h_obj_dev_cert);

  /* Fill in the template with attributes to retrieve from the object found with FindObjects */
  template_read[0].type       = CKA_LABEL;
  template_read[0].pValue     = a_object_label;
  template_read[0].ulValueLen = sizeof(a_object_label);
  template_read[1].type       = CKA_CLASS;
  template_read[1].pValue     = &object_certificate_class;
  template_read[1].ulValueLen = sizeof(CK_OBJECT_CLASS);
  template_read[2].type       = CKA_CERTIFICATE_TYPE;
  template_read[2].pValue     = &object_certificate_type;
  template_read[2].ulValueLen = sizeof(CK_CERTIFICATE_TYPE);
  template_read[3].type       = CKA_CERTIFICATE_CATEGORY;
  template_read[3].pValue     = &object_certificate_category;
  template_read[3].ulValueLen = sizeof(CK_CERTIFICATE_CATEGORY);
  template_read[4].type       = CKA_VALUE;
  template_read[4].pValue     = a_DeviceCert;
  template_read[4].ulValueLen = sizeof(a_DeviceCert);

  /* Get attribute value*/
  if (rv == CKR_OK)
  {
    rv = C_GetAttributeValue(session,  h_obj_dev_cert, template_read, sizeof(template_read) / sizeof(template_read[0]));
  }

  DeviceCertLen = template_read[4].ulValueLen;

  (void)printf("--- Device Certificate ------------------------------------------\r\n");
  tkms_buff2str(template_read[4].pValue, a_string, DeviceCertLen);
  (void)printf("--- Device Certificate [%s]\r\n", a_string);
  (void)printf("--- Length    [%d]\r\n", DeviceCertLen);

  /* Extract Unique Device Public key ----------------------------------------------*/
  mbedtls_x509_crt dev_cert_x509_crt;
  mbedtls_ecdsa_context dev_cert_ecdsa_ctx;
  mbedtls_ecp_group dev_cert_grp;

  /* Initialize MBED X509 core for certificate parsing */
  mbedtls_x509_crt_init(&dev_cert_x509_crt);
  mbedtls_ecdsa_init(&dev_cert_ecdsa_ctx);
  mbedtls_ecp_group_init(&dev_cert_grp);

  mbedtls_x509_crt_parse(&dev_cert_x509_crt, a_DeviceCert, DeviceCertLen);
  mbedtls_ecdsa_from_keypair(&dev_cert_ecdsa_ctx, mbedtls_pk_ec(dev_cert_x509_crt.pk));
  mbedtls_ecp_group_load(&dev_cert_grp, MBEDTLS_ECP_DP_SECP256R1) ;

  /* Extracting the public key EC POINT from the certificate */
  mbedtls_ecp_point_write_binary(&dev_cert_grp, &(mbedtls_pk_ec(dev_cert_x509_crt.pk)->Q), MBEDTLS_ECP_PF_UNCOMPRESSED,
                                 (size_t *)&DevicePubKeyEcPointLen, a_DevicePubKeyEcPoint,
                                 sizeof(a_DevicePubKeyEcPoint));

  /*DER Header preparation*/
  a_DevicePubKeyEcPointDER[0] = 0x04;
  a_DevicePubKeyEcPointDER[1] = DevicePubKeyEcPointLen;
  memcpy(&a_DevicePubKeyEcPointDER[2], a_DevicePubKeyEcPoint, DevicePubKeyEcPointLen);
  DevicePubKeyEcPointDERLen = DevicePubKeyEcPointLen + 2U;

  mbedtls_ecp_group_free(&dev_cert_grp);
  mbedtls_x509_crt_free(&dev_cert_x509_crt);
  mbedtls_ecdsa_free(&dev_cert_ecdsa_ctx);

  (void)printf("--- Device Unique Public Key ----------------------------------------------\r\n");
  tkms_buff2str(a_DevicePubKeyEcPointDER, a_string, DevicePubKeyEcPointDERLen);
  (void)printf("--- Public Key [%s]\r\n", a_string);
  (void)printf("--- Length     [%d]\r\n", DevicePubKeyEcPointDERLen);

  /* Create Object for verification using the Device Public Key ----------------------------------------------*/

  /* Fill in the template with the values to create the verification key (NIST-P256) */
  template_ver_key[0].type       = CKA_LABEL;
  template_ver_key[0].pValue     = P11_OBJ_LABEL_VER_PUB_KEY_256;
  template_ver_key[0].ulValueLen = sizeof(P11_OBJ_LABEL_VER_PUB_KEY_256);
  template_ver_key[1].type       = CKA_EC_POINT;
  template_ver_key[1].pValue     = (void *)a_DevicePubKeyEcPointDER;
  template_ver_key[1].ulValueLen = DevicePubKeyEcPointDERLen;

  /* Create Object : template of the object to create is passed to KMS */
  if (rv == CKR_OK)
  {
    rv = C_CreateObject(session, template_ver_key, (sizeof(template_ver_key) / sizeof(template_ver_key[0])),
                        &h_obj_ver_key);
  }

  (void)printf("--- Object Created -----------------------------------------------\r\n");
  (void)printf("--- Object Handle [0x%d]\r\n", h_obj_ver_key);

  /* Verification ------------------------------------------------------------*/

  /* Configure session to sign message using ECDSA object created with CreateObject
  with settings included into the mechanism */
  if (rv == CKR_OK)
  {
    ecdsa_mechanism.pParameter = NULL;
    ecdsa_mechanism.ulParameterLen = 0UL;
    ecdsa_mechanism.mechanism = CKM_ECDSA;

    rv = C_VerifyInit(session, &ecdsa_mechanism, h_obj_ver_key);
  }

  /* Verify message */
  if (rv == CKR_OK)
  {
    rv = C_Verify(session, a_Hash, sizeof(a_Hash),
                  signature, signature_length);
  }

  if (rv == CKR_OK)
  {
    (void)printf(">>> Message authenticated\r\n");
  }
  else
  {
    (void)printf("XXX Message not authenticated\r\n");
  }


  /* Close session with KMS */
  (void)C_CloseSession(session);

  return rv;
}
