/*
******************************************************************************
**
**  File        : LinkerScript.ld
**
**  Author		: STM32CubeIDE
**
**  Abstract    : Linker script for STM32WB55xG Device
**                      1024Kbytes ROM
**                      256Kbytes RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STMicroelectronics STM32
**
**  Distribution: The file is distributed as is without any warranty
**                of any kind.
**
*****************************************************************************
** @attention
**
** Copyright (c) 2019 STMicroelectronics.
** All rights reserved.
**
** This software is licensed under terms that can be found in the LICENSE file
** in the root directory of this software component.
** If no LICENSE file comes with this software, it is provided AS-IS.
**
*****************************************************************************
*/

/* Slots must be aligned on 4096 bytes (0x1000) */

/* Temporary mapping provided as example, as no external flash available on this board
   ===================================================================================
   - Dwl slot should be mapped in external flash
   - Backup slot should be mapped in external flash
   - Active slot should be extended to all the internal flash free
*/

/* Active slot #1 (424 kbytes) */
__ICFEDIT_SLOT_Active_1_start__  = 0x08017000;
__ICFEDIT_SLOT_Active_1_end__    = 0x0807FFFF;
__ICFEDIT_SLOT_Active_1_header__ = 0x08016000;

/* Dwl slot #1 (448 KBytes, that is to say seven 64-KBytes blocks) */
__ICFEDIT_SLOT_Dwl_1_start__     = 0x90000000;
__ICFEDIT_SLOT_Dwl_1_end__       = 0x9006FFFF;

/* Backup (448 Kbytes, that is to say seven 64-KBytes blocks) */
__ICFEDIT_BACKUP_start__         = 0x90070000;
__ICFEDIT_BACKUP_end__           = 0x900DFFFF;

/* Slots not configured */
__ICFEDIT_SLOT_Active_2_header__ = 0x00000000;
__ICFEDIT_SLOT_Active_2_start__  = 0x00000000;
__ICFEDIT_SLOT_Active_2_end__    = 0x00000000;
__ICFEDIT_SLOT_Active_3_header__ = 0x00000000;
__ICFEDIT_SLOT_Active_3_start__  = 0x00000000;
__ICFEDIT_SLOT_Active_3_end__    = 0x00000000;
__ICFEDIT_SLOT_Dwl_2_start__     = 0x00000000;
__ICFEDIT_SLOT_Dwl_2_end__       = 0x00000000;
__ICFEDIT_SLOT_Dwl_3_start__     = 0x00000000;
__ICFEDIT_SLOT_Dwl_3_end__       = 0x00000000;

/* SWAP installation feature not allowed in this example in order to keep dwl slot encrypted */
__ICFEDIT_SWAP_start__           = 0x00000000;
__ICFEDIT_SWAP_end__             = 0x00000000;
