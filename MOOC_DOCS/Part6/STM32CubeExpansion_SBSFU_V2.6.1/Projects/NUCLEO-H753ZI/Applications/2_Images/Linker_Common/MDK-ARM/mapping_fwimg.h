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

/* Slots  must be aligned on sector size */

/* Swap sector in sector 0 of Bank2 : 128 kbytes */  
#define SWAP_START                   0x08100000
#define SWAP_END                     0x0811FFFF

/* Active slot #1 in Sector 1 to 7 of Bank1; 7 x 128 kbytes */
#define SLOT_ACTIVE_1_START          0x08020000
#define SLOT_ACTIVE_1_END            0x080FFFFF
#define SLOT_ACTIVE_1_HEADER         SLOT_ACTIVE_1_START

/* Dwl slot #1 in sector 1 to 7 of Bank2 : 7 * 128 kbytes */
#define SLOT_DWL_1_START             0x08120000
#define SLOT_DWL_1_END               0x081FFFFF

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
