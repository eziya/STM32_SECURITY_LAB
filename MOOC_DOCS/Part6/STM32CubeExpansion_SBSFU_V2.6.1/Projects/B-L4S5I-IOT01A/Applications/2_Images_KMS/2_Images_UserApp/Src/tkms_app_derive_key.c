/**
  ******************************************************************************
  * @file    tkms_app_derive_key.c
  * @author  MCD Application Team
  * @brief   tKMS application examples module.
  *          This file provides examples of KMS API usage to derivate a key
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
#define EC_POINT_MAX_LEN          (3U + (2U * 32U))   /* EC Point Max Length : X962 header + DER header +
                                                         NIST-P256 *2 - > (0x04|LEN|0x04|X|Y) */
#define DH_SECRET_X_MAX_LEN       (32U)               /* DH Secret X Max Length */

/* Private structures --------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static uint8_t a_string[1024] = {0};


/**
  * @brief  Key derivation
  *         This example is showing how to use key derivation to create
  *         a specific session key.
  * @note
  *     Key derivation shared information between emitter and receiver
  *       - Secret derivation Key: stored in KMS under
  *                  KMS_KEY_AES128_DERIVABLE_OBJECT_HANDLE object handle
  *       - Session key variation: transmitted or calculated by both emitter & receiver
  * @param  length  Length of the clear text for encryption/decryption tests
  * @param  pClearMessage  Clear text for encryption/decryption tests
  * @retval CK_RV return value.
  */
