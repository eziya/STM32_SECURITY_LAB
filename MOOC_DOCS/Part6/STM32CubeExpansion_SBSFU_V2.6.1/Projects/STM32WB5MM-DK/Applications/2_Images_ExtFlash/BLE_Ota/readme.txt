/**
  @page BLE_Ota loader
  
  @verbatim
  ******************************************************************************
  * @file    BLE_Ota/readme.txt
  * @author  MCD Application Team
  * @brief   OTA implementation
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

OTA implementation to download a new image into the user flash.

@note Care must be taken when using HAL_Delay(), this function provides accurate delay (in milliseconds)
      based on variable incremented in SysTick ISR. This implies that if HAL_Delay() is called from
      a peripheral ISR process, then the SysTick interrupt must have higher priority (numerically lower)
      than the peripheral interrupt. Otherwise the caller ISR process will be blocked.
      To change the SysTick interrupt priority you have to use HAL_NVIC_SetPriority() function.
      
@note The application needs to ensure that the SysTick time base is always set to 1 millisecond
      to have correct HAL operation.

@par Keywords

Connectivity, BLE, IPCC, HSEM, RTC, UART, PWR, BLE protocol, Dual core

@par Directory contents 

  - BLE_Ota/Core/Inc/app_common.h               Header for all modules with common definition
  - BLE_Ota/Core/Inc/app_conf.h                 Parameters configuration file of the application
  - BLE_Ota/Core/Inc/app_debug.h                Header for the debug interface
  - BLE_Ota/Core/Inc/app_entry.h                Header of the application
  - BLE_Ota/Core/Inc/ext_flash_driver.h         Header of the external flash driver
  - BLE_Ota/Core/Inc/flash_driver.h             Header of the internal flash driver
  - BLE_Ota/Core/Inc/hw_conf.h                  Configuration file of the HW
  - BLE_Ota/Core/Inc/hw_if.h                    Header for the HW interface
  - BLE_Ota/Core/Inc/main.h                     Header for main.c module
  - BLE_Ota/Core/Inc/ota_sbsfu.h                Header for the OTA manager interface
  - BLE_Ota/Core/Inc/s25fl128s_conf.h           Configuration file of the S25FL128S QSPI memory
  - BLE_Ota/Core/Inc/stm32_lpm_if.h             Header for the Low Power Manager Interface
  - BLE_Ota/Core/Inc/stm32wb5mm_dk_conf.h       BSP configuration file
  - BLE_Ota/Core/Inc/stm32wbxx_hal_conf.h       HAL configuration file
  - BLE_Ota/Core/Inc/stm32wbxx_it.h             Interrupt handlers header file
  - BLE_Ota/Core/Inc/utilities_conf.h           Configuration file of the utilities
  - BLE_Ota/Core/Inc/vcp_conf.h                 Configuration of the vcp interface
  - BLE_Ota/Core/Src/app_debug.c                Debug interface
  - BLE_Ota/Core/Src/app_entry.c                Initialization of the application
  - BLE_Ota/Core/Src/ext_flash_driver.c         External flash driver
  - BLE_Ota/Core/Src/flash_driver.c             Internal flash driver
  - BLE_Ota/Core/Src/hw_timerserver.c           Timer Server based on RTC
  - BLE_Ota/Core/Src/hw_uart.c                  UART Driver
  - BLE_Ota/Core/Src/main.c                     Main program
  - BLE_Ota/Core/Src/stm32_lpm_if.c             Low Power Manager Interface
  - BLE_Ota/Core/Src/stm32wbxx_it.c             Interrupt handlers
  - BLE_Ota/Core/Src/system_stm32wbxx.c         stm32wbxx system source file
  - BLE_Ota/STM32_WPAN/App/app_ble.c            BLE Profile implementation
  - BLE_Ota/STM32_WPAN/App/app_ble.h            Header for app_ble.c module
  - BLE_Ota/STM32_WPAN/App/ble_conf.h           BLE Services configuration
  - BLE_Ota/STM32_WPAN/App/ble_dbg_conf.h       BLE Traces configuration of the BLE services
  - BLE_Ota/STM32_WPAN/App/otas_app.c           The OTA service management
  - BLE_Ota/STM32_WPAN/App/tl_dbg_conf.h        Debug configuration file
  - BLE_Ota/STM32_WPAN/Target/hw_ipcc.c         IPCC Driver

     
@par Hardware and Software environment

  - This example runs on STM32WB5Mxx devices.
  
  - This example has been tested with an STMicroelectronics STM32WB5MM-DK
    board and can be easily tailored to any other supported device 
    and development board.

@par How to use it ?

To update the application or the BLE stack, you need to use a P-NUCLEO-WB55 board
(either the Nucleo or the USBDongle).

0. Nucleo or USBDongle device preparation
   You need to download a specific transparent mode application into it.
      If using a Nucleo board, download application through toolchain IDE or STM32 CubeProgrammer in STLink mode.
      If using a USBDongle, download application STM32 CubeProgrammer in USB mode.
   Please refer to the following STM32WB Cube package projects for details:
      Projects\P-NUCLEO-WB55.Nucleo\Applications\BLE\BLE_TransparentMode
      Projects\P-NUCLEO-WB55.USBDongle\Applications\BLE\BLE_TransparentModeVCP

1. Download and install STM32CubeMonitor-RF from st.com

2. Put your SBSFU device in download mode
   Select menu 'Download a new Fw Image'
      Note: In order to work, the BLE_OTA loader requires that the BLE stack is started (it means that CPU2 start address
      is properly set in the option byte 'SBRV'; for more details refer to AN5185 : ST firmware upgrade services for STM32WB Series.

3. Start STM32CubeMonitor-RF
   Select device: choose serial port corresponding to your P-NUCLEO-WB55 board
   Connect to it
   Menu 'Device' -> 'OTA Updater'
   'Search for devices'
   'Select device' -> 'STM_OTA'
   'Target CPU' -> Select 'CPU1:M4' to update the application 'CPU2:M0+' to update the BLE stack
   'Image base address' -> don't care
   'Image file path' -> Choose the file to download
   'Update' and wait for completion


