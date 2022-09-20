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

/* SE Code region */
VECTOR_SIZE = 0x200;
__ICFEDIT_SE_Code_region_ROM_start__       = 0x08000000 + VECTOR_SIZE;
__ICFEDIT_SE_CallGate_region_ROM_start__   = __ICFEDIT_SE_Code_region_ROM_start__;  /* No need to do +4 as we have dummy bytes in SE_CoreBin .ld file */
__ICFEDIT_SE_CallGate_region_ROM_end__     = __ICFEDIT_SE_Code_region_ROM_start__ + 0x1FF;

/* SE key region */
__ICFEDIT_SE_Key_region_ROM_start__        = __ICFEDIT_SE_CallGate_region_ROM_end__ + 0x1;
__ICFEDIT_SE_Key_region_ROM_end__          = __ICFEDIT_SE_Key_region_ROM_start__ + 0xFF;

/* SE Startup */
__ICFEDIT_SE_Startup_region_ROM_start__    = __ICFEDIT_SE_Key_region_ROM_end__ + 0x1;
__ICFEDIT_SE_Code_nokey_region_ROM_start__ = __ICFEDIT_SE_Startup_region_ROM_start__ + 0x100;
__ICFEDIT_SE_Code_region_ROM_end__         = __ICFEDIT_SE_Startup_region_ROM_start__ + 0x32FF;

/* SE IF ROM */
__ICFEDIT_SE_IF_region_ROM_start__         = __ICFEDIT_SE_Code_region_ROM_end__ + 1;
__ICFEDIT_SE_IF_region_ROM_end__           = __ICFEDIT_SE_IF_region_ROM_start__ + 0x4FF;

/* SBSFU Code region */
__ICFEDIT_SB_region_ROM_start__            = __ICFEDIT_SE_IF_region_ROM_end__ + 0x1;
__ICFEDIT_SB_region_ROM_end__              = 0x0800FFFF;

/* LOADER code region : 24 kBytes area aligned on 4kBytes address, MPU protection constraints */
__ICFEDIT_LOADER_region_ROM_start__        = __ICFEDIT_SB_region_ROM_end__ + 1;
__ICFEDIT_LOADER_region_ROM_end__          = 0x08015FFF;

SE_Entry_Secure_ROM_Region_Length = __ICFEDIT_SE_CallGate_region_ROM_end__ - __ICFEDIT_SE_CallGate_region_ROM_start__ + 1;
SE_Key_region_ROM_Length          = __ICFEDIT_SE_Key_region_ROM_end__ - __ICFEDIT_SE_Key_region_ROM_start__ + 1;
SE_Startup_region_ROM_Length      = __ICFEDIT_SE_Code_nokey_region_ROM_start__ - __ICFEDIT_SE_Startup_region_ROM_start__ ;
SE_ROM_region_Length              = __ICFEDIT_SE_Code_region_ROM_end__ - __ICFEDIT_SE_Code_nokey_region_ROM_start__ + 1;
SE_IF_region_ROM_Length           = __ICFEDIT_SE_IF_region_ROM_end__ - __ICFEDIT_SE_IF_region_ROM_start__ + 1;
SB_ROM_region_Length              = __ICFEDIT_SB_region_ROM_end__ - __ICFEDIT_SB_region_ROM_start__ + 1;
LOADER_ROM_region_Length          = __ICFEDIT_LOADER_region_ROM_end__ - __ICFEDIT_LOADER_region_ROM_start__ + 1;

/* RAM section */
/* SE stack is placed 1st in RAM, stack overflow does not write on other RAM area */
__ICFEDIT_SE_region_RAM_start__     = 0x20000000;
__ICFEDIT_SE_region_RAM_stack_top__ = 0x20000400;
__ICFEDIT_SE_region_RAM_end__       = 0x20000FFF;

/* LOADER communication region when bypass mode activated */
__ICFEDIT_LOADER_COM_region_RAM_start__ = __ICFEDIT_SE_region_RAM_end__ + 1;
__ICFEDIT_LOADER_COM_region_RAM_end__   = __ICFEDIT_LOADER_COM_region_RAM_start__ + 0x0F;

/* LOADER RAM1 region */
__ICFEDIT_LOADER_region_RAM_start__ = __ICFEDIT_LOADER_COM_region_RAM_end__ + 1;
__ICFEDIT_LOADER_region_RAM_end__   = 0x2002FFFF;

/* SBSFU RAM1 region */
__ICFEDIT_SB_region_RAM_start__     = __ICFEDIT_LOADER_COM_region_RAM_end__ + 1;
__ICFEDIT_SB_region_RAM_end__       = 0x2002FFFF;

SE_RAM_region_Length = __ICFEDIT_SE_region_RAM_end__ - __ICFEDIT_SE_region_RAM_stack_top__ + 1;
LOADER_RAM_region_Length = __ICFEDIT_LOADER_region_RAM_end__ - __ICFEDIT_LOADER_region_RAM_start__ + 1;
SB_RAM_region_Length = __ICFEDIT_SB_region_RAM_end__ - __ICFEDIT_SB_region_RAM_start__ + 1;

MEMORY
{
 SE_Entry_Secure_ROM_Region (rx)     : ORIGIN = __ICFEDIT_SE_CallGate_region_ROM_start__, LENGTH = SE_Entry_Secure_ROM_Region_Length
 SE_Key_region_ROM (rx)              : ORIGIN = __ICFEDIT_SE_Key_region_ROM_start__, LENGTH = SE_Key_region_ROM_Length
 SE_Startup_region_ROM (rx)          : ORIGIN = __ICFEDIT_SE_Startup_region_ROM_start__, LENGTH = SE_Startup_region_ROM_Length
 SE_ROM_region (rx)                  : ORIGIN = __ICFEDIT_SE_Code_nokey_region_ROM_start__, LENGTH = SE_ROM_region_Length
 SE_IF_region_ROM (rx)               : ORIGIN = __ICFEDIT_SE_IF_region_ROM_start__, LENGTH = SE_IF_region_ROM_Length
 SB_ROM_region (rx)                  : ORIGIN = __ICFEDIT_SB_region_ROM_start__, LENGTH = SB_ROM_region_Length
 LOADER_ROM_region (rx)              : ORIGIN = __ICFEDIT_LOADER_region_ROM_start__, LENGTH = LOADER_ROM_region_Length
 SE_RAM_region (xrw)                 : ORIGIN = __ICFEDIT_SE_region_RAM_stack_top__, LENGTH = SE_RAM_region_Length
 LOADER_RAM_region (xrw)             : ORIGIN = __ICFEDIT_LOADER_region_RAM_start__, LENGTH = LOADER_RAM_region_Length
 SB_RAM_region (xrw)                 : ORIGIN = __ICFEDIT_SB_region_RAM_start__, LENGTH = SB_RAM_region_Length
 RAM_SHARED (rw)                     : ORIGIN = 0x20030000, LENGTH = 10K
}