CK_RV tkms_app_derive_key(uint32_t length, uint8_t *pClearMessage)
{
  uint8_t encrypted_message[256] __attribute__((aligned(8))) = {0};
  uint8_t decrypted_message[256] __attribute__((aligned(8))) = {0};
  /* Generated key template definition */
  CK_ULONG DeriveKey_template_class = CKO_SECRET_KEY;
  CK_ULONG DeriveKey_template_destroyable = CK_TRUE;
  CK_ULONG DeriveKey_template_encrypt = CK_TRUE;
  CK_ULONG DeriveKey_template_decrypt = CK_TRUE;
  CK_ULONG DeriveKey_template_extract = CK_TRUE;
  CK_ATTRIBUTE DeriveKey_template[] =
  {
    {CKA_CLASS, (CK_VOID_PTR) &DeriveKey_template_class,       sizeof(CK_ULONG)},
    {CKA_DESTROYABLE, (CK_VOID_PTR) &DeriveKey_template_destroyable, sizeof(CK_ULONG)},
    {CKA_ENCRYPT, (CK_VOID_PTR) &DeriveKey_template_encrypt,     sizeof(CK_ULONG)},
    {CKA_DECRYPT, (CK_VOID_PTR) &DeriveKey_template_decrypt,     sizeof(CK_ULONG)},
    {CKA_EXTRACTABLE, (CK_VOID_PTR) &DeriveKey_template_extract,     sizeof(CK_ULONG)}
  };
  /* Template to retrieve generate key value */
  CK_ATTRIBUTE GetKey_template[] = {{CKA_VALUE, (CK_VOID_PTR) &DeriveKey_template_class, 32UL}};
  CK_RV rv;
  CK_SESSION_HANDLE session;
  CK_FLAGS session_flags = CKF_SERIAL_SESSION;  /* Read ONLY session */
  /* Key derivation */
  uint8_t session_variant[32] = "My session variation 0122004578";
  CK_MECHANISM      mech = {CKM_AES_ECB_ENCRYPT_DATA, session_variant, 32};
  CK_OBJECT_HANDLE  derivedKeyHdle = 0UL;
  uint8_t derivedKey[32] = {0};
  /* AES encryption / decryption */
  uint32_t encrypted_length = 0UL;
  uint32_t decrypted_length = 0UL;
  uint8_t cbc_iv[16] = ">CBC VECTOR    ";
  CK_MECHANISM aes_cbc_mechanism = { CKM_AES_CBC, (CK_VOID_PTR)cbc_iv, sizeof(cbc_iv) };
  uint8_t tag[16] = {0};
  uint32_t tag_lenth;

  /* Open session with KMS */
  rv = C_OpenSession(0,  session_flags, NULL, 0, &session);

  /* Derive key with pass phrase */
  if (rv == CKR_OK)
  {
    rv = C_DeriveKey(session, &(mech), (CK_OBJECT_HANDLE)KMS_KEY_AES128_DERIVABLE_OBJECT_HANDLE,
                     &DeriveKey_template[0], sizeof(DeriveKey_template) / sizeof(CK_ATTRIBUTE), &derivedKeyHdle);
  }

  /* Get derived key to display */
  if (rv == CKR_OK)
  {
    GetKey_template[0].pValue = derivedKey;
    rv = C_GetAttributeValue(session, derivedKeyHdle, &(GetKey_template[0]), sizeof(GetKey_template) /
                             sizeof(CK_ATTRIBUTE));
  }
  if (rv == CKR_OK)
  {
    (void)printf("--- Derivating key --------------------------------------------\r\n");
    (void)printf("--- AES ECB\r\n");
    (void)printf("--- Pass phrase   [%s]\r\n", session_variant);
    tkms_buff2str(derivedKey, a_string, 32);
    (void)printf("--- Derived key   [%s]\r\n", a_string);
  }

  /* Encryption --------------------------------------------------------------*/
  (void)printf("--- Encrypting --------------------------------------------------\r\n");
  (void)printf("--- AES CBC\r\n");
  (void)printf("--- IV      [%s]\r\n", cbc_iv);
  (void)printf("--- Message [%s]\r\n", pClearMessage);
  tkms_buff2str(pClearMessage, a_string, length);
  (void)printf("---         [0x%s]\r\n", a_string);
  (void)printf("--- Length  [%d]\r\n", length);

  /* Configure session to encrypt message in AES GCM with settings included into the mechanism */
  if (rv == CKR_OK)
  {
    rv = C_EncryptInit(session, &aes_cbc_mechanism, derivedKeyHdle);
  }

  /* Encrypt clear message */
  if (rv == CKR_OK)
  {
    encrypted_length = sizeof(encrypted_message);
    rv = C_EncryptUpdate(session, pClearMessage, length,
                         encrypted_message, &encrypted_length);
  }

  /* Finalize message encryption */
  if (rv == CKR_OK)
  {
    tag_lenth = sizeof(tag);
    rv = C_EncryptFinal(session, &tag[0], &tag_lenth);
  }

  (void)printf("--- Encrypted ---------------------------------------------------\r\n");
  tkms_buff2str(encrypted_message, a_string, encrypted_length);
  (void)printf("--- Message [0x%s]\r\n", a_string);

  /* Decryption --------------------------------------------------------------*/

  /* Configure session to decrypt message in AES GCM with settings included into the mechanism */
  if (rv == CKR_OK)
  {
    rv = C_DecryptInit(session, &aes_cbc_mechanism, derivedKeyHdle);
  }

  /* Decrypt encrypted message */
  if (rv == CKR_OK)
  {
    decrypted_length = sizeof(decrypted_message);
    rv = C_DecryptUpdate(session, encrypted_message, encrypted_length,
                         decrypted_message, &decrypted_length);
  }

  /* Finalize message decryption */
  if (rv == CKR_OK)
  {
    tag_lenth = sizeof(tag);
    rv = C_DecryptFinal(session, &tag[0], &tag_lenth);
  }
  (void)printf("--- Decrypted ---------------------------------------------------\r\n");
  (void)printf("--- Message [%s]\r\n", decrypted_message);
  tkms_buff2str(decrypted_message, a_string, decrypted_length);
  (void)printf("---         [0x%s]\r\n", a_string);

  /* Compare decrypted message with clear one */
  if (rv == CKR_OK)
  {
    if (memcmp(pClearMessage, decrypted_message, length) != 0)
    {
      (void)printf("XXX Decrypted message differs\r\n");
      rv = CKR_FUNCTION_FAILED;
    }
    else
    {
      (void)printf(">>> Decrypted message is same\r\n");
    }
  }

  /* Remove key once no more needed */
  if (rv == CKR_OK)
  {
    rv = C_DestroyObject(session, derivedKeyHdle);
  }

  /* Verify key is no more usable */
  if (rv == CKR_OK)
  {
    if (C_EncryptInit(session, &aes_cbc_mechanism, derivedKeyHdle) == CKR_OK)
    {
      rv = CKR_FUNCTION_FAILED;
    }
  }

  /* Close sessions */
  (void)C_CloseSession(session);

  return rv;
}

/**
  * @brief  ECDH Ephemeral Key Generation + Diffie-Hellman derivation
  *        This example is showing how to use KMS to generate an
  *        ephemeral NIST-P256 key pair and how to use the private key to compute a
  *        Diffie-Hellman derivation in combination with a peer public key.
  *        The same shared secret is simulated on peer side combining the ephemeral
  *        public key generated using KMS with a peer private key.
  *        This example simulates the Diffie-Hellman derivation in a TLS-like scenario.
  * @param  None.
  * @retval CK_RV return value.
  */
