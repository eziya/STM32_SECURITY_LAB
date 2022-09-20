/**
  ******************************************************************************
  * @file    stsafe_platf_objects.c
  * @author  SMD/AME application teams
  * @brief   This file contains definitions for STSAFE Platform Objects and
  *          related methods.
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
#include <stdbool.h>
#include <stdint.h>
#include "kms.h"
#include "kms_ext_token.h"
#include "kms_platf_objects_interface.h"

#include "string.h"
#include "kms_nvm_storage.h"
#include "kms_platf_objects.h"
#include "stsafeaxxx.h"
#include "stsafea_core.h"
#include "stsafea_conf.h"
#include "stsafe_platf_objects.h"
#define STSAFE_PLATF_OBJECTS_C
#include "stsafe_platf_objects_config.h"
#undef STSAFE_PLATF_OBJECTS_C

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
static uint32_t ulMinObjId, ulMaxObjId;  /*!< Min and Max Objects id */
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

/**
  * @brief  Initialize the embedded objects.
  * @param  None.
  * @retval CK_RV CKR_GENERAL_ERROR, CKR_OK.
  */
CK_RV STSAFE_OBJ_Init(void)
{
  /* Read the available external token slots from the platform */
  /* Note: this returns the available objects handle from KMS - actual number of
    * objects depends on platform configuration */
  KMS_PlatfObjects_ExtTokenStaticRange(&ulMinObjId, &ulMaxObjId);

  /*Check if platform configuration is ok*/
  if (ulMaxObjId < ulMinObjId + STSAFE_P11_NUMBER_OBJECTS)
  {
    return CKR_GENERAL_ERROR;

  }
  else
  {
    /* The only actually used objects*/
    ulMaxObjId = ulMinObjId + STSAFE_P11_NUMBER_OBJECTS;
  }

  return CKR_OK;
}

/**
  * @brief  Get template pointer from Object Handle.
  * @param  ppAttribute  gets the pointer to the object template.
  * @param  pulCount     gets the number of attrivutes of the template.
  * @param  hObject      the object's handle.
  * @retval CK_RV CKR_ARGUMENTS_BAD, CKR_OK.
  */
CK_RV STSAFE_OBJ_GetTemplateFromObjectHandle(CK_ATTRIBUTE_PTR *ppAttribute, CK_ULONG_PTR pulCount,
                                             CK_OBJECT_HANDLE hObject)
{
  /* Check input parameters */
  if ((hObject < ulMinObjId) || (hObject > ulMaxObjId) || (pulCount == NULL) || (ppAttribute == NULL))
  {
    return CKR_ARGUMENTS_BAD;
  }

  /*Check if the object is present*/
  if (pSTSAFE_OBJ_List[hObject - ulMinObjId] != NULL)
  {
    /* Assign output*/
    *ppAttribute = pSTSAFE_OBJ_List[hObject - ulMinObjId]->pTemplate;
    *pulCount = pSTSAFE_OBJ_List[hObject - ulMinObjId]->ulCount;
  }
  else
  {
    *ppAttribute = NULL;
    *pulCount = 0;
  }

  return CKR_OK;
}

/**
  * @brief  Get an attribute from a template.
  * @param  ppAttribute     gets the pointer to the attribute.
  * @param  Type     the attribute type to extract.
  * @param  pTemplate    the template
  * @param  ulCount    size of the template.
  * @retval CK_RV CKR_ARGUMENTS_BAD, CKR_GENERAL_ERROR, CKR_OK.
  */
CK_RV STSAFE_OBJ_FindAttributeInTemplate(CK_ATTRIBUTE_PTR *ppAttribute, CK_ATTRIBUTE_TYPE Type,
                                         CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount)
{
  uint32_t i;

  /*Check input parameters*/
  if ((pTemplate == NULL)  || (ulCount == 0))
  {
    return CKR_ARGUMENTS_BAD;
  }

  /* Go through the list of attributes of the template*/
  for (i = 0; i < ulCount; i++)
  {
    /*Look for Type param*/
    if (pTemplate[i].type == Type)
    {
      /* if param not null */
      if (ppAttribute != NULL)
      {
        *ppAttribute = &pTemplate[i];
      }

      return CKR_OK;
    }
  }

  return CKR_GENERAL_ERROR;
}

/**
  * @brief  Filter a list of attributes from a template.
  * @note   Useful to obtain a subset of attributes.
  * @param  pTemplateOut    the template in output
  * @param  ulCountOut    size of the template in output.
  * @param  pTemplateIn      the template in input.
  * @param  ulCountIn        size of the template in input.
  * @param  pTemplateFilter  the template with the list of attributes to remove from input.
  * @param  ulCountFilter    size of the filter template .
  * @retval CK_RV CKR_ARGUMENTS_BAD, CKR_OK.
  */
