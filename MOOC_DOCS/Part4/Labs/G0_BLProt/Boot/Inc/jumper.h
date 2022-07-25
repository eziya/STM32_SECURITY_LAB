/**
  ******************************************************************************
  * @file    Boot/Inc/jumper.h
  * @brief   Header for jumper.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __JUMPER_H
#define __JUMPER_H

/* Includes ------------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

#define FW_LIMIT 0x08008000
#define APP_META_DATA_ADD FW_LIMIT
#define APP_REGION_ROM_START (APP_META_DATA_ADD + 0x200)

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void jump_to_application(void);
#endif /* __JUMPER_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
