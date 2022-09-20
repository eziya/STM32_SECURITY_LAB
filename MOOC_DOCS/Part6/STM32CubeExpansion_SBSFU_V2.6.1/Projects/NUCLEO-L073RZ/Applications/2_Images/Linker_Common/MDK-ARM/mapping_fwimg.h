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

/* swap  (4 kbytes) */
#define SWAP_START                   0x08029000
#define SWAP_END                     0x08029FFF

/* Active slot #1  (36 kbytes) */
#define SLOT_ACTIVE_1_START          0x08020000
#define SLOT_ACTIVE_1_END            0x08028FFF
#define SLOT_ACTIVE_1_HEADER         SLOT_ACTIVE_1_START

/* Dwl slot #1  (36 kbytes) */
#define SLOT_DWL_1_START             0x0800F000
#define SLOT_DWL_1_END               0x08017FFF

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