CK_RV STSAFE_OBJ_FilterTemplate(CK_ATTRIBUTE_PTR pTemplateOut, CK_ULONG_PTR pulCountOut, CK_ATTRIBUTE_PTR pTemplateIn,
                                CK_ULONG ulCountIn, CK_ATTRIBUTE_PTR pTemplateFilter, CK_ULONG ulCountFilter)
{
  uint32_t i;
  uint32_t ul_out_count;

  /*Check input parameters*/
  if ((pTemplateOut == NULL)  || (pulCountOut == NULL) || (pTemplateIn == NULL) || (ulCountIn == 0UL))
  {
    return CKR_ARGUMENTS_BAD;
  }

  /* Init*/
  ul_out_count = 0;


  for (i = 0; i < ulCountIn; i++)
  {
    /* Check if the attribute is not in the list of objects to filter */
    if (STSAFE_OBJ_FindAttributeInTemplate(NULL, pTemplateIn[i].type, pTemplateFilter, ulCountFilter) != CKR_OK)
    {
      /*if the attribute is not in the filter list Copy the attribute out*/
      (void)memcpy(&pTemplateOut[ul_out_count], &pTemplateIn[i], sizeof(CK_ATTRIBUTE));

      /*Increase the size of output template*/
      ul_out_count++;
    }
  }

  /*Assingn the output template len*/
  *pulCountOut = ul_out_count;

  return CKR_OK;
}

/**
  * @brief  Get Object handle range for STSAFE.
  * @param  phObjectMin    gets the object handle min.
  * @param  phObjectMax    gets the object handle max.
  * @retval CK_RV CKR_ARGUMENTS_BAD, CKR_OK.
  */
CK_RV STSAFE_OBJ_GetObjectRange(CK_OBJECT_HANDLE_PTR phObjectMin, CK_OBJECT_HANDLE_PTR phObjectMax)
{
  if ((phObjectMin == NULL) || (phObjectMax == NULL))
  {
    return CKR_ARGUMENTS_BAD;
  }

  *phObjectMin = ulMinObjId;
  *phObjectMax = ulMaxObjId;

  return CKR_OK;
}


/**
  * @brief  Get attribute Value from an embedded object.
  * @param  pValue     gets the attribute value.
  * @param  pulValueLen     gets the attribute value len.
  * @param  pAttribute     attribute from where to extract the value.
  * @retval CK_RV CKR_ARGUMENTS_BAD, CKR_OK.
  */
CK_RV STSAFE_OBJ_GetAttributeValue(CK_BYTE_PTR pValue, CK_ULONG_PTR pulValueLen, CK_ATTRIBUTE_PTR pAttribute)
{

  /*Check input parameters*/
  if ((pAttribute == NULL) || (pulValueLen == NULL))
  {
    return CKR_ARGUMENTS_BAD;
  }

  /*Copy */
  if (pValue != NULL)
  {
    (void)memcpy(pValue, pAttribute->pValue, pAttribute->ulValueLen);
  }

  /* Copy the Value Len out  */
  *pulValueLen =  pAttribute->ulValueLen;

  return CKR_OK;

}


/**
  * @brief  Update attribute Value of an embedded object.
  * Note: to be used only to update value of a volatile attribute.
  * @param  pAttribute     attribute where to write the value.
  * @param  pValue         the new attribute value.
  * @param  ulValueLen     the attribute value len.
  * @retval CK_RV CKR_ARGUMENTS_BAD, CKR_OK.
  */
CK_RV STSAFE_OBJ_SetAttributeValue(CK_ATTRIBUTE_PTR pAttribute, CK_BYTE_PTR pValue, CK_ULONG ulValueLen)
{
  /*Check input parameters*/
  if ((pAttribute == NULL) || (ulValueLen == 0U) || (pValue == NULL))
  {
    return CKR_ARGUMENTS_BAD;
  }

  /*Update the value*/
  /*Note: no check on the destination pointer. If it is in FLASH this will cause an error*/
  (void)memcpy(pAttribute->pValue, pValue, ulValueLen);

  return CKR_OK;

}

/**
  * @brief  Compare a template in input with a reference template.
  * @note   Input template could have a subset of attributes of the reference.
  *       For each attribute in input, the corresponding value in the reference template is checked.
  *       An error is returned if an attribute is not present in the reference template, or if the corresponding values
  *       don't match.
  * @param  pTemplate         the template in output
  * @param  ulCount           size of the template in output.
  * @param  pTemplateRef      the template in input.
  * @param  ulCountRef      size of the template in input.
  * @retval CK_RV CKR_ARGUMENTS_BAD, CKR_GENERAL_ERROR, CKR_OK.
  */
CK_RV STSAFE_OBJ_CompareTemplates(CK_ATTRIBUTE_PTR pTemplate, CK_ULONG ulCount, CK_ATTRIBUTE_PTR pTemplateRef,
                                  CK_ULONG ulCountRef)
{
  CK_ULONG i;
  CK_ATTRIBUTE_PTR p_attribute;

  /* Check input parameters */
  if ((pTemplate == NULL) || (ulCount == 0UL) || (pTemplateRef == NULL) || (ulCountRef == 0UL) ||
      (ulCount > ulCountRef))
  {
    return CKR_ARGUMENTS_BAD;
  }

  /*Go through the attributes of the input template*/
  for (i = 0; i < ulCount; i ++)
  {
    /* Check for the i-th attribute type*/
    if (STSAFE_OBJ_FindAttributeInTemplate(&p_attribute, pTemplate[i].type, pTemplateRef, ulCountRef) != CKR_OK)
    {
      /*Input template has an attribute Type that is not present in the reference*/
      return CKR_ATTRIBUTE_TYPE_INVALID;
    }

    /* CKA ATTRIBUTE attribute found in the embedded object - now compare the two values */
    if (memcmp(p_attribute->pValue, pTemplate[i].pValue, p_attribute->ulValueLen) != 0U)
    {
      /* Attribute Value don't match*/
      return CKR_ATTRIBUTE_VALUE_INVALID;
    }
  }

  return CKR_OK;
}
