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

/******************************************************************************/
/*                              ROM section                                   */
/******************************************************************************/

VECTOR_SIZE = 0x400;

/* SE Code region protected by MPU isolation */
__ICFEDIT_SE_Code_region_ROM_start__       = 0x08000400;
__ICFEDIT_SE_CallGate_region_ROM_start__   = __ICFEDIT_SE_Code_region_ROM_start__ + 4;
__ICFEDIT_SE_CallGate_Region_ROM_End__     = __ICFEDIT_SE_Code_region_ROM_start__ + 0x1FF;

/* SE key region protected by MPU isolation */
__ICFEDIT_SE_Key_region_ROM_start__        = __ICFEDIT_SE_CallGate_Region_ROM_End__ + 1;
__ICFEDIT_SE_Key_region_ROM_end__          = __ICFEDIT_SE_Key_region_ROM_start__ + 0x2FF; /* H7: The minimum PCROP area that can be set is 16 Flash words, that is 512 bytes. */

/* SE Startup                               */
__ICFEDIT_SE_Startup_region_ROM_start__    = __ICFEDIT_SE_Key_region_ROM_end__ + 1;
__ICFEDIT_SE_Code_nokey_region_ROM_start__ = __ICFEDIT_SE_Startup_region_ROM_start__ + 0x100;
/* Aligned SE End at the end of the 1st 24Kbytes of flash, MPU protection isolation constraints */
__ICFEDIT_SE_Code_region_ROM_end__         = 0x08005FFF;

/* SE IF ROM: used locate Secure Engine interface code out of MPU isolation     */
/*
 * We must be out of Region 2 allow unprivileged access
 * With the current settings Region 2 starts from __ICFEDIT_bank1_secure_start__ and covers 24kB of code (0x6000)
 */
__ICFEDIT_SE_IF_region_ROM_start__         = 0x08000000 + 0x6000;
__ICFEDIT_SE_IF_region_ROM_end__           = __ICFEDIT_SE_IF_region_ROM_start__ + 0x7FF;

/* Secure memory activation code - initialization area */
__ICFEDIT_SB_HDP_region_ROM_start__        = __ICFEDIT_SE_IF_region_ROM_end__ + 1;
__ICFEDIT_SB_HDP_region_ROM_end__          = __ICFEDIT_SB_HDP_region_ROM_start__ + 0xFF;

/* SBSFU Code region */
__ICFEDIT_SB_region_ROM_start__            = __ICFEDIT_SB_HDP_region_ROM_end__ + 1;
__ICFEDIT_SB_region_ROM_end__              = __ICFEDIT_SB_region_ROM_start__ + 0x9FFF;

/* LOADER code region : Aligned on % 0x400 for vector table constraint */
__ICFEDIT_LOADER_region_ROM_start__        = ((__ICFEDIT_SB_region_ROM_end__ / 0x400) + 1) * 0x400;
/* Aligned SBSFU end at the end of the 1st 96Kbytes of FLASH, MPU protection isolation constraints */
__ICFEDIT_LOADER_region_ROM_end__          = 0x08017FFF;

/* SECURE COUNTER region */
__ICFEDIT_SECURE_COUNTER_region_ROM_start__ = __ICFEDIT_LOADER_region_ROM_end__ + 1;
__ICFEDIT_SECURE_COUNTER_region_ROM_end__   = 0x0801FFFF;

/* ROM regions definition */
SE_Entry_Secure_ROM_Region_Length = __ICFEDIT_SE_CallGate_Region_ROM_End__ - __ICFEDIT_SE_Code_region_ROM_start__ + 1;
SE_Key_region_ROM_Length          = __ICFEDIT_SE_Key_region_ROM_end__ - __ICFEDIT_SE_Key_region_ROM_start__ + 1;
SE_Startup_region_ROM_Length      = __ICFEDIT_SE_Code_nokey_region_ROM_start__ - __ICFEDIT_SE_Startup_region_ROM_start__ ;
SE_ROM_region_Length              = __ICFEDIT_SE_Code_region_ROM_end__ - __ICFEDIT_SE_Code_nokey_region_ROM_start__ + 1;
SE_IF_ROM_region_Length           = __ICFEDIT_SE_IF_region_ROM_end__ - __ICFEDIT_SE_IF_region_ROM_start__ + 1;
SB_HDP_ROM_region_Length          = __ICFEDIT_SB_HDP_region_ROM_end__ - __ICFEDIT_SB_HDP_region_ROM_start__ + 1;
SB_ROM_region_Length              = __ICFEDIT_SB_region_ROM_end__ - __ICFEDIT_SB_region_ROM_start__ + 1;
LOADER_ROM_region_Length          = __ICFEDIT_LOADER_region_ROM_end__ - __ICFEDIT_LOADER_region_ROM_start__ + 1;

/******************************************************************************/
/*                              RAM section                                   */
/*                                                                            */
/******************************************************************************/

/* SE RAM1 region protected area */
__ICFEDIT_SE_region_RAM_start__      = 0x20000000;
__ICFEDIT_SE_region_RAM_stack_top__  = 0x20000400; /* Secure Engine's private stack */
__ICFEDIT_SE_region_RAM_end__        = 0x20000FFF;

