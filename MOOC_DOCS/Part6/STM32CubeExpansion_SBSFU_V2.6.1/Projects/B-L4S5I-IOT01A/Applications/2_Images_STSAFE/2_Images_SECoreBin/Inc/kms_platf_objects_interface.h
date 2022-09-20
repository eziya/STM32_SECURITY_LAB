/**
  ******************************************************************************
  * @file    kms_platf_objects_interface.h
  * @author  MCD Application Team
  * @brief   This file contains definitions for Key Management Services (KMS)
  *          module platform objects management configuration interface
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef KMS_PLATF_OBJECTS_INTERFACE_H
#define KMS_PLATF_OBJECTS_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

/* KMS exported Object handles for Secure Boot */
/* Device Identity Key and Certificate */
#define P11_OBJ_ID_DEV                            "1"                      /*!< Device ID */
#define P11_OBJ_LABEL_DEV_PRV_KEY                 "DevicePrivateKey"        /*!< Device Private Key */
#define P11_OBJ_LABEL_DEV_CRT                     "DeviceCertificate"       /*!< Device Certificate */

/* Ephemeral Key Pair */
#define P11_OBJ_ID_EPH                            "2"                       /*!< Ephemeral Key Pair ID */
#define P11_OBJ_LABEL_EPH_PRV_KEY                 "EphemeralPrivateKey"     /*!< Ephemeral Private Key */
#define P11_OBJ_LABEL_EPH_PUB_KEY                 "EphemeralPublicKey"      /*!< Ephemeral Public Key */

/* Key Pair with private key stored in slot 1 */
#define P11_OBJ_ID_DYN                            "3"                       /*!< Dynamic Key Pair ID */
#define P11_OBJ_LABEL_DYN_PRV_KEY                 "DynamicPrivateKey"     /*!< Dynamic Private Key */
#define P11_OBJ_LABEL_DYN_PUB_KEY                 "DynamicPublicKey"      /*!< Dynamic Public Key  */
#define P11_OBJ_LABEL_DYN_CRT                     "DynamicCertificate"       /*!< Dynamic Certificate */

/* Verification Public Key - 256 */
#define P11_OBJ_ID_VER_256                        "4"                       /*!< Verification Public Key 256 ID */
#define P11_OBJ_LABEL_VER_PUB_KEY_256             "VerificationPubKey256"   /*!< Verification Public Key 256 Label */

/* Verification Public Key - 384 */
#define P11_OBJ_ID_VER_384                        "5"                       /*!< Verification Public Key 384 ID */
#define P11_OBJ_LABEL_VER_PUB_KEY_384             "VerificationPubKey384"   /*!< Verification Public Key 384 Label */

/* Diffie Hellman Shared Secret */
#define P11_OBJ_ID_DH_SECRET_X                    "6"                       /*!< Diffie-Hellman Shared Secret ID */
#define P11_OBJ_LABEL_DH_SECRET_X                 "DHSharedSecretX"         /*!< Diffie-Hellman Shared Secret Label */

/* Device CA Certificate */
#define P11_OBJ_ID_DEV_CA_CRT                     "7"                       /*!< Device CA Certificate ID */
#define P11_OBJ_LABEL_DEV_CA_CRT                  "DeviceCACertificate"     /*!< Device CA Certificate Label */

/* SBSFU Root CA Certificate */
#define P11_OBJ_ID_SBSFU_ROOT_CA_CRT              "8"                       /*!< SBSFU Root CA Certificate ID */
#define P11_OBJ_LABEL_SBSFU_ROOT_CA_CRT           "SBSFURootCACertificate"  /*!< SBSFU Root CA Certificate Label */

/* OEM Interm CA Certificate */
#define P11_OBJ_ID_OEM_INTM_CA_CRT                "9"                       /*!< OEM Interm CA ID */
#define P11_OBJ_LABEL_OEM_INTM_CA_CRT             "OEMIntermCACertificate"  /*!< OEM Interm CA Certificate */


/* KMS exported Object handles for tKMS test application */
#define KMS_KEY_RSA2048_OBJECT_HANDLE                 ((CK_OBJECT_HANDLE)21)
#define KMS_KEY_AES128_OBJECT_HANDLE                  ((CK_OBJECT_HANDLE)3)
#define KMS_KEY_AES256_OBJECT_HANDLE                  ((CK_OBJECT_HANDLE)4)
#define KMS_KEY_AES128_DERIVABLE_OBJECT_HANDLE        ((CK_OBJECT_HANDLE)22)

#ifdef __cplusplus
}
#endif

#endif /* KMS_PLATF_OBJECTS_INTERFACE_H */
