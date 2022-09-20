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

/* Slots  must be aligned on 2048 bytes (0x800) */

/* Active slot #1  (472 kbytes) */
/* Header does not start at the beginning of the sector to ensure the overlap constraint between 
   firewall NVDATA section and firewall CODE section. 
   This constraint is relevant only for products with dual bank and firewall isolation */
#define SLOT_ACTIVE_1_HEADER         0x08089E00
#define SLOT_ACTIVE_1_START          0x0808A000
#define SLOT_ACTIVE_1_END            0x080FFFFF

/* Dwl slot #1  (472 kbytes) */
#define SLOT_DWL_1_START             0x90000000
#define SLOT_DWL_1_END               0x90075FFF

/* Active slot #2  (432 kbytes) */
#define SLOT_ACTIVE_2_HEADER         0x08089000
#define SLOT_ACTIVE_2_START          0x08014000
#define SLOT_ACTIVE_2_END            0x0807FFFF

/* Dwl slot #2  (432 kbytes) */
#define SLOT_DWL_2_START             0x90080000
#define SLOT_DWL_2_END               0x900EBFFF

/* Slots not configured */
#define SLOT_ACTIVE_3_HEADER         0x00000000
#define SLOT_ACTIVE_3_START          0x00000000
#define SLOT_ACTIVE_3_END            0x00000000
#define SLOT_DWL_3_START             0x00000000
#define SLOT_DWL_3_END               0x00000000

/* SWAP installation feature not allowed in this example in order to keep dwl slot encrypted */
#define SWAP_START                   0x00000000
#define SWAP_END                     0x00000000


#endif
