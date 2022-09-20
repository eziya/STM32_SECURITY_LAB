/**
  @page Standalone Loader Demo Application

  @verbatim
  ******************** (C) COPYRIGHT 2017 STMicroelectronics *******************
  * @file    readme.txt
  * @brief   This application shows Ymodem Loader example.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file in
  * the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  @endverbatim

@par Application Description

This application based on YModem protocol demonstrates firmware download capabilities.
It can be executed in a standalone way or executed through SBSFU.

In such configuration with an external flash, the loader functionality should be executed either from internal flash
or from RAM to be able to store downloaded firmware in external flash during program execution.

For more details, refer to UM2262 "Getting started with SBSFU - software expansion for STM32Cube" available from the
STMicroelectronics microcontroller website www.st.com.

@par Directory contents

   - 2_Images_LOADER/Src/loader.c                      Image download services
   - 2_Images_LOADER/Src/loader_low_level.c            General (CRC, UART) low level interface
   - 2_Images_LOADER/Src/loader_low_level_flash.c      Flash low level interface (wrapper)
   - 2_Images_LOADER/Src/loader_low_level_flash_int.c  Internal flash low level interface
   - 2_Images_LOADER/Src/loader_low_level_flash_ext.c  External flash low level interface
   - 2_Images_LOADER/Src/main.c                        Main application file
   - 2_Images_LOADER/Src/stm32h7xx_it.c                STM32 interrupt handlers
   - 2_Images_LOADER/Src/system_stm32h7xx.c            STM32 system file
   - 2_Images_LOADER/Inc/loader.h                      Header for loader.c file
   - 2_Images_LOADER/Inc/loader_low_level.h            Header for loader_low_level.c file
   - 2_Images_LOADER/Inc/loader_low_level_flash.h      Header for loader_low_level_flash file
   - 2_Images_LOADER/Inc/loader_low_level_flash_int.h  Header for loader_low_level_flash_int file
   - 2_Images_LOADER/Inc/loader_low_level_flash_ext.h  Header for loader_low_level_flash_ext file
   - 2_Images_LOADER/Inc/main.h                        Header for main.c file
   - 2_Images_LOADER/Inc/stm32h7xx_conf.h              HAL configuration file
   - 2_Images_LOADER/Inc/stm32h7xx_it.h                Header for stm32h7xx_it.h file

@par Hardware and Software environment

   - This example runs on STM32H7B3xx devices.
   - This example has been tested with STM32H7B3I-DK board and can be easily tailored to any other supported device and
     development board.
   - An up-to-date version of ST-LINK firmware is required. Upgrading ST-LINK firmware is a feature provided by
     STM32Cube programmer available on www.st.com.
   - This example needs a terminal emulator.
   - Microsoft Windows has a limitation whereby paths to files and directories cannot be longer than 256 characters.
     Paths to files exceeding that limits cause tools (e.g. compilers, shell scripts) to fail reading from or writing
     to such files.
     As a workaround, it is advised to use the subst.exe command from within a command prompt to set up a local drive
     out of an existing directory on the hard drive, such as:
     C:\> subst X: <PATH_TO_CUBEFW>\Firmware


@par How to use it ?

Execution in a standalone way :
=============================

   - Connect a terminal emulator (Tera Term for example, open source free software terminal emulator
     that can be downloaded from https://osdn.net/projects/ttssh2/) for UART connection with the board.
     Support of YMODEM protocol is required. Serial port configuration should be :
     - Baud rate = 115200
     - Data = 8 bits
     - Parity = none
     - Stop = 1 bit
     - Flow control = none
   - Open your preferred toolchain
   - Rebuild all files of Loader application
     and load your image into the target memory
   - Run the application after compilation


1. At start, "File> Transfer> YMODEM> Send ...." is displayed on Tera Term terminal.

2. To download a firmware, send the user encrypted firmware file (\2_Images\2_Images_UserApp\Binary\UserApp.sfb)
   selecting the menu "File > Transfer > YMODEM > Send..." of Tera Term.

3. After reception of the header part of the firmware, the length and the firmware version (anti-rollback)
   is controlled.
   Then the image is stored in the downloaded area (slot #1) defined in \2_Images\Linker_Common\ mapping files.

4. At the end of the transfer, the number of bytes received is displayed in case of success :
   - "Download successful : 25500 bytes received"
   or an error status in case of failure :
   - "Download failed (4)"

5. Finally, a request of installation is recorded then a reset is generated to start installation.

Please refer to to UM2262 "Getting started with SBSFU - software expansion for STM32Cube" for more detail
on .sfb format.

Note : DEBUG_MODE compilation switch can be activated for more prompts on terminal.

Note for Linux users : Minicom can be used but to do so you need to compile the loader project with the MINICOM_YMODEM switch enabled (loader.h)


Execution through SBSFU :
=======================
Refer to SBSFU readme.txt and follow steps by steps instructions.
