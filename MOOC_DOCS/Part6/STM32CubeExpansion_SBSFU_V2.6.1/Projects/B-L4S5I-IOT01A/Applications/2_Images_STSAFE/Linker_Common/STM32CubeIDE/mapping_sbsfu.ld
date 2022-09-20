/*
*****************************************************************************
**
**  File        : LinkerScript.ld
**
**  Abstract    : Linker script for STM32L4S5VITx Device with
**                2048KByte FLASH, 640KByte RAM
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
**  (c)Copyright Ac6.
**  You may use this file as-is or modify it according to the needs of your
**  project. Distribution of this file (unmodified or modified) is not
**  permitted. Ac6 permit registered System Workbench for MCU users the
**  rights to distribute the assembled, compiled & linked contents of this
**  file as part of an application binary file, provided that it is built
**  using the System Workbench for MCU toolchain.
**
*****************************************************************************
*/


/* SE Code region protected by firewall */
VECTOR_SIZE = 0x200;
__ICFEDIT_SE_Code_region_ROM_start__       = 0x08000000 + VECTOR_SIZE;
__ICFEDIT_SE_CallGate_region_ROM_start__   = __ICFEDIT_SE_Code_region_ROM_start__;  /* No need to do +4 as we have dummy bytes in SE_CoreBin .ld file */
__ICFEDIT_SE_CallGate_region_ROM_end__     = __ICFEDIT_SE_Code_region_ROM_start__ + 0x1FF;

/* SE key region protected by firewall */
__ICFEDIT_SE_Key_region_ROM_start__        = __ICFEDIT_SE_CallGate_region_ROM_end__ + 0x1;
__ICFEDIT_SE_Key_region_ROM_end__          = __ICFEDIT_SE_Key_region_ROM_start__ + 0xFF;

/* SE Startup: call before enabling firewall*/
__ICFEDIT_SE_Startup_region_ROM_start__    = __ICFEDIT_SE_Key_region_ROM_end__ + 0x1;
__ICFEDIT_SE_Code_nokey_region_ROM_start__ = __ICFEDIT_SE_Startup_region_ROM_start__ + 0x100;
__ICFEDIT_SE_Code_region_ROM_end__         = __ICFEDIT_SE_Startup_region_ROM_start__ + 0x1BFFF;

/* SE IF ROM: used to locate Secure Engine interface code out of firewall */
__ICFEDIT_SE_IF_region_ROM_start__         = __ICFEDIT_SE_Code_region_ROM_end__ + 1;
__ICFEDIT_SE_IF_region_ROM_end__           = __ICFEDIT_SE_IF_region_ROM_start__ + 0xDFF;

/* SBSFU Code region */
__ICFEDIT_SB_region_ROM_start__            = __ICFEDIT_SE_IF_region_ROM_end__ + 0x1;
__ICFEDIT_SB_region_ROM_end__              = 0x0802FFFF;

/* KMS Data Storage (NVMS) region protected by firewall */
/* In Single Bank configuration, we propose to set it up just before the active slot header */
/* so that KMS data storage and active slot headers are included in the NVDATA section */
/* KMS Data Storage need for 2 blocks : 8 kbytes * 2 ==> 16 kbytes */
__ICFEDIT_KMS_DataStorage_start__          = 0x08118000;
__ICFEDIT_KMS_DataStorage_end__            = 0x0811BFFF;

SE_Entry_Secure_ROM_Region_Length = __ICFEDIT_SE_CallGate_region_ROM_end__ - __ICFEDIT_SE_CallGate_region_ROM_start__ + 1;
SE_Key_region_ROM_Length          = __ICFEDIT_SE_Key_region_ROM_end__ - __ICFEDIT_SE_Key_region_ROM_start__ + 1;
SE_Startup_region_ROM_Length      = __ICFEDIT_SE_Code_nokey_region_ROM_start__ - __ICFEDIT_SE_Startup_region_ROM_start__ ;
SE_ROM_region_Length              = __ICFEDIT_SE_Code_region_ROM_end__ - __ICFEDIT_SE_Code_nokey_region_ROM_start__ + 1;
SE_IF_region_ROM_Length           = __ICFEDIT_SE_IF_region_ROM_end__ - __ICFEDIT_SE_IF_region_ROM_start__ + 1;
SB_ROM_region_Length              = __ICFEDIT_SB_region_ROM_end__ - __ICFEDIT_SB_region_ROM_start__ + 1;

