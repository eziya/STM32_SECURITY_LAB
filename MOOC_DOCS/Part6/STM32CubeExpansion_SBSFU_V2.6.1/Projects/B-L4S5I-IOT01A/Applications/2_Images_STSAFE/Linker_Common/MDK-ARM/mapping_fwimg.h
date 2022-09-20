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

/* Slots  must be aligned on 8192 bytes (0x2000) */

/* swap  (32 kbytes) */
#define SWAP_START                   0x08110000
#define SWAP_END                     0x08117FFF

/* Active slot #1  (896 kbytes) */
#define SLOT_ACTIVE_1_HEADER         0x0811E000
#define SLOT_ACTIVE_1_START          0x08120000
#define SLOT_ACTIVE_1_END            0x081FFFFF

/* Dwl slot #1  (896 kbytes) */
#define SLOT_DWL_1_START             0x08030000
#define SLOT_DWL_1_END               0x0810FFFF

/* Slots not configured */
#define SLOT_ACTIVE_2_HEADER         0x00000000
#define SLOT_ACTIVE_2_START          0x00000000
#define SLOT_ACTIVE_2_END            0x00000000
#define SLOT_ACTIVE_3_HEADER         0x00000000
#define SLOT_ACTIVE_3_START          0x00000000
#define SLOT_ACTIVE_3_END            0x00000000
#define SLOT_DWL_2_START             0x00000000
#define SLOT_DWL_2_END               0x00000000
#define SLOT_DWL_3_START             0x00000000
#define SLOT_DWL_3_END               0x00000000

#endif
