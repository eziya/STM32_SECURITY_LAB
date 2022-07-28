/**
  ******************************************************************************
  * @file    launcher.h
  * @brief   launder header file
  *          This file provides functions to launch application from bootloader
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */




/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LAUNCHER_H
#define __LAUNCHER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/**
  * @brief This function jump into a function to set sticky bit and to jump into application function
  * @param  address Function to jump into the BL code in case of exit_sticky usage
  * @param  magic number used by BL code in case of exit_sticky usage
  * @param  applicationVectorAddress Start address of the user application (slot0+offset: vectors)
  * @retval void
  */
void exit_sticky1(uint8_t *address, uint32_t magic, uint32_t applicationVectorAddress);

void exit_sticky(uint8_t *address);
void launch_application(uint32_t applicationVectorAddress, uint32_t exitFunctionAddress, uint32_t address,
                        uint32_t magic);

#ifdef __cplusplus
}
#endif

#endif /* __LAUNCHER_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
