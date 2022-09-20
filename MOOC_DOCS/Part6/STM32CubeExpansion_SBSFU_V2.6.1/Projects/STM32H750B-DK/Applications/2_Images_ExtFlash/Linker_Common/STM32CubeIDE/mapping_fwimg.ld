/*
******************************************************************************
**
**  File        : STM32H750XBHx_FLASH.ld
**
**
**
**  Abstract    : Linker script for STM32H7 series
**                128Kbytes FLASH and 192Kbytes RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STMicroelectronics STM32
**
**  Distribution: The file is distributed ''as is'', without any warranty
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

/*###ICF### Set of symbols used in SE and SB_SFU projects ****/

/* Slots Regions must be aligned on sector size */

/* Swap sector in external memory : 128 kbytes */
__ICFEDIT_SWAP_start__ = 0x90800000;
__ICFEDIT_SWAP_end__   = 0x9081FFFF;

/* Active slot #1 in external memory : 4 Mbytes */
__ICFEDIT_SLOT_Active_1_header__ = 0x90000000;
__ICFEDIT_SLOT_Active_1_start__  = 0x90000000;
__ICFEDIT_SLOT_Active_1_end__    = 0x903FFFFF;
/* APP LOADER code located inside Active slot 1
   with execution from RAM to be able to download new image inside external flash
   First 0x800 bytes reserved for header + vector table */
__ICFEDIT_APP_LOADER_region_ROM_start__  = 0x90000800;
__ICFEDIT_APP_LOADER_region_ROM_end__    = 0x90006FFF;

/* Dwl slot #1 in external memory : 4 Mbytes */
__ICFEDIT_SLOT_Dwl_1_start__ = 0x90400000;
__ICFEDIT_SLOT_Dwl_1_end__   = 0x907FFFFF;

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
