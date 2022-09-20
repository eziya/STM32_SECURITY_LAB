/**
  ******************************************************************************
  * @file    stsafe_platf_objects.h
  * @author  SMD/AME application teams
  * @brief   This file contains definitions for STSAFE Platform Objects.
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
#ifndef STSAFE_PLATF_OBJECTS_H
#define STSAFE_PLATF_OBJECTS_H

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
CK_RV STSAFE_OBJ_Init(void);
CK_RV STSAFE_OBJ_GetTemplateFromObjectHandle(CK_ATTRIBUTE_PTR *ppAttribute, CK_ULONG_PTR pulCount,
                                             CK_OBJECT_HANDLE hObject);
CK_RV STSAFE_OBJ_FindAttributeInTemplate(CK_ATTRIBUTE_PTR *ppAttribute, CK_ATTRIBUTE_TYPE Type,
                                         CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount);
CK_RV STSAFE_OBJ_FilterTemplate(CK_ATTRIBUTE_PTR pTemplateOut, CK_ULONG_PTR pulCountOut, CK_ATTRIBUTE_PTR pTemplateIn,
                                CK_ULONG ulCountIn, CK_ATTRIBUTE_PTR pTemplateFilter, CK_ULONG ulCountFilter);
CK_RV STSAFE_OBJ_GetAttributeValue(CK_BYTE_PTR pValue, CK_ULONG_PTR pulValueLen, CK_ATTRIBUTE_PTR pAttribute);
CK_RV STSAFE_OBJ_SetAttributeValue(CK_ATTRIBUTE_PTR pAttribute, CK_BYTE_PTR pValue, CK_ULONG ulValueLen);
CK_RV STSAFE_OBJ_CompareTemplates(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount, CK_ATTRIBUTE_PTR pTemplateRef,
                                  CK_ULONG ulCountRef);
CK_RV STSAFE_OBJ_GetObjectRange(CK_OBJECT_HANDLE_PTR phObjectMin, CK_OBJECT_HANDLE_PTR phObjectMax);

#ifdef __cplusplus
}
#endif

#endif /*STSAFE_PLATF_OBJECTS_H */