CK_RV tkms_app_ECDH_generate_derive_key(void)
{
  CK_BYTE a_DeviceEphemeralPubKeyEcPoint[EC_POINT_MAX_LEN];   /*Device Ephemeral Public Key EC Point*/
  CK_ULONG DeviceEphemeralPubKeyEcPointLen;                   /*Device Ephemeral Public Key EC Point Len*/
  /*
   * In this test peer ECDH communication endpoint is simulated
   * through call to MBED services.
   */
  CK_BYTE a_PeerPubKeyEcPoint[] = /*Peer Public Key EC Point*/
  {
    0x04, 0x41, 0x04, /*X962 header + DER header*/
    0x93, 0x5B, 0x79, 0xC5, 0xC3, 0x10, 0x7C, 0x4A, 0x86, 0x24, 0x6C, 0xB5, 0x10, 0x10, 0x0F, 0xBB, 0xEE, 0x34, 0x81,
    0xFB, 0x24, 0x1D, 0xDE, 0x65, 0x65, 0x09, 0xEF, 0x50, 0xFB, 0xEE, 0x6E, 0x24, 0x4F, 0x76, 0x88, 0x51, 0x87, 0x06,
    0xDD, 0x07, 0x4F, 0x2F, 0x82, 0xA8, 0x8B, 0x05, 0xC1, 0x3F, 0xFD, 0x3D, 0xD6, 0x79, 0x00, 0x5A, 0x6D, 0xDE, 0xE0,
    0xDD, 0xE9, 0xE6, 0xC7, 0x5C, 0x04, 0x96
  };
  CK_ULONG PeerPubKeyEcPointLen = sizeof(a_PeerPubKeyEcPoint);       /*Peer Public Key EC Point Len */
  CK_BYTE a_PeerPrvKey[] = /*Peer Private Key*/
  {
    0x3F, 0x3B, 0x67, 0x61, 0x8C, 0x6F, 0xD3, 0x1F, 0xFB, 0xF9, 0x06, 0x1F, 0xFC, 0xD5, 0x1C, 0x1E,
    0xC7, 0x8C, 0x7A, 0x24, 0xFA, 0x36, 0x7B, 0xB7, 0xA2, 0x13, 0x05, 0x4F, 0xB8, 0x77, 0x82, 0xE7
  };
  CK_ULONG PeerPrvKeyEcPointLen = sizeof(a_PeerPrvKey);/*Peer Private Key Len */
  CK_BYTE  a_DeviceSecretX[DH_SECRET_X_MAX_LEN];       /* Device Diffie-Hellman Device Shared Secret X coordinate */
  CK_ULONG DeviceSecretXLen;                          /* Device Diffie-Hellman Device Shared Secret X coordinate Len */
  CK_BYTE  a_PeerSecretX[DH_SECRET_X_MAX_LEN];        /* Peer Diffie-Hellman Shared Secret X coordinate */
  CK_ULONG PeerSecretXLen = sizeof(a_PeerSecretX);    /* Peer Diffie-Hellman Shared Secret X coordinate Len */
  CK_RV rv;
  CK_SESSION_HANDLE session;
  CK_FLAGS session_flags = CKF_SERIAL_SESSION;  /* Read ONLY session */
  CK_ATTRIBUTE template_pub_key[2], template_prv_key, template_read, template_shrd_scrt;
  CK_OBJECT_HANDLE h_obj_pub_key = 0UL;
  CK_OBJECT_HANDLE h_obj_prv_key = 0UL;
  CK_OBJECT_HANDLE h_obj_shrd_scrt = 0UL;
  CK_MECHANISM key_gen_mech, der_mech;
  CK_ECDH1_DERIVE_PARAMS der_params;
  uint32_t ref_secp256r1[] = {0x06082a86, 0x48ce3d03, 0x0107};    /* SECP256R1 OID */
  uint8_t eph_priv_key_label[] = "EphemeralPrivateKey";
  uint8_t eph_pub_key_label[] = "EphemeralPublicKey";
  uint8_t dh_shared_secret_label[] = "DHSharedSecretX";

  /* Open session with KMS */
  rv = C_OpenSession(0,  session_flags, NULL, 0, &session);

  /* Generate Key Pair ECDSA NIST-P256 -------------------------------------------------------*/

  /* Fill in the template for the publib key */
  template_pub_key[0].type       = CKA_LABEL;
  template_pub_key[0].pValue     = eph_pub_key_label;
  template_pub_key[0].ulValueLen = sizeof(eph_pub_key_label);
  template_pub_key[1].type       = CKA_EC_PARAMS;
  template_pub_key[1].pValue     = ref_secp256r1;
  template_pub_key[1].ulValueLen = 10;

  /* Fill in the template for the private key */
  template_prv_key.type       = CKA_LABEL;
  template_prv_key.pValue     = eph_priv_key_label;
  template_prv_key.ulValueLen = sizeof(eph_priv_key_label);

  (void)printf("--- Generate Key Pair -----------------------------------------------------\r\n");
  (void)printf("--- ECDSA NIST-P256\r\n");
  (void)printf("--- Public Key Label [%s]\r\n", (uint8_t *)template_pub_key[0].pValue);
  (void)printf("--- Private Key Label [%s]\r\n", (uint8_t *)template_prv_key.pValue);

  /* Generate Key Pair NIST-P256 with settings included into the mechanism */

  if (rv == CKR_OK)
  {
    key_gen_mech.pParameter = NULL;
    key_gen_mech.ulParameterLen = 0UL;
    key_gen_mech.mechanism = CKM_EC_KEY_PAIR_GEN;

    rv = C_GenerateKeyPair(session, &key_gen_mech, template_pub_key, 2, &template_prv_key, 1, &h_obj_pub_key,
                           &h_obj_prv_key);
  }

  /* Retrieve Public Key value - private key never leaves KMS*/

  /* Fill in the template with attribute to retrieve */
  template_read.type       = CKA_EC_POINT;
  template_read.pValue     = a_DeviceEphemeralPubKeyEcPoint;
  template_read.ulValueLen = sizeof(a_DeviceEphemeralPubKeyEcPoint);

  /* Get attribute value*/
  if (rv == CKR_OK)
  {
    rv = C_GetAttributeValue(session,  h_obj_pub_key, &template_read, 1);
  }

  DeviceEphemeralPubKeyEcPointLen = template_read.ulValueLen;

  (void)printf("--- Key Pair Generated ------------------------------------------------------\r\n");
  tkms_buff2str(a_DeviceEphemeralPubKeyEcPoint, a_string, DeviceEphemeralPubKeyEcPointLen);
  (void)printf("--- Public Key [%s]\r\n", a_string);
  (void)printf("--- Length     [%d]\r\n", DeviceEphemeralPubKeyEcPointLen);

  /* Diffie-Hellman Key Derivation (Device) -------------------------------------------------*/

  /* Fill in the template to refer to the shared secret
  (CKA_LABEL is used to refer to a specific object) */
  template_shrd_scrt.type       = CKA_LABEL;
  template_shrd_scrt.pValue     = dh_shared_secret_label;
  template_shrd_scrt.ulValueLen = sizeof(dh_shared_secret_label);

  /*Specifcy derivation params: CKD_NULL with Peer Public Key specified in input*/
  der_params.kdf             = CKD_NULL;
  der_params.pSharedData     = NULL;
  der_params.ulSharedDataLen = 0UL;
  der_params.pPublicData     = a_PeerPubKeyEcPoint;
  der_params.ulPublicDataLen = PeerPubKeyEcPointLen;

  /*Specifcy mechanism*/
  der_mech.mechanism      = CKM_ECDH1_DERIVE;
  der_mech.ulParameterLen = sizeof(der_params);
  der_mech.pParameter     = &der_params;

  (void)printf("--- Derive Key (Device) ---------------------------------------------------\r\n");
  (void)printf("--- ECDH Diffie-Hellman \r\n");
  tkms_buff2str(a_PeerPubKeyEcPoint, a_string, PeerPubKeyEcPointLen);
  (void)printf("--- Peer Public Key    [%s]\r\n", a_string);
  (void)printf("--- Length             [%d]\r\n", PeerPubKeyEcPointLen);

  /* Derive Key nit : template of the object to retrieve is passed to KMS */
  if (rv == CKR_OK)
  {
    rv = C_DeriveKey(session, &der_mech, h_obj_prv_key, &template_shrd_scrt, 1, &h_obj_shrd_scrt);
  }

  /* Retrieve DH secret computed by the device*/

  /* Fill in the template with attribute to retrieve */
  template_read.type       = CKA_VALUE;
  template_read.pValue     = a_DeviceSecretX;
  template_read.ulValueLen = sizeof(a_DeviceSecretX);

  /* Get attribute value*/
  if (rv == CKR_OK)
  {
    rv = C_GetAttributeValue(session,  h_obj_shrd_scrt, &template_read, 1);
  }

  DeviceSecretXLen = template_read.ulValueLen;

  (void)printf("--- Diffie-Hellman secret (Device) -----------------------------------------\r\n");
  tkms_buff2str(a_DeviceSecretX, a_string, DeviceSecretXLen);
  (void)printf("--- DH Secret [%s]\r\n", a_string);
  (void)printf("--- Length    [%d]\r\n", DeviceSecretXLen);


  /* Diffie-Hellman Key Derivation (Peer) ----------------------------------------------------*/
  /* This step simulates a remote server (peer) using the ephemeral public key                */
  /* generated using C_GenerateKeyPair in combination with its own private key to             */
  /* compute the same shared secret as the one generated using C_Derive                       */

  (void)printf("--- Derive Key (Peer) ---------------------------------------------------\r\n");
  (void)printf("--- ECDH Diffie-Hellman \r\n");
  tkms_buff2str(a_DeviceEphemeralPubKeyEcPoint, a_string, DeviceEphemeralPubKeyEcPointLen);
  (void)printf("--- Device Ephemeral Public Key [%s]\r\n", a_string);
  (void)printf("--- Length                      [%d]\r\n", DeviceEphemeralPubKeyEcPointLen);
  tkms_buff2str(a_PeerPrvKey, a_string, PeerPrvKeyEcPointLen);
  (void)printf("--- Peer Private Key            [%s]\r\n", a_string);
  (void)printf("--- Length                      [%d]\r\n", PeerPrvKeyEcPointLen);

  /* Initialize MBED contexts */
  mbedtls_ecdsa_context peer_ctx;
  mbedtls_ecp_point dev_eph_pub_key_ec_point;
  mbedtls_ecp_point peer_shared_secret_x_ec_point;

  /* Load key type */
  mbedtls_ecdsa_init(&peer_ctx);
  (void)mbedtls_ecp_group_load(&peer_ctx.grp, MBEDTLS_ECP_DP_SECP256R1);

  /* Import Device ephemeral public key generated with C_GenerateKeyPair */
  mbedtls_ecp_point_init(&dev_eph_pub_key_ec_point);
  (void)mbedtls_ecp_point_read_binary(&peer_ctx.grp, &dev_eph_pub_key_ec_point, &a_DeviceEphemeralPubKeyEcPoint[2],
                                      DeviceEphemeralPubKeyEcPointLen - 2U);

  /* Import Peer Private key */
  (void)mbedtls_mpi_read_binary(&peer_ctx.d, a_PeerPrvKey, PeerPrvKeyEcPointLen);
  (void)mbedtls_ecp_check_privkey(&peer_ctx.grp, &peer_ctx.d);

  /* Compute shared secret using Device Ephemeral Public Key & Peer Private key */
  mbedtls_ecp_point_init(&peer_shared_secret_x_ec_point);
  (void)mbedtls_ecp_mul(&peer_ctx.grp, &peer_shared_secret_x_ec_point, &peer_ctx.d, &dev_eph_pub_key_ec_point, NULL,
                        NULL);

  (void)mbedtls_mpi_write_binary(&peer_shared_secret_x_ec_point.X, a_PeerSecretX, PeerSecretXLen);

  (void)printf("--- Diffie-Hellman secret (Peer) -----------------------------------------\r\n");
  tkms_buff2str(a_PeerSecretX, a_string, PeerSecretXLen);
  (void)printf("--- DH Secret [%s]\r\n", a_string);
  (void)printf("--- Length    [%d]\r\n", PeerSecretXLen);

  /* Compare the secrets generated on device side and peer side -----------------------------*/

  if (memcmp(a_PeerSecretX, a_DeviceSecretX, PeerSecretXLen) != 0)
  {
    rv = CKR_GENERAL_ERROR;
  }

  /* Free memory */
  mbedtls_ecp_point_free(&dev_eph_pub_key_ec_point);
  mbedtls_ecdsa_free(&peer_ctx);
  mbedtls_ecp_point_free(&peer_shared_secret_x_ec_point);

  if (rv == CKR_OK)
  {
    (void)printf(">>> Diffie-Hellman derived secrets match\r\n");
  }
  else
  {
    (void)printf("XXX Diffie-Hellman derive secrets don't match\r\n");
  }

  /* Cleanup generated keys */
  (void)C_DestroyObject(session, h_obj_pub_key);
  (void)C_DestroyObject(session, h_obj_prv_key);
  (void)C_DestroyObject(session, h_obj_shrd_scrt);

  /* Close session with KMS */
  (void)C_CloseSession(session);

  return rv;
}
