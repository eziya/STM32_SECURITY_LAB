/**
  @page STSAFE_Provisionning project

  @verbatim
  ******************************************************************************
  * @file    readme.txt
  * @brief   Provisionning project
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

@endverbatim

@par Application Description

  This application is used to personalize secure element STSAFE-A for Secure Boot & Secure Firmware Update application
  (SBSFU).

  It is based on the following middlewares:

  - STSAFE_A1xx Middleware
  - MbedTLS

  Project needs to be compiled to generate the binary file to be loaded inside STM32 that allow to personalize STSAFE-A.
  Once run this application is doing several tasks :

  1. Establish communication between STM32 and STSAFE-A
  2. Check if Pairing keys (Host_MAC_Key and Host_Cipher_Key) are set inside STM32 & STSAFE-A else it set these keys
  3. Write in clear Host_MAC_Key and Host_Cipher_Key and executable code inside 510 & 511 page of STM32 flash
  4. Clear STSAFE-A100 zone 2 & 3
  5. Write certificate CERT_CA_FW inside STSAFE-A zone 2
  6. Write certificate CERT_CA_OEM_FW inside STSAFE-A zone 3
  7. Read and compare with expected contents both certificates written inside STSAFE-A

  Purpose of provisionning tools is also to provide to SECoreBin project values of pairing keys (Host_MAC_Key and Host_Cipher_Key).

  For more details, refer to UM2262 "Getting started with SBSFU - software expansion for STM32Cube"
  available from the STMicroelectronics microcontroller website www.st.com.
  Note: The information available for STSAFE-A100 is also valid for STSAFE-A110.

@par Directory contents

  - STSAFE_Provisioning/Src/main.c                                      Main program body
  - STSAFE_Provisioning/Src/flash_if.c                                  This file provides all the memory related operation functions
  - STSAFE_Provisioning/Src/stm32l4xx_hal_msp.c                         provides code for the MSP Initialization
  - STSAFE_Provisioning/Src/stm32l4xx_it.c                              Interrupt Service Routines
  - STSAFE_Provisioning/Src/system_stm32l4xx.c                          CMSIS Cortex-M4 Device Peripheral Access Layer System Source File
  - STSAFE_Provisioning/Src/HAL_STSAFE-Axx.c                           Secure storage API external functions
  - STSAFE_Provisioning/Src/HAL_STSAFE-Axx_INTERNAL.c                  Secure storage API internal functions
  - STSAFE_Provisioning/Src/stsafea_crypto_mbedtls_interface.c          Crypto Interface file to support the crypto services required by the STSAFE-A middleware
  - STSAFE_Provisioning/Src/stsafea_service_interface.c                 Service Interface file to support the hardware services required by the STSAFE-A Middleware
  - STSAFE_Provisioning/Src/Pairing.c                                   PCROP functions

  - STSAFE_Provisioning/Inc/main.h                                      Headers for Main program body
  - STSAFE_Provisioning/Inc/stm32l4xx_hal_conf.h                        Headers for HAL configuration file
  - STSAFE_Provisioning/Inc/stm32l4xx_it.h                              Headers for Interrupt Service Routines headers
  - STSAFE_Provisioning/Inc/flash_if.h                                  Headers for flash memory related operation functions
  - STSAFE_Provisioning/Inc/Commonappli.h                               Global configuration file for the application
  - STSAFE_Provisioning/Inc/config_mbedtls.h                            Configuration file for MBEDTLS
  - STSAFE_Provisioning/Inc/HAL_STSAFE-Axx.h                           Header for Secure storage API internal functions
  - STSAFE_Provisioning/Inc/HAL_STSAFE-Axx_INTERNAL.h                  Header for Secure storage API internal functions
  - STSAFE_Provisioning/Inc/pairing.h                                   Headers for PCROP functions
  - STSAFE_Provisioning/Inc/stsafea_conf.h                              STSAFE-A1xx Middleware configuration file
  - STSAFE_Provisioning/Inc/stsafea_interface_conf.h                    STSAFE-A1xx Middleware interface configuration

@par Hardware and Software environment

  - This example runs on STM32L4S5xx devices
  - This example has been tested with B-L4S5I-IOT01A board and can be
    easily tailored to any other supported device and development board.
  - An up-to-date version of ST-LINK firmware (V2.J29 or later) is required. Upgrading ST-LINK firmware
    is a feature provided by STM32Cube programmer available on www.st.com.
  - Microsoft Windows has a limitation whereby paths to files and directories cannot
    be longer than 256 characters. Paths to files exceeding that limits cause tools (e.g. compilers,
    shell scripts) to fail reading from or writing to such files.
    As a workaround, it is advised to use the subst.exe command from within a command prompt to set
    up a local drive out of an existing directory on the hard drive, such as:
    C:\> subst X: <PATH_TO_CUBEFW>\Firmware

@par IDE prebuild script

  None

@par How to use it ?

  A) Installation

  In order to generate Binary file which does provisionning, you must do the following :

  1. Open your preferred toolchain
  2. Rebuild the project
  3. Connect the B-L4S5I-IOT01A board to your PC
  4. Perform a full STM32 chip erase
  5. Flash binary Provisioning.bin using your IDE or STM32 STLINK Utility
  6. Open a terminal(example TERATERM) and connect it to STM32 virtual com Port (Speed 115200, Data 8 bits, Parity none, Stop bit 1 bit, Flow control none)
  7. Plug / unplug USB to power cycle STM32
  8. Push reset Button
  9. After provisionning ran successfully you should see under terminal :

-------------------------------------------------------------------------------
Start provisioning of STSAFE-A

Force STSAFE-A provisioning

Launching STSAFE-A pairing
Check if Pairing Host keys available
Pairing OK

Erase Data : OK

Now Store Certificate STM_POC_SBSFU_ROOT_TEST_CA_00 inside STSAFE-A
Certificate STM_POC_SBSFU_ROOT_TEST_CA_00 successfully written inside STSAFE-A : OK

Now Store Certificate  STM_POC_SBSFU_OEM_TEST_CA_00 inside STSAFE-A
Certificate STM_POC_SBSFU_OEM_TEST_CA_00 successfully written inside STSAFE-A : OK

Now reading and compare Certificates written inside STSAFE-A :

Get Certificate CERT_CA_FW OK
Certificate CERT_CA_FW successfully read and compared : OK
Get Certificate CERT_CA_OEM_FW OK
Certificate CERT_CA_OEM_FW successfully read and compared : OK

End provisioning of STSAFE-A

  B) Tuning of Host_MAC_Key and Host_Cipher_Key though Secure storage API (HAL_STSAFE-Axx.h)

  Provisionning tools can generate Host_MAC_Key and Host_Cipher_Key following 3 different methods as defined inside
  Init_Perso function. This means with with perso_type=0 these keys are known and fixed internally to the API secure storage.
  This is default method once using provisionning project for development and testing purposes. With perso_type=1 keys are random,
  this method is preferred once switcting to production, because keys are differentiated per chip. This is the best method
  in terms of security implementation. For perso_type=2 it's possible to provide
  as parameter Host_MAC_Key and Host_Cipher_Key values through a buffer.

  *  perso_type=0: use default keys and meanwhile set buf = NULL
  *  perso_type=1: use random generated keys and meanwhile set buf = NULL
  *  perso_type=2: use the keys customers prefer and meanwhile set buf = preferred key.

  Example of personalization mode random pairing keys :

  Modification to be done inside main.c :

  Mode perso_type=0 => Init_Perso(&stsafea_handle,DEFAULT_KEYS,NULL)
  Mode perso_type=1 => Init_Perso(&stsafea_handle,RANDOM_KEYS,NULL)
  Mode perso_type=2 => Init_Perso(&stsafea_handle,USER_KEYS,buffer)

  C) How to extract CA certificate manually generated by OpenSSL script and insert it inside provisionning project :

  CA certificate location :

  <YOUR DIRECTORY>\Firmware\Projects\B-L4S5I-IOT01A\Applications\2_Images_STSAFE\2_Images_SECoreBin\Binary\sbsfu_r0_crt.der

  OEM CA Certificate location :

  <YOUR DIRECTORY>\Firmware\Projects\B-L4S5I-IOT01A\Applications\2_Images_STSAFE\2_Images_SECoreBin\Binary\sbsfu_inter1_oem1_crt.der

  To convert from a DER certificate to a Char array :

  Output in C include file style :

  xxd -i sbsfu_r0_crt.der
  xxd -i sbsfu_inter1_oem1_crt.der

  Then integrate these certificates in define section of main.c

  /* sbsfu_r0_crt.der */
  static uint8_t CERT_CA_FW [] = { To be updated };

  /* sbsfu_inter1_oem1_crt.der */
  static uint8_t CERT_CA_OEM_FW [] = { To be updated  };

  * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
  */
