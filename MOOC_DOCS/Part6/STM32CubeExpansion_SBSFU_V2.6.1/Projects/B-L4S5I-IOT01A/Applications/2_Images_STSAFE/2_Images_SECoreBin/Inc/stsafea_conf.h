/**
  ******************************************************************************
  * @file    stsafea_conf.h
  * @author  SMD/AME application teams
  * @brief   STSAFE-A1XX Middleware configuration file
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
#ifndef STSAFEA_CONF_H
#define STSAFEA_CONF_H


#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/** @addtogroup STSAFEA
  * @{
  */

/** @defgroup STSAFEA_CONFIG
  * @{
  */

/** @defgroup STSAFEA_CONFIG_Exported_Constants
  * @{
  */

/* Set to 1 to optimize RAM usage. If set to 1 the StSafeA_Handle_t.InOutBuffer used through the Middleware APIs is shared
   with the application between each command & response. It means that every time the MW API returns a TLVBuffer pointer,
   it returns in fact a pointer to the shared StSafeA_Handle_t.InOutBuffer.
   As consequence the user shall copy data from given pointer into variable defined by himself in case data need to be stored.
   If set to 0 the user must specifically allocate (statically or dynamically) a right sized buffer to be passed as parameter
   to the Middleware command API */
#define STSAFEA_USE_OPTIMIZATION_SHARED_RAM             1U

/* Set to 1 to optimize RAM/Flash usage, when Host MAC and encryption features are not required (eg. Authentication use case) */
#define STSAFEA_USE_OPTIMIZATION_NO_HOST_MAC_ENCRYPT    0U

/* Set to 1 to have CRC16_CCIT Table already calculated and placed in Flash as const. Set to zero to dynamically calculate it in RAM */
#define STSAFEA_USE_OPTIMIZATION_CRC_TABLE              1U

/* Set to 1  to expanse the "assert_param" macro in the STSAFE middleware code   */
#define STSAFEA_USE_FULL_ASSERT                         0U


/* Weak function definition. A different __weak function definition might be needed for different platforms */
#if  defined (__GNUC__)
  #ifndef __weak
    #define __weak   __attribute__ ((weak))
  #endif /* __weak */
#endif /* __GNUC__ */

/**
  * @}
  */

/** @defgroup STSAFEA_CONFIG_Exported_Macros
  * @{
  */


/* Enable big-little endian bytes swap optimization using ARM Instructions. Compiler dependent*/
/*------------------ ICC Compiler ----------------------*/
#if defined (__ICCARM__)
    #include <intrinsics.h> /* Included for optimized ARM endianness conversion */
    #define SWAP2BYTES(x)    __REV16(x)
    #define SWAP4BYTES(x)    __REV(x)
#endif /* __ICCARM__ */



#if (STSAFEA_USE_FULL_ASSERT)
  #include <stdint.h>
/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function
  *         which reports the name of the source file and the source
  *         line number of the call that failed.
  *         If expr is true, it returns no value.
  * @retval None
  */
  #define stsafea_assert_param(expr) ((expr) ? (void)0U : stsafea_assert_failed((uint8_t *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void stsafea_assert_failed(uint8_t* file, uint32_t line);
#else
  #define stsafea_assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */

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

#endif /* STSAFEA_CONF_H */