/* Secure memory activation code area - execution in RAM */
__ICFEDIT_SB_HDP_Code_region_RAM_start__ = __ICFEDIT_SE_region_RAM_end__ + 1;
__ICFEDIT_SB_HDP_Code_region_RAM_end__   = __ICFEDIT_SB_HDP_Code_region_RAM_start__ + 0xFF;

/* LOADER communication region */
__ICFEDIT_LOADER_COM_region_RAM_start__  = __ICFEDIT_SB_HDP_Code_region_RAM_end__ + 1;
__ICFEDIT_LOADER_COM_region_RAM_end__    = __ICFEDIT_LOADER_COM_region_RAM_start__ + 0x0F;

/* SBSFU RAM region */
__ICFEDIT_SB_region_RAM_start__ = __ICFEDIT_LOADER_COM_region_RAM_end__ + 1;
__ICFEDIT_SB_region_RAM_end__   = 0x2001FFFF;

/* LOADER RAM region */
__ICFEDIT_LOADER_region_RAM_start__ = __ICFEDIT_LOADER_COM_region_RAM_end__ + 1;
__ICFEDIT_LOADER_region_RAM_end__   = 0x20013FFF;

/* User App LOADER : code execution from RAM */
__ICFEDIT_APP_LOADER_region_RAM_code_start__ = 0x20014000;
__ICFEDIT_APP_LOADER_region_RAM_code_end__   = 0x2001FFFF;

/* RAM regions definition */
SE_RAM_region_Length               = __ICFEDIT_SE_region_RAM_end__ - __ICFEDIT_SE_region_RAM_stack_top__ + 1;
LOADER_COM_RAM_region_Length       = __ICFEDIT_LOADER_COM_region_RAM_end__ - __ICFEDIT_LOADER_COM_region_RAM_start__ + 1;
SB_HDP_CODE_RAM_region_Length      = __ICFEDIT_SB_HDP_Code_region_RAM_end__ - __ICFEDIT_SB_HDP_Code_region_RAM_start__ + 1;
SB_RAM_region_Length               = __ICFEDIT_SB_region_RAM_end__ - __ICFEDIT_SB_region_RAM_start__ + 1;
LOADER_RAM_region_Length 	       = __ICFEDIT_LOADER_region_RAM_end__ - __ICFEDIT_LOADER_region_RAM_start__ + 1;
APP_LOADER_RAM_code_region_Length  = __ICFEDIT_APP_LOADER_region_RAM_code_end__ - __ICFEDIT_APP_LOADER_region_RAM_code_start__ + 1;

MEMORY
{
 SE_Entry_Secure_ROM_Region (rx)     : ORIGIN = __ICFEDIT_SE_CallGate_region_ROM_start__, LENGTH = SE_Entry_Secure_ROM_Region_Length
 SE_Key_region_ROM (rx)              : ORIGIN = __ICFEDIT_SE_Key_region_ROM_start__, LENGTH = SE_Key_region_ROM_Length
 SE_Startup_region_ROM (rx)          : ORIGIN = __ICFEDIT_SE_Startup_region_ROM_start__, LENGTH = SE_Startup_region_ROM_Length
 SE_ROM_region (rx)                  : ORIGIN = __ICFEDIT_SE_Code_nokey_region_ROM_start__, LENGTH = SE_ROM_region_Length
 SE_IF_region_ROM (rx)               : ORIGIN = __ICFEDIT_SE_IF_region_ROM_start__, LENGTH = SE_IF_ROM_region_Length
 SB_HDP_ROM_region (rx)              : ORIGIN = __ICFEDIT_SB_HDP_region_ROM_start__, LENGTH = SB_HDP_ROM_region_Length
 SB_ROM_region (rx)                  : ORIGIN = __ICFEDIT_SB_region_ROM_start__, LENGTH = SB_ROM_region_Length
 LOADER_ROM_region (rx)              : ORIGIN = __ICFEDIT_LOADER_region_ROM_start__, LENGTH = LOADER_ROM_region_Length
 SE_RAM_region (xrw)               : ORIGIN = __ICFEDIT_SE_region_RAM_stack_top__, LENGTH = SE_RAM_region_Length
 SB_HDP_Code_RAM_region (xrw)      : ORIGIN = __ICFEDIT_SB_HDP_Code_region_RAM_start__, LENGTH = SB_HDP_CODE_RAM_region_Length
 LOADER_COM_RAM_region (xrw)       : ORIGIN = __ICFEDIT_LOADER_COM_region_RAM_start__, LENGTH = LOADER_COM_RAM_region_Length
 SB_RAM_region (xrw)               : ORIGIN = __ICFEDIT_SB_region_RAM_start__, LENGTH = SB_RAM_region_Length
 LOADER_RAM_region (xrw)           : ORIGIN = __ICFEDIT_LOADER_region_RAM_start__, LENGTH = LOADER_RAM_region_Length
 APP_LOADER_RAM_code_region(rx)    : ORIGIN = __ICFEDIT_APP_LOADER_region_RAM_code_start__, LENGTH = APP_LOADER_RAM_code_region_Length
}

