/**
  ******************************************************************************
  * @file    mapping_sbsfu.h
  * @author  MCD Application Team
  * @brief   This file contains definitions for SBSFU mapping
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
#ifndef MAPPING_SBSFU_H
#define MAPPING_SBSFU_H

/* SE Code region protected by firewall */
#define VECTOR_SIZE                         0x200
#define ROM_START                           0x08000000
#define SE_CODE_REGION_ROM_START            (ROM_START + VECTOR_SIZE)

#define SE_CALLGATE_REGION_ROM_START        (SE_CODE_REGION_ROM_START + 0x4) 
#define SE_CALLGATE_REGION_ROM_END          (SE_CODE_REGION_ROM_START + 0x1FF)

/* SE key region protected by firewall */
#define SE_KEY_REGION_ROM_START             (SE_CALLGATE_REGION_ROM_END + 0x1)
#define SE_KEY_REGION_ROM_END               (SE_KEY_REGION_ROM_START+ 0xFF)

/* SE Startup: call before enabling firewall*/
#define SE_STARTUP_REGION_ROM_START         (SE_KEY_REGION_ROM_END + 0x1)
#define SE_CODE_NOKEY_REGION_ROM_START      (SE_STARTUP_REGION_ROM_START + 0x100)
#define SE_CODE_REGION_ROM_END              (SE_STARTUP_REGION_ROM_START + 0x1B7FF)

/* SE IF ROM: used to locate Secure Engine interface code out of firewall     */
#define SE_IF_REGION_ROM_START              (SE_CODE_REGION_ROM_END + 0x1)
#define SE_IF_REGION_ROM_END                (SE_IF_REGION_ROM_START + 0xBFF)

/* SBSFU Code region */
#define SB_REGION_ROM_START                 (SE_IF_REGION_ROM_END + 0x1)
#define SB_REGION_ROM_END                   0x0802FFFF

/* KMS Data Storage (NVMS) region protected by firewall */
/* In Single Bank configuration, we propose to set it up just before the active slot header */
/* so that KMS data storage and active slot headers are included in the NVDATA section */
/* KMS Data Storage need for 2 blocks : 8 kbytes * 2 ==> 16 kbytes */
#define KMS_DATASTORAGE_START               0x08118000
#define KMS_DATASTORAGE_END                 0x0811BFFF

/* ROM regions size definition */
#define SE_CALLGATE_REGION_ROM_SIZE         (SE_CALLGATE_REGION_ROM_END - SE_CODE_REGION_ROM_START + 0x1)
#define SE_KEY_REGION_ROM_SIZE              (SE_KEY_REGION_ROM_END - SE_KEY_REGION_ROM_START + 0x1)
#define SE_ROM_REGION_SIZE                  (SE_CODE_REGION_ROM_END - SE_CODE_NOKEY_REGION_ROM_START + 0x1)
#define SE_IF_ROM_REGION_SIZE               (SE_IF_REGION_ROM_END - SE_IF_REGION_ROM_START + 0x1)
#define SB_ROM_REGION_SIZE                  (SB_REGION_ROM_END - SB_REGION_ROM_START + 0x1)
#define KMS_DATASTORAGE_SIZE                (KMS_DATASTORAGE_END - KMS_DATASTORAGE_START + 0x1)

/*                              RAM section                                   */
/* SE RAM region protected by firewall */
/* SE stack is placed 1st in RAM, stack overflow does not write on other RAM area */

#define SE_REGION_RAM_START                  0x20000000
#define SE_REGION_RAM_STACK_TOP              0x20002000
#define SE_REGION_RAM_END                    0x2001FFBF

/* See errata sheet: ES0393 - Rev 6 - October 2019                                                                              */
/* Only 128 Kbytes - 1 byte and with the 6 lowest bits set to 0 of SRAM1 can be protected by the firewall instead of the full memory space (192 Kbytes of SRAM1). */
/* Use SRAM2 or SRAM3 as system RAM and avoid accessing SRAM1 before any access to an address outside the protected SRAM1 area,                                   */
/* of which the 18 LSBs correspond to an address within the protected SRAM1 area.                                                                                 */
/* So we declare all SRAM1 to avoid spurious Firewall reset but only 0x1FFC0 bytes can be actually used (under Firewall).                                         */
#define SE_REGION_RAM_DO_NOT_USE_START       0x2001FFC0
#define SE_REGION_RAM_DO_NOT_USE_END         0x2002FFFF   /* 0x10040 bytes we cannot use to avoid spurious firewall reset */

/* SBSFU RAM dummy memory access: See errata sheet: ES0393 - Rev 6 - October 2019 :                */
/* - if multiple RAM banks are available: select another RAM bank than the RAM bank used by FWALL  */
/*   AND                                                                                           */
/* - select a memory range with 18 LSB outside the 18 LSB range protected by FWALL                 */
#define SB_REGION_RAM_DUMMY_ACCESS_START    0x20030000
#define SB_REGION_RAM_DUMMY_ACCESS_END      (SB_REGION_RAM_DUMMY_ACCESS_START + 0x3)

/* SBSFU RAM region */
#define SB_REGION_RAM_START               0x20030010     /* Alignment on 16 bytes */
#define SB_REGION_RAM_END                 0x2009FFFF

#define SE_RAM_REGION_START               (SE_REGION_RAM_STACK_TOP)
#define SE_RAM_REGION_SIZE                (SE_REGION_RAM_END - SE_REGION_RAM_STACK_TOP + 0x1)
#define SB_RAM_REGION_SIZE                (SB_REGION_RAM_END - SB_REGION_RAM_START + 0x1)
#define SB_RAM_REGION_DUMMY_ACCESS_SIZE   (SB_REGION_RAM_DUMMY_ACCESS_END - SB_REGION_RAM_DUMMY_ACCESS_START + 0x1)

#endif

