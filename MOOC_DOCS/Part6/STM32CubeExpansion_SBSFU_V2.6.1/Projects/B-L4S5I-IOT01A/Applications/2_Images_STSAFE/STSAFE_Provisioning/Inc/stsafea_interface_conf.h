/**
  ******************************************************************************
  * @file    stsafea_interface_conf.h
  * @author  SMD/AME application teams
  * @brief   STSAFE-A Interface header file for application customization and use
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
#ifndef STSAFEA_INTERFACE_CONF_H
#define STSAFEA_INTERFACE_CONF_H


#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/** @addtogroup STSAFEA
  * @{
  */

/** @defgroup STSAFEA_INTERFACE
  * @{
  */

/** @defgroup STSAFEA_INTERFACE_Exported_Constants
  * @{
  */

/* Set to 1 if Host Keys must be retrieved from the MCU Flash. Set to 0 otherwise to use example keys */
#define USE_PRE_LOADED_HOST_KEYS        1U

/* Set to 1 in order to use Signature Sessions.
   Set to 0 to optimize code/memory size otherwise */
#define USE_SIGNATURE_SESSION                   0U

/* Include for MCU definition platform */
#define MCU_PLATFORM_INCLUDE                    "stm32l4xx.h"

#define NUM_READ_FLASH 16

/**
  * @}
  */

/** @defgroup STSAFEA_INTERFACE_Exported_Macros
  * @{
  */

/* To simplify a LVBuffer variable declaration and static data allocation.
 * LVBuffer Data field is in fact a simple pointer and needs to be correctly
 * initialized before to be used. This macro is used and can be moved at application level */
#define STSAFEA_STATIC_ALLOCATE_LV_BUFFER_VAR(LVBufferType, LVBufferVar, size)\
  LVBufferType LVBufferVar;                          \
  uint8_t data_##LVBufferVar [size] = {0};           \
  LVBufferVar.Length = size;                         \
  LVBufferVar.Data = data_##LVBufferVar;             \


/* To simplify a LVBuffer variable declaration and static data allocation.
 * LVBuffer Data field is in fact a simple pointer and needs to be correctly
 * initialized before to be used. This macro is used and can be moved at application level */
#define STSAFEA_DYNAMIC_ALLOCATE_LV_BUFFER_VAR(LVBufferType, LVBufferVar, size)\
  LVBufferType LVBufferVar;                          \
  LVBufferVar.Length = size;                         \
  LVBufferVar.Data = calloc( size, sizeof(uint8_t) );

/* To release the memomory dynamic allocated using the Macro here above.
   This macro is used and can be moved at application level */
#define STSAFEA_DYNAMIC_FREE_LV_BUFFER_VAR(LVBufferVar)\
  free( LVBufferVar.Data );

/**
  * @}
  */


/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* STSAFEA_INTERFACE_CONF_H */
