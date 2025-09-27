/**
  @page Secure Boot and Secure Firmware Update Demo Application
  
  @verbatim
  ******************************************************************************
  * @file    readme.txt
  * @brief   This application shows Secure Boot and Secure Firmware Update example.
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

The X-CUBE-SBSFU Secure Boot and Secure Firmware Update solution allows the update of the STM32 microcontroller built-in
program with new firmware versions, adding new features and correcting potential issues. The update process is performed
in a secure way to prevent unauthorized updates and access to confidential on-device data such as secret code and 
firmware encryption key.

The Secure Boot (Root of Trust services) is an immutable code, always executed after a system reset, that checks STM32 
static protections, activates STM32 runtime protections and then verifies the authenticity and integrity of user 
application code before every execution in order to ensure that invalid or malicious code cannot be run.

The Secure Firmware Update application receives the firmware image via a UART interface with the Ymodem protocol, checks
its authenticity, and checks the integrity of the code before installing it. The firmware update is done on the complete
firmware image, or only on a portion of the firmware image. 

This examples is provided with a dual firmware image configuration in order to ensure safe image installation and enable
over-the-air firmware update capability commonly used in IOT devices. 

This example is based on a third party cryptographic middleware : mbed TLS. 
Cryptographic services are delivered as open source code. It can be configured to use asymmetric or symmetric 
cryptographic schemes with or without firmware encryption.

@par Directory contents

   - 2_Images_SECoreBin  Generate secure engine binary including all the "trusted" code
   - 2_Images_SBSFU      Secure boot and secure firmware update application
   - 2_Images_UserApp    Example of user application
   - Linker_Common       Linker files definition shared between SECoreBin, SBSFU, UserApp
 
  
@par How to use it ? 

You need to follow a strict compilation order : 
 
1. Compile SECoreBin application
   This step is needed to create the Secure Engine core binary including all the "trusted" code and keys mapped inside
   the protected environment. The binary is linked with the SBSFU code in step 2. 

2. Compile SBSFU application
   This step compiles the SBSFU source code implementing the state machine and configuring the protections. In addition, 
   it links the code with the SECore binary generated at step 1 in order to generate a single SBSFU binary including the
   SE trusted code.

3. Compile UserApp application
   It generates:
   – The user application binary file that is uploaded to the device using the Secure Firmware Update process (UserApp.sfb).
   – A binary file concatenating the SBSFU binary, the user application binary in clear format, and the corresponding 
     FW header.

For more details, refer to UM2262 "Getting started with SBSFU - software expansion for STM32Cube" available from the 
STMicroelectronics microcontroller website www.st.com.

