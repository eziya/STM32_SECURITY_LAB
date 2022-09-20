;******************************************************************************
;* File Name          : Loader_Bin.s
;* Author             : MCD Application Team
;* Description        : Include BLE Ota Loader binary.
;*******************************************************************************
;*
;* Copyright (c) 2021 STMicroelectronics.
;* All rights reserved.
;*
;* This software is licensed under terms that can be found in the LICENSE file in
;* the root directory of this software component.
;* If no LICENSE file comes with this software, it is provided AS-IS.
;*
;*******************************************************************************
;
;

 AREA LOADER_Bin, CODE, READONLY
 INCBIN ../../BLE_Ota/MDK-ARM/BLE_Ota\Exe/Loader.bin
 END

