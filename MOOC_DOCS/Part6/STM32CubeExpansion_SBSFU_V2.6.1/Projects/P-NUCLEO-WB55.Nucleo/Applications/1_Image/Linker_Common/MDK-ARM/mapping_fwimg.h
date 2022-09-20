/**
  ******************************************************************************
  * @file    mapping_fwimg.h
  * @author  MCD Application Team
  * @brief   This file contains definitions for firmware images mapping
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MAPPING_FWIMG_H
#define MAPPING_FWIMG_H

/* Slots  must be aligned on 4096 bytes (0x1000) */

/* Active slot #1 : 424 Kbytes */
/* Header located outside download area for anti-rollback mechanism
   ==> In case of wireless stack update the download area from 
   SLOT_ACTIVE_1_START to SLOT_ACTIVE_1_END is erased
*/
#define SLOT_ACTIVE_1_HEADER         0x08015000
#define SLOT_ACTIVE_1_START          0x08016000
#define SLOT_ACTIVE_1_END            0x0807FFFF

/* Slots not configured */
#define SLOT_ACTIVE_2_HEADER         0x00000000
#define SLOT_ACTIVE_2_START          0x00000000
#define SLOT_ACTIVE_2_END            0x00000000
#define SLOT_ACTIVE_3_HEADER         0x00000000
#define SLOT_ACTIVE_3_START          0x00000000
#define SLOT_ACTIVE_3_END            0x00000000

#endif
