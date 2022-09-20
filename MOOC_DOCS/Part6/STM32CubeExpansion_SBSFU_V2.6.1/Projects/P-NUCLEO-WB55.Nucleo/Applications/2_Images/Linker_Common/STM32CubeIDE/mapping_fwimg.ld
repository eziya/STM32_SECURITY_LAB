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
**  @attention
**
**  <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
**  All rights reserved.</center></h2>
**
**  This software is licensed under terms that can be found in the LICENSE file in
**  the root directory of this software component.
**  If no LICENSE file comes with this software, it is provided AS-IS.
**
*****************************************************************************
*/

/* Slots must be aligned on 4096 bytes (0x1000) */

/* swap (8 kbytes) */
__ICFEDIT_SWAP_start__           = 0x08016000;
__ICFEDIT_SWAP_end__             = 0x08017FFF;

/* Active slot #1 (208 kbytes) */
__ICFEDIT_SLOT_Active_1_start__  = 0x08018000;
__ICFEDIT_SLOT_Active_1_end__    = 0x0804BFFF;
__ICFEDIT_SLOT_Active_1_header__ = __ICFEDIT_SLOT_Active_1_start__;

/* Dwl slot #1 (208 kbytes) */
__ICFEDIT_SLOT_Dwl_1_start__     = 0x0804C000;
__ICFEDIT_SLOT_Dwl_1_end__       = 0x0807FFFF;

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
