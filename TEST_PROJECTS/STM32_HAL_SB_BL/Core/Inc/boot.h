/*
 * boot.h
 *
 *  Created on: 2022. 8. 23.
 *      Author: eziya76@gmail.com
 */

#ifndef INC_BOOT_H_
#define INC_BOOT_H_

#include "main.h"

extern uint32_t _sdata;
extern uint32_t _eRAM;
#define BL_SRAM1_START      ((uint32_t)&_sdata)     //linker script
#define BL_SRAM1_END        ((uint32_t)&_eRAM)      //linker script

#define META_BLOCK_ADDR     (0x800A000)     //0x8000000 + 0xA000(BL length)
#define META_DATA_LEN       (128U)          //meta = meta_info + app_hash + padding
#define META_BLOCK_LEN      (0x200)         //meta_block = meta + meta_hash + meta_sig + padding

#define APP_ADDR            (META_BLOCK_ADDR+META_BLOCK_LEN)                //0x800A200
#define SEC_SIZE            ((APP_ADDR - FLASH_BASE) / FLASH_PAGE_SIZE)     //0xA200 / 2048 = 14

#define HASH_LEN            (32U)       //SHA256
#define SIG_LEN             (64U)       //ECDSA
#define ECC_PUB_LEN         (64U)       //ECC prime256v1


#define BL_EXIT_STICKY      (0x1FFF6800)        //exit secure memory address
#define MAGIC_NUMBER        (0x08192A3C)        //magic number for hash & system bootloader

typedef struct {
  uint32_t magicNum;                    //magic number          (4bytes)
  uint32_t appSize;                     //application size      (4bytes)
  uint32_t appVer;                      //application version   (4bytes)
  uint8_t  appHash[HASH_LEN];           //application hash      (32bytes)
  uint8_t  Reserved[84];                //padding bytes         (84bytes)
  uint8_t  metaHash[HASH_LEN];          //meta data hash        (32bytes)
  uint8_t  metaSig[SIG_LEN];            //meta data signature   (64bytes)
}MetaBlock;

bool ApplyOptionBytesProtections(void);
bool VerifyApplication(void);
void jump_to_application(void);

#endif /* INC_BOOT_H_ */
