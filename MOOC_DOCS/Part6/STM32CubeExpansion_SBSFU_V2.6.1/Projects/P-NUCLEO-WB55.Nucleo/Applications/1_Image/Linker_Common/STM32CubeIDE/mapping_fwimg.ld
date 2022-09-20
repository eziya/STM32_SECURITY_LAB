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

/* Active slot #1 : 424 Kbytes */
/* Header located outside download area for anti-rollback mechanism
   ==> In case of wireless stack update the download area from
   __ICFEDIT_SLOT_Active_1_start__ to __ICFEDIT_SLOT_Active_1_end__ is erased
*/
__ICFEDIT_SLOT_Active_1_header__ = 0x08015000;
__ICFEDIT_SLOT_Active_1_start__  = 0x08016000;
__ICFEDIT_SLOT_Active_1_end__    = 0x0807FFFF;

/* Slots not configured */
__ICFEDIT_SLOT_Active_2_header__ = 0x00000000;
__ICFEDIT_SLOT_Active_2_start__  = 0x00000000;
__ICFEDIT_SLOT_Active_2_end__    = 0x00000000;
__ICFEDIT_SLOT_Active_3_header__ = 0x00000000;
__ICFEDIT_SLOT_Active_3_start__  = 0x00000000;
__ICFEDIT_SLOT_Active_3_end__    = 0x00000000;

