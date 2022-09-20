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

/* Temporary mapping provided as example, as no external flash available on this board
   ===================================================================================
   - Dwl slot should be mapped in external flash
   - Backup slot should be mapped in external flash
   - Active slot should be extended to all the internal flash free
*/

/* Active slot #1  (424 kbytes) */
#define SLOT_ACTIVE_1_START          0x08017000
#define SLOT_ACTIVE_1_END            0x0807FFFF
#define SLOT_ACTIVE_1_HEADER         0x08016000

/* Dwl slot #1  (448 KBytes, that is to say seven 64-KBytes blocks) */
#define SLOT_DWL_1_START             0x90000000
#define SLOT_DWL_1_END               0x9006FFFF

/* Backup (448 Kbytes, that is to say seven 64-KBytes blocks) */
#define BACKUP_START                 0x90070000
#define BACKUP_END                   0x900DFFFF

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

/* SWAP installation feature not allowed in this example in order to keep dwl slot encrypted */
#define SWAP_START                   0x00000000
#define SWAP_END                     0x00000000

#endif
