/**
  ******************************************************************************
  * @file    kms_ext_token.h
  * @author  SMD/AME application teams
  * @brief   This file contains definitions for KMS External Token.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __KMS_EXT_TOKEN_H
#define __KMS_EXT_TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "pkcs11.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
CK_RV KMS_EXT_TOKEN_GetFunctionList(CK_FUNCTION_LIST_PTR_PTR ppFunctionList);


#ifdef __cplusplus
}
#endif

#endif /* __KMS_EXT_TOKEN_H */
