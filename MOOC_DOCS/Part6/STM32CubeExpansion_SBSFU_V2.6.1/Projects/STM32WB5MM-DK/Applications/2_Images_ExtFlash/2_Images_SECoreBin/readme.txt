/**
  @page Secure Engine Core binary generation.

  @verbatim
  ******************** (C) COPYRIGHT 2017 STMicroelectronics *******************
  * @file    readme.txt
  * @brief   Secure Engine Core binary generation.
  *          SE Core binary exports SE CallGate function entry point for
  *          SFU Application.
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
  @endverbatim

@par Application Description

This application is used to generate Secure Engine Core binary file to be linked with Secure Boot & Secure Firmware
Update application (SBSFU).
SE Core binary exports a single entry point to SE CallGate function and a startup method for data initialization.
It is based on the Secure Engine Middleware.

It has to be compiled before SBSFU project.

Three cryptographic schemes are provided as example to illustrate the cryptographic operations. The default
cryptographic scheme uses both symmetric (AES-CBC) and asymmetric (ECDSA) cryptography. Two alternate schemes are
provided and can be selected thanks to SECBOOT_CRYPTO_SCHEME compiler switch named :
   - SECBOOT_ECCDSA_WITH_AES128_CBC_SHA256 (default)
   - SECBOOT_ECCDSA_WITHOUT_ENCRYPT_SHA256
   - SECBOOT_AES128_GCM_AES128_GCM_AES128_GCM

For more details, refer to UM2262 "Getting started with SBSFU - software expansion for STM32Cube"
available from the STMicroelectronics microcontroller website www.st.com.

@par Directory contents

   - 2_Images_SECoreBin/Src/hw_ipcc.c                          Hardware IPCC source file for BLE
   - 2_Images_SECoreBin/Src/se_cm0.c                           Secure Engine communication with CM0
   - 2_Images_SECoreBin/Src/se_crypto_bootloader.c              Implementation of the crypto scheme functions used by the bootloader
   - 2_Images_SECoreBin/Src/se_low_level.c                      Low level interface
   - 2_Images_SECoreBin/Src/stm32wbxx_hal_msp.c                 HAL MSP implementation
   - 2_Images_SECoreBin/Src/stm32wbxx_it.c                      Interrupt handler
   - 2_Images_SECoreBin/Inc/app_common.h                        App Common application configuration file for BLE
   - 2_Images_SECoreBin/Inc/app_conf.h                          Application configuration file for BLE
   - 2_Images_SECoreBin/Inc/hw_conf.h                           Hardware configuration file for BLE
   - 2_Images_SECoreBin/Inc/hw_if.h                             Hardware Interface for BLE
   - 2_Images_SECoreBin/Inc/se_cm0.h                            Definitions for CM0 operations
   - 2_Images_SECoreBin/Inc/se_crypto_bootloader.h              Header file for se_crypto_bootloader.c
   - 2_Images_SECoreBin/Inc/se_crypto_config.h                  Crypto scheme configuration (crypto scheme used by the bootloader)
   - 2_Images_SECoreBin/Inc/se_def_metadata.h                   Firmware metadata (header) definition
   - 2_Images_SECoreBin/Inc/se_low_level.h                      Header file for low level interface
   - 2_Images_SECoreBin/Inc/stm32wbxx_hal_conf.h                HAL configuration file
   - 2_Images_SECoreBin/Inc/stm32wbxx_it.h                      Header file for Interrupt handler
   - 2_Images_SECoreBin/Inc/utilities_conf.h                    Configuration file for utilities
   - 2_Images_SECoreBin/Binary/ECCKEY1.txt                      Private ECCDSA key for signature verification
   - 2_Images_SECoreBin/Binary/nonce.bin                        Number used only once for firmware encryption
   - 2_Images_SECoreBin/Binary/OEM_KEY_COMPANY1_key_AES_CBC.bin Public key for AES CBC encryption
   - 2_Images_SECoreBin/Binary/OEM_KEY_COMPANY1_key_AES_GCM.bin Public key for AES GCM encryption

@par Hardware and Software environment

   - This example runs on STM32WB5Mxx devices
   - This example has been tested with STM32WB5MM-DK board and can be easily tailored to any other supported device and
     development board.
   - An up-to-date version of ST-LINK firmware is required. Upgrading ST-LINK firmware is a feature provided by
     STM32Cube programmer available on www.st.com.
   - Microsoft Windows has a limitation whereby paths to files and directories cannot be longer than 256 characters.
     Paths to files exceeding that limits cause tools (e.g. compilers, shell scripts) to fail reading from or writing
     to such files. As a workaround, it is advised to use the subst.exe command from within a command prompt to set up
     a local drive out of an existing directory on the hard drive, such as:
     C:\> subst X: <PATH_TO_CUBEFW>\Firmware

@par AES key provisioning

Before the first execution of SBSFU example, the symmetric key used for AES cryptographic functions should be
provisioned into the dedicated FUS Flash via STM32CubeProgrammer Command Line Interface (CLI) :
1. Access to Bootloader USB Interface (system flash) :
   - Jumper on CN13(VDD-Boot0) after Pins header soldering
   - Power ON via USB_USER and Jumper JP2(USB_MCU)

2. Verify RSS state is IDLE :
   - STM32_Programmer_CLI.exe -c port=usb1 -fusgetstate
   RSS state is RSS_IDLE ==> do the fusgetstate command twice if not in idle

3. Read FUS Version
   - STM32_Programmer_CLI.exe -c port=usb1 -r32 0x20030030 1
     0x20030030 : 00050300 ==> FUSv0.5.3. Must be at least FUSv1.0.2. Please refer to STM32WB_Copro_Wireless_Binaries
     release note from STM32Cube_FW_WB package to update FUS firmware.

4. You may update the key into key binary file

5. Provisioning AES key in index 0x1
   - STM32_Programmer_CLI.exe -c port=usb1 -wusrkey OEM_KEY_COMPANY1_key_AES_CBC.bin keytype=1
   - this step can be done only once. Index will be incremented at each command but our example requires key to be
     located at index 0x1 (SBSFU_AES_KEY_IDX).

6. Revert Step 1 procedure to put back device in normal mode

For more details refer to the Application Note:
   AN5185 - ST firmware upgrade services for STM32WB Series
   UM2237 - STM32CubeProgrammer software description

@par IDE prebuild script

In order to ease the development process, a prebuild script ("prebuild.bat") is integrated in each IDE project.
This preliminary processing is in charge of:
   - determining the requested cryptographic scheme
   - generating the appropriate keys ("se_keys.s" file)
   - generating the appropriate script to prepare the firmware image ("postbuild.bat") when building the UserApp project

A known limitation of this integration occurs when you update a cryptographic parameter (for instance the cryptographic
key). The IDE does not track this update so you need to force the rebuild of the project manually.

@par How to use it ?

In order to generate Secure Engine Core binary file, you must do the following :
   - Open your preferred toolchain
   - Rebuild the project

