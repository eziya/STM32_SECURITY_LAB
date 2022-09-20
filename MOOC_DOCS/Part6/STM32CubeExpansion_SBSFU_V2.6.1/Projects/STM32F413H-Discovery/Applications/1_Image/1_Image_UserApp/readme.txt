/**
  @page Secure Firmware Update - User Application Demo

  @verbatim
  ******************** (C) COPYRIGHT 2017 STMicroelectronics *******************
  * @file    readme.txt
  * @brief   This application shows a User Application
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

This application demonstrates firmware download capabilities and provides a set of functions to test the active
protections offered by Secure Boot and Secure Engine.
A terminal connected with the board via VCOM is needed to communicate with the board and to select which feature
to demonstrate.

For more details, refer to UM2262 "Getting started with SBSFU - software expansion for STM32Cube" available from
the STMicroelectronics microcontroller website www.st.com.

@par Directory contents

   - 1_Image_UserApp/Src/com.c                      Communication module file
   - 1_Image_UserApp/Src/common.c                   Common module file
   - 1_Image_UserApp/Src/flash_if.c                 Flash interface file
   - 1_Image_UserApp/Src/fw_update_app.c            Firmware update application
   - 1_Image_UserApp/Src/main.c                     Main program
   - 1_Image_UserApp/Src/mpu.c                      Secure Engine isolation with MPU (when no Firewall is available)
   - 1_Image_UserApp/Src/se_user_code.c             Call user defined services running in Secure Engine
   - 1_Image_UserApp/Src/stm32f4xx_it.c             Interrupt handlers
   - 1_Image_UserApp/Src/system_stm32f4xx.c         STM32 system file
   - 1_Image_UserApp/Src/test_protections.c         Protection test
   - 1_Image_UserApp/Src/ymodem.c                   Ymodem communication module
   - 1_Image_UserApp/Inc/com.h                      Header for com.c file
   - 1_Image_UserApp/Inc/common.h                   Header for common.c file
   - 1_Image_UserApp/Inc/flash_if.h                 Header for flash_if.c file
   - 1_Image_UserApp/Inc/fw_update_app.h            Header for fw_update_app.c file
   - 1_Image_UserApp/Inc/main.h                     Header for main.c file
   - 1_Image_UserApp/Inc/mpu.h                      Header file for mpu.c
   - 1_Image_UserApp/Inc/se_user_code.h             Header file for se_user_code.c
   - 1_Image_UserApp/Inc/stm32f4xx_conf.h           HAL configuration file
   - 1_Image_UserApp/Inc/stm32f4xx_it.h             Header for stm32f4xx_it.c file
   - 1_Image_UserApp/Inc/test_protections.h         Header for test_protections.c file
   - 1_Image_UserApp/Inc/ymodem.h                   Header for ymodem.c file

@par Hardware and Software environment

   - This example runs on STM32F413xx devices.
   - This example has been tested with STM32F413H-DISCOVERY board and can be easily tailored to any other supported device and
     development board.
   - An up-to-date version of ST-LINK firmware is required. Upgrading ST-LINK firmware is a feature provided by
     STM32Cube programmer available on www.st.com.
   - This example is based on se_interface_application.o module exported by SBSFU project.
   - This example needs a terminal emulator.
   - Microsoft Windows has a limitation whereby paths to files and directories cannot be longer than 256 characters.
     Paths to files exceeding that limits cause tools (e.g. compilers, shell scripts) to fail reading from or writing to
     such files.
     As a workaround, it is advised to use the subst.exe command from within a command prompt to set up a local drive
     out of an existing directory on the hard drive, such as:
     C:\> subst X: <PATH_TO_CUBEFW>\Firmware

@par IDE postbuild script

In order to ease the development process, a postbuild script ("postbuild.bat") is integrated in each IDE project.
This postbuild script:
   - is generated when compiling the Secure Engine Core project,
   - prepares the firmware image of the user application to be installed in the device.

A known limitation of this integration occurs when you update the firmware version (parameter of postbuild.bat script).
The IDE does not track this update so you need to force the rebuild of the project manually.

@par How to use it ?

Refer to SBSFU readme and follow steps by steps instructions.

Once executed, this user application gives access to a menu which allows:
   1 - to download a new firmware : feature not supported in 1 firmware image example
   2 - to test protections (MPU isolation, PCROP, WRP, IWDG, TAMPER)
   3 - to demonstrate how to call user defined services running in Secure Engine
   4 - to provide access to multiple images feature
   5 - to validate a firmware image at first start-up

1. Pressing 1 allows to download a new firmware.
Feature not supported in 1 firmware image example

2. Pressing 2 allows to test protections.
   - CORRUPT IMAGE test (#1): causes a signature verification failure of the selected firmware image at next boot.
   - MPU isolation tests (#2, #3): cause a reset trying to access protected code or data (either in RAM or FLASH).
   - PCROP test (#4): PCROP protection is not available on this board.
   - WRP test (#5): causes an error trying to erase write protected code.
   - IWDG test (#6): causes a reset simulating a deadlock by not refreshing the watchdog.
   - TAMPER test (#7): causes a reset if a tamper event is detected. In order to generate a tamper event, user has to
     connect PC13 (CN8.8) to GND (It may be enough to put your finger close to PC13 (CN8.8)).
     Several resets may be generated during this test.
     It may be better to have PC13 (CN8.8) connected to 3V3 before doing the transition to GND.

3. Pressing 3 allows to call user defined services running in Secure Engine.
As an example, after selecting the firmware image, SE_APP_GetActiveFwInfo service is called in order
to display the information located in the protected area such as version and size.

4. This menu is dedicated to multiple images feature.
Feature not supported in 1 firmware image example.

5. This menu is dedicated to image validation.
Feature not supported in 1 firmware image example.


Note1 : There is only 1 active slot configured in this example.
Note2 : for Linux users Minicom can be used but to do so you need to compile the UserApp project with the MINICOM_YMODEM
        switch enabled (ymodem.h)
