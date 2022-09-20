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

/* swap  (128 kbytes) */
#define SWAP_START                   0x90800000
#define SWAP_END                     0x9081FFFF

/* Active slot #1 in external memory : 4 Mbytes
   Header located into internal flash to be protected by secure area but
   out of WRP protection areas (SBSFU + standalone loader)
 */
#define SLOT_ACTIVE_1_HEADER         0x08018000
#define SLOT_ACTIVE_1_START          0x90000000
#define SLOT_ACTIVE_1_END            0x903FFFFF

/* Dwl slot #1  (4 Mbytes) */
#define SLOT_DWL_1_START             0x90400000
#define SLOT_DWL_1_END               0x907FFFFF

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
