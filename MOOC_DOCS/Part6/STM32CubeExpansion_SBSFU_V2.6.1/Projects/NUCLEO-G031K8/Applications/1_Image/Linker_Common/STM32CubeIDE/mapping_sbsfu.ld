/*
*****************************************************************************
**
**  File        : LinkerScript.ld
**
**  Abstract    : Linker script for STM32G031K8Tx Device with
**                64KByte FLASH, 8KByte RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STMicroelectronics STM32
**
**
**  Distribution: The file is distributed as is, without any warranty
**                of any kind.
**
*****************************************************************************
*/

VECTOR_SIZE = 0x200;

/* SE Code region protected by MPU isolation */
__ICFEDIT_SE_Code_region_ROM_start__       = 0x08000000 + VECTOR_SIZE;
__ICFEDIT_SE_CallGate_region_ROM_start__   = __ICFEDIT_SE_Code_region_ROM_start__;  /* No need to do +4 as we have dummy bytes in SE_CoreBin .ld file */
__ICFEDIT_SE_CallGate_region_ROM_end__     = __ICFEDIT_SE_Code_region_ROM_start__ + 0x1FF;

/* SE key region protected by MPU isolation */
__ICFEDIT_SE_Key_region_ROM_start__        = __ICFEDIT_SE_CallGate_region_ROM_end__ + 0x1;   /* PCROP protection : Alignment on 512 bytes */
__ICFEDIT_SE_Key_region_ROM_end__          = __ICFEDIT_SE_Key_region_ROM_start__ + 0x3FF;    /* PCROP protection : Minimum size is 1024 bytes */

/* SE Startup */
__ICFEDIT_SE_Startup_region_ROM_start__    = __ICFEDIT_SE_Key_region_ROM_end__ + 0x1;
__ICFEDIT_SE_Code_nokey_region_ROM_start__ = __ICFEDIT_SE_Startup_region_ROM_start__ + 0x100;
/* Aligned SE End at the end of the 1st 16 Kbytes of flash, MPU protection isolation constraints */
__ICFEDIT_SE_Code_region_ROM_end__         = 0x08003FFF;

/* SE IF ROM: used to locate Secure Engine interface code out of MPU isolation     */
__ICFEDIT_SE_IF_region_ROM_start__         = __ICFEDIT_SE_Code_region_ROM_end__ + 0x1;
__ICFEDIT_SE_IF_region_ROM_end__           = __ICFEDIT_SE_IF_region_ROM_start__ + 0x8FF;

/* Secure memory activation code - initialization area */
__ICFEDIT_SB_HDP_region_ROM_start__        = __ICFEDIT_SE_IF_region_ROM_end__ + 1;
__ICFEDIT_SB_HDP_region_ROM_end__          = __ICFEDIT_SB_HDP_region_ROM_start__ + 0x13F;

/* SBSFU Code region */
__ICFEDIT_SB_region_ROM_start__            = __ICFEDIT_SB_HDP_region_ROM_end__ + 1;
/* Aligned SBSFU end at the end of the 1st 40 Kbytes of FLASH, MPU protection isolation constraints */
__ICFEDIT_SB_region_ROM_end__              = 0x08009FFF;

SE_Entry_Secure_ROM_Region_Length = __ICFEDIT_SE_CallGate_region_ROM_end__ - __ICFEDIT_SE_CallGate_region_ROM_start__ + 1;
SE_Key_region_ROM_Length          = __ICFEDIT_SE_Key_region_ROM_end__ - __ICFEDIT_SE_Key_region_ROM_start__ + 1;
SE_Startup_region_ROM_Length      = __ICFEDIT_SE_Code_nokey_region_ROM_start__ - __ICFEDIT_SE_Startup_region_ROM_start__ ;
SE_ROM_region_Length              = __ICFEDIT_SE_Code_region_ROM_end__ - __ICFEDIT_SE_Code_nokey_region_ROM_start__ + 1;
SE_IF_region_ROM_Length           = __ICFEDIT_SE_IF_region_ROM_end__ - __ICFEDIT_SE_IF_region_ROM_start__ + 1;
SB_HDP_ROM_region_Length          = __ICFEDIT_SB_HDP_region_ROM_end__ - __ICFEDIT_SB_HDP_region_ROM_start__ + 1;
SB_ROM_region_Length              = __ICFEDIT_SB_region_ROM_end__ - __ICFEDIT_SB_region_ROM_start__ + 1;

