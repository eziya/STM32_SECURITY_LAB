/**
  ******************************************************************************
  * @file    fwauth.c
  * @brief   Example of fw signature check.
  *          This file provides set of firmware functions to manage Com
  *          functionalities.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fwauth.h"
#include "crypto.h"
#include "ecc_pub_key.h"
#include <stdio.h>
#include <string.h>

/* Private function prototypes -----------------------------------------------*/
static int32_t SignatureVerify(const uint8_t *pSignature, 
                            const uint8_t *MessageDigest, const int32_t MessageDigestLength);

int32_t STM32_SHA256_HASH_DigestCompute(uint8_t* InputMessage, uint32_t InputMessageLength,
                                        uint8_t *MessageDigest, int32_t* MessageDigestLength);

void Fatal_Error_Handler(void)
{
  printf("\r\nFatal error! Enter endless loop!\r\n");
  while(1){};
}

										
/**
  * @brief  SHA256 HASH digest compute example.
  * @param  InputMessage: pointer to input message to be hashed.
  * @param  InputMessageLength: input data message length in byte.
  * @param  MessageDigest: pointer to output parameter that will handle message digest
  * @param  MessageDigestLength: pointer to output digest length.
  * @retval error status: can be HASH_SUCCESS if success or one of
  *         HASH_ERR_BAD_PARAMETER, HASH_ERR_BAD_CONTEXT,
  *         HASH_ERR_BAD_OPERATION if error occured.
  */
int32_t STM32_SHA256_HASH_DigestCompute(uint8_t* InputMessage, uint32_t InputMessageLength,
                                        uint8_t *MessageDigest, int32_t* MessageDigestLength)
{
  SHA256ctx_stt P_pSHA256ctx;
  uint32_t error_status = HASH_SUCCESS; 

  /* Set the size of the desired hash digest */
  P_pSHA256ctx.mTagSize = CRL_SHA256_SIZE;

  /* Set flag field to default value */
  P_pSHA256ctx.mFlags = E_HASH_DEFAULT;

  error_status = SHA256_Init(&P_pSHA256ctx);

  /* check for initialization errors */
  if (error_status == HASH_SUCCESS)
  {
    /* Add data to be hashed */
    error_status = SHA256_Append(&P_pSHA256ctx,
                                 InputMessage,
                                 InputMessageLength);

    if (error_status == HASH_SUCCESS)
    {
      /* retrieve */
      error_status = SHA256_Finish(&P_pSHA256ctx, MessageDigest, MessageDigestLength);
    }
  }

  return error_status;
}