/* RAM section */
/* SE RAM1 region protected by firewall */
/* SE stack is placed 1st in RAM, stack overflow does not write on other RAM area */
__ICFEDIT_SE_region_RAM_start__     = 0x20000000;
__ICFEDIT_SE_region_RAM_stack_top__ = 0x20002000;
__ICFEDIT_SE_region_RAM_end__       = 0x2001FFBF;

/* See errata sheet: ES0393 - Rev 6 - October 2019                                                                              */
/* Only 128 Kbytes - 1 byte and with the 6 lowest bits set to 0 of SRAM1 can be protected by the firewall instead of the full memory space (192 Kbytes of SRAM1). */
/* Use SRAM2 or SRAM3 as system RAM and avoid accessing SRAM1 before any access to an address outside the protected SRAM1 area,                                   */
/* of which the 18 LSBs correspond to an address within the protected SRAM1 area.                                                                                 */
/* So we declare all SRAM1 to avoid spurious Firewall reset but only 0x1FFC0 bytes can be actually used (under Firewall).                                         */
__ICFEDIT_SE_region_SRAM1_do_not_use_start__ = 0x2001FFC0;
__ICFEDIT_SE_region_SRAM1_do_not_use_end__   = 0x2002FFFF; /* 0x10040 bytes we cannot use to avoid spurious firewall reset */

/* SBSFU RAM dummy memory access: See errata sheet: ES0393 - Rev 6 - October 2019 :                */
/* - if multiple RAM banks are available: select another RAM bank than the RAM bank used by FWALL  */
/*   AND                                                                                           */
/* - select a memory range with 18 LSB outside the 18 LSB range protected by FWALL                 */
__ICFEDIT_SB_region_RAM_dummy_access_start__ = 0x20030000;
__ICFEDIT_SB_region_RAM_dummy_access_end__   = __ICFEDIT_SB_region_RAM_dummy_access_start__ + 3;

/* SBSFU RAM region */
__ICFEDIT_SB_region_RAM_start__     = 0x20030010;     /* Alignment on 16 bytes */
__ICFEDIT_SB_region_RAM_end__       = 0x2009FFFF;

SE_RAM_region_Length = __ICFEDIT_SE_region_RAM_end__ - __ICFEDIT_SE_region_RAM_stack_top__ + 1;
SB_RAM_region_Length = __ICFEDIT_SB_region_RAM_end__ - __ICFEDIT_SB_region_RAM_start__ + 1;
SB_RAM_dummy_access_region_Length = __ICFEDIT_SB_region_RAM_dummy_access_end__ - __ICFEDIT_SB_region_RAM_dummy_access_start__ + 1;

MEMORY
{
 SE_Entry_Secure_ROM_Region (rx)     : ORIGIN = __ICFEDIT_SE_CallGate_region_ROM_start__, LENGTH = SE_Entry_Secure_ROM_Region_Length
 SE_Key_region_ROM (rx)              : ORIGIN = __ICFEDIT_SE_Key_region_ROM_start__, LENGTH = SE_Key_region_ROM_Length
 SE_Startup_region_ROM (rx)          : ORIGIN = __ICFEDIT_SE_Startup_region_ROM_start__, LENGTH = SE_Startup_region_ROM_Length
 SE_ROM_region (rx)                  : ORIGIN = __ICFEDIT_SE_Code_nokey_region_ROM_start__, LENGTH = SE_ROM_region_Length
 SE_IF_region_ROM (rx)               : ORIGIN = __ICFEDIT_SE_IF_region_ROM_start__, LENGTH = SE_IF_region_ROM_Length
 SB_ROM_region (rx)                  : ORIGIN = __ICFEDIT_SB_region_ROM_start__, LENGTH = SB_ROM_region_Length
 SE_RAM_region (xrw)                 : ORIGIN = __ICFEDIT_SE_region_RAM_stack_top__, LENGTH = SE_RAM_region_Length
 SB_RAM_region (xrw)                 : ORIGIN = __ICFEDIT_SB_region_RAM_start__, LENGTH = SB_RAM_region_Length
 SB_RAM_dummy_access (xrw)           : ORIGIN = __ICFEDIT_SB_region_RAM_dummy_access_start__, LENGTH = SB_RAM_dummy_access_region_Length
}

