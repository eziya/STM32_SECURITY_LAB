/**
  ******************************************************************************
  * @file    Loader_Bin.c
  * @author  MCD Application Team
  * @brief   Include LOADER binary.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file in
  * the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
  
/* Includes ------------------------------------------------------------------*/
#include "string.h"

#if defined (__GNUC__)
asm(".section LOADER_Bin,\"a\";"
                             ".incbin \"./../../BLE_Ota/STM32CubeIDE/Release/Loader.bin\";"
);
#endif 
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