int32_t SignatureVerify(const uint8_t *pSignature,
                     const uint8_t *MessageDigest, const int32_t MessageDigestLength)
{
  int32_t status = -1;
  uint8_t *pKey = &SIGN_ECC_PUB_KEY[0];
  static uint8_t preallocated_buffer[2048];
  
    static const uint8_t P_256_a[] __attribute__((aligned(4))) =
  {
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC
  };
  static const uint8_t P_256_b[] __attribute__((aligned(4))) =
  {
    0x5a, 0xc6, 0x35, 0xd8, 0xaa, 0x3a, 0x93, 0xe7, 0xb3, 0xeb, 0xbd, 0x55, 0x76,
    0x98, 0x86, 0xbc, 0x65, 0x1d, 0x06, 0xb0, 0xcc, 0x53, 0xb0, 0xf6, 0x3b, 0xce,
    0x3c, 0x3e, 0x27, 0xd2, 0x60, 0x4b
  };
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
  
  membuf_stt Crypto_Buffer;
  /* Proceed with ECC signature generation */
  EC_stt EC_st;
  /* Structure that will contain the public key, please note that the public key
    * is just a point on the curve, hence the name ECpoint_stt
    */
  ECpoint_stt *PubKey = NULL;
  /* Structure context used to call the ECDSAverify */
  ECDSAverifyCtx_stt verctx;
  const uint8_t *pSign_r;
  const uint8_t *pSign_s;
  
  uint8_t *pSign = (uint8_t*)pSignature;  /* ECCDSA signed SHA256 of the FW metadata) */
  const uint8_t *pPub_x;
  const uint8_t *pPub_y;
  ECDSAsignature_stt *sign = NULL;
  
  if (pSignature == NULL || MessageDigest==NULL) goto ERROR;
  
  /* Retrieve the ECC Public Key */
  pKey = &(SIGN_ECC_PUB_KEY[0]);
 
  /* Set the local variables dealing with the public key */
  pPub_x  =  pKey;
  pPub_y = (uint8_t *)(pKey + 32);

  /* signature to be verified with r and s components */
  pSign_r = pSign;
  pSign_s = (uint8_t *)(pSign + 32); 

  /* We prepare the memory buffer structure */
  Crypto_Buffer.pmBuf =  preallocated_buffer;
  Crypto_Buffer.mUsed = 0;
  Crypto_Buffer.mSize = (int16_t) sizeof(preallocated_buffer);
  EC_st.pmA = P_256_a;
  EC_st.pmB = P_256_b;
  EC_st.pmP = P_256_p;
  EC_st.pmN = P_256_n;
  EC_st.pmGx = P_256_Gx;
  EC_st.pmGy = P_256_Gy;
  EC_st.mAsize = (int32_t)sizeof(P_256_a);
  EC_st.mBsize = (int32_t)sizeof(P_256_b);
  EC_st.mNsize = (int32_t)sizeof(P_256_n);
  EC_st.mPsize = (int32_t)sizeof(P_256_p);
  EC_st.mGxsize = (int32_t)sizeof(P_256_Gx);
  EC_st.mGysize = (int32_t)sizeof(P_256_Gy);

  status = ECCinitEC(&EC_st, &Crypto_Buffer);

  if (status == ECC_SUCCESS)
  {
    status = ECCinitPoint(&PubKey, &EC_st, &Crypto_Buffer);
  }

  if (status == ECC_SUCCESS)
  {
    /* Point is initialized, now import the public key */
    (void)ECCsetPointCoordinate(PubKey, E_ECC_POINT_COORDINATE_X, pPub_x, 32);
    (void)ECCsetPointCoordinate(PubKey, E_ECC_POINT_COORDINATE_Y, pPub_y, 32);
    /* Try to validate the Public Key. */
    status = ECCvalidatePubKey(PubKey, &EC_st, &Crypto_Buffer);
  }

  if (status == ECC_SUCCESS)
  {
    /* Public Key is validated, Initialize the signature object */
    status = ECDSAinitSign(&sign, &EC_st, &Crypto_Buffer);
  }

  if (status == ECC_SUCCESS)
  {
    /* Import the signature values */
    (void)ECDSAsetSignature(sign, E_ECDSA_SIGNATURE_R_VALUE, pSign_r, 32);
    (void)ECDSAsetSignature(sign, E_ECDSA_SIGNATURE_S_VALUE, pSign_s, 32);

    /* Prepare the structure for the ECDSA signature verification */
    verctx.pmEC = &EC_st;
    verctx.pmPubKey = PubKey;

    /* Verify it */
    status = ECDSAverify(MessageDigest, MessageDigestLength, sign, &verctx, &Crypto_Buffer);
    
    /* release ressource ...*/
    (void)ECDSAfreeSign(&sign, &Crypto_Buffer);
    (void)ECCfreePoint(&PubKey, &Crypto_Buffer);
    (void)ECCfreeEC(&EC_st, &Crypto_Buffer);
    
    if (status == SIGNATURE_VALID)
    {
      printf("\r\n FW Signature Check PASS!\r\n");    
    }
    else
    {
      printf("\r\n FW Signature Check FAIL!\r\n");    
      Fatal_Error_Handler();
    }
  }
  else
  {
    printf("\r\n FW Signature Check FAIL!\r\n");    
    Fatal_Error_Handler();
  }
  
  return status;
  
ERROR:
  status = -1;
  Fatal_Error_Handler();
  return status;
}

static void print_buffer(char *name, uint8_t * buf, uint32_t size)
{
  int i;
  if (name != NULL )
  {
    printf("\r\n*** %s *** \r\n", name);
  }
  else
  {
    printf("\r\n");
  }
  for ( i = 0; i < size; i++ )
  {
    printf("%02x",buf[i]);
    if ((i+1)%8 == 0) printf("  ");
    if ((i+1)%16 == 0) printf("\r\n");
  }
  printf("\r\n");
}