/* RAM section */
/* SE RAM1 region protected by MPU */
/* SE stack is placed 1st in RAM, stack overflow does not write on other RAM area */
__ICFEDIT_SE_region_RAM_start__     = 0x20000000;
__ICFEDIT_SE_region_RAM_stack_top__ = 0x20000400;
__ICFEDIT_SE_region_RAM_end__       = 0x20000DFF;

/* Secure memory activation code area - execution in RAM */
__ICFEDIT_SB_HDP_Code_region_RAM_start__ = __ICFEDIT_SE_region_RAM_end__ + 1;
__ICFEDIT_SB_HDP_Code_region_RAM_end__   = __ICFEDIT_SB_HDP_Code_region_RAM_start__ + 0x13F;

/* SBSFU RAM1 region */
__ICFEDIT_SB_region_RAM_start__     = __ICFEDIT_SB_HDP_Code_region_RAM_end__ + 1;
__ICFEDIT_SB_region_RAM_end__       = 0x20001FFF;

SE_RAM_region_Length = __ICFEDIT_SE_region_RAM_end__ - __ICFEDIT_SE_region_RAM_stack_top__ + 1;
SB_HDP_CODE_RAM_region_Length = __ICFEDIT_SB_HDP_Code_region_RAM_end__ - __ICFEDIT_SB_HDP_Code_region_RAM_start__ + 1;
SB_RAM_region_Length = __ICFEDIT_SB_region_RAM_end__ - __ICFEDIT_SB_region_RAM_start__ + 1;

MEMORY
{
 SE_Entry_Secure_ROM_Region (rx)     : ORIGIN = __ICFEDIT_SE_CallGate_region_ROM_start__, LENGTH = SE_Entry_Secure_ROM_Region_Length
 SE_Key_region_ROM (rx)              : ORIGIN = __ICFEDIT_SE_Key_region_ROM_start__, LENGTH = SE_Key_region_ROM_Length
 SE_Startup_region_ROM (rx)          : ORIGIN = __ICFEDIT_SE_Startup_region_ROM_start__, LENGTH = SE_Startup_region_ROM_Length
 SE_ROM_region (rx)                  : ORIGIN = __ICFEDIT_SE_Code_nokey_region_ROM_start__, LENGTH = SE_ROM_region_Length
 SE_IF_region_ROM (rx)               : ORIGIN = __ICFEDIT_SE_IF_region_ROM_start__, LENGTH = SE_IF_region_ROM_Length
 SB_HDP_ROM_region (rx)              : ORIGIN = __ICFEDIT_SB_HDP_region_ROM_start__, LENGTH = SB_HDP_ROM_region_Length
 SB_ROM_region (rx)                  : ORIGIN = __ICFEDIT_SB_region_ROM_start__, LENGTH = SB_ROM_region_Length
 SE_RAM_region (xrw)               : ORIGIN = __ICFEDIT_SE_region_RAM_stack_top__, LENGTH = SE_RAM_region_Length
 SB_HDP_Code_RAM_region (xrw)      : ORIGIN = __ICFEDIT_SB_HDP_Code_region_RAM_start__, LENGTH = SB_HDP_CODE_RAM_region_Length
 SB_RAM_region (xrw)               : ORIGIN = __ICFEDIT_SB_region_RAM_start__, LENGTH = SB_RAM_region_Length
}