int32_t FW_Verify(void)
{
  FW_Meta_t *pFWMeta = (FW_Meta_t *)FW_META_DATA_ADD;
  uint32_t hash_status = 0; 
  int32_t sig_status = -1;
  uint8_t MetaDigest[FW_HASH_LEN] = {0};
  int32_t MetaDigestLength = FW_HASH_LEN;
  
  /* enable CRC to allow cryptolib to work */ 
  __CRC_CLK_ENABLE();
  
  printf("\r\nStart APP FW Verification...\r\n");
  printf("FW Meta data @0x%08x:\r\n", FW_META_DATA_ADD);
  printf("FW Magic: 0x%08x\r\n", pFWMeta->FWMagic);
  printf("FW Size: 0x%08x\r\n", pFWMeta->FwSize);
  printf("FW Version: 0x%08x\r\n", pFWMeta->FwVersion);
  print_buffer("FW HASH", pFWMeta->FwTag, FW_HASH_LEN);
  print_buffer("META DATA HASH", pFWMeta->MetaTag, FW_HASH_LEN);
  print_buffer("META DATA SIGNATURE", pFWMeta->MetaSig, FW_META_SIG_LEN);
  
  /* 1. Check the magic number of fw meta data */
  printf("\r\n Check FW Magic\r\n");
  if ( pFWMeta->FWMagic != FW_MAGIC )
  {
    goto ERROR;
  }
  printf(" FW Meta data Magic check OK!\r\n");
      
  /* 2. Verify fw meta data */  
  printf("\r\n Check FW Meta data\r\n");
  /* 2.1 Compute meta data hash */
  printf("\r\n Check FW Meta data hash\r\n");
  hash_status = STM32_SHA256_HASH_DigestCompute((uint8_t*)pFWMeta, sizeof(FW_Meta_t)- FW_HASH_LEN - FW_META_SIG_LEN,
                                        &MetaDigest[0], &MetaDigestLength);
  if ((hash_status == HASH_SUCCESS) && (MetaDigestLength == FW_HASH_LEN))
  {
    /* 2.2 Compare meta data hash with the stored hash */
    int i;
    print_buffer("Computed META DATA HASH", MetaDigest, FW_HASH_LEN);
  
    for (i = 0; i< FW_HASH_LEN; i++)
    {
      if ( pFWMeta->MetaTag[i] != MetaDigest[i] )
      {
        goto ERROR;
      }
    }      
    printf(" FW Meta data Hash check OK!\r\n");
    
    /* 2.3 Verify meta data signature*/
    print_buffer("META HASH", MetaDigest, MetaDigestLength);
    printf("\r\n Check FW Meta data signature\r\n");
    sig_status = SignatureVerify(&pFWMeta->MetaSig[0], &pFWMeta->MetaTag[0], MetaDigestLength);
  }
  else
  {    
    goto ERROR;
  }
  
  /* 3. Compute fw hash and compare with fw tag in meta data */
  printf("\r\n Check FW Hash\r\n");
  if (sig_status == SIGNATURE_VALID )
  {
    printf(" FW Meta data Signature check OK!\r\n");
    
    /* 3.1 compute the fw hash */
    hash_status = STM32_SHA256_HASH_DigestCompute((uint8_t*)FW_ADD, pFWMeta->FwSize,
                                        &MetaDigest[0], &MetaDigestLength);
    if ((hash_status == HASH_SUCCESS) && (MetaDigestLength == FW_HASH_LEN))
    {
      int i;
      print_buffer("FW HASH", MetaDigest, MetaDigestLength);
      /* 3.2 Compare fw hash with fw tag */
      for (i = 0; i< FW_HASH_LEN; i++)
      {
        if ( pFWMeta->FwTag[i] != MetaDigest[i] )
        {
          goto ERROR;
        }
      } 
      printf(" FW Hash check OK!\r\n");
    }
    else
    {      
      goto ERROR;
    }
  }
  
  printf("\r\n\r\n");
  return sig_status;
  
ERROR:
  sig_status = -1;    
  Fatal_Error_Handler();
    
  return sig_status;
}
