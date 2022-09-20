;*******************************************************************************
;* File Name          : se_key.s
;* Author             : MCD Application Team
;* Description        : ReadKey from PCROP area.
;*******************************************************************************
;*
;* Copyright (c) 2017 STMicroelectronics.
;* All rights reserved.
;*
;* This software is licensed under terms that can be found in the LICENSE file in
;* the root directory of this software component.
;* If no LICENSE file comes with this software, it is provided AS-IS.
;*
;*******************************************************************************
;
;
; Cortex-M version
;
	section .SE_Key_Data:CODE
	EXPORT SE_ReadKey
SE_ReadKey
	PUSH {R1-R5}
	MOVS R1, #0x5f
	LSLS R1, R1, #24
	MOVS R5, #0x4d
	LSLS R5, R5, #16
	ADD R1, R1, R5
	MOVS R5, #0x45
	LSLS R5, R5, #8
	ADD R1, R1, R5
	MOVS R5, #0x4f
	ADD R1, R1, R5
	MOVS R2, #0x5f
	LSLS R2, R2, #24
	MOVS R5, #0x59
	LSLS R5, R5, #16
	ADD R2, R2, R5
	MOVS R5, #0x45
	LSLS R5, R5, #8
	ADD R2, R2, R5
	MOVS R5, #0x4b
	ADD R2, R2, R5
	MOVS R3, #0x50
	LSLS R3, R3, #24
	MOVS R5, #0x4d
	LSLS R5, R5, #16
	ADD R3, R3, R5
	MOVS R5, #0x4f
	LSLS R5, R5, #8
	ADD R3, R3, R5
	MOVS R5, #0x43
	ADD R3, R3, R5
	MOVS R4, #0x31
	LSLS R4, R4, #24
	MOVS R5, #0x59
	LSLS R5, R5, #16
	ADD R4, R4, R5
	MOVS R5, #0x4e
	LSLS R5, R5, #8
	ADD R4, R4, R5
	MOVS R5, #0x41
	ADD R4, R4, R5
	STM R0!, {R1-R4}
	POP {R1-R5}
	BX LR

	EXPORT SE_ReadKey_Pub
SE_ReadKey_Pub
	PUSH {R1-R5}
	MOVS R1, #0xf8
	LSLS R1, R1, #24
	MOVS R5, #0x97
	LSLS R5, R5, #16
	ADD R1, R1, R5
	MOVS R5, #0xf2
	LSLS R5, R5, #8
	ADD R1, R1, R5
	MOVS R5, #0xba
	ADD R1, R1, R5
	MOVS R2, #0xdc
	LSLS R2, R2, #24
	MOVS R5, #0x7
	LSLS R5, R5, #16
	ADD R2, R2, R5
	MOVS R5, #0xe3
	LSLS R5, R5, #8
	ADD R2, R2, R5
	MOVS R5, #0x3e
	ADD R2, R2, R5
	MOVS R3, #0x78
	LSLS R3, R3, #24
	MOVS R5, #0x71
	LSLS R5, R5, #16
	ADD R3, R3, R5
	MOVS R5, #0xc3
	LSLS R5, R5, #8
	ADD R3, R3, R5
	MOVS R5, #0x16
	ADD R3, R3, R5
	MOVS R4, #0x3e
	LSLS R4, R4, #24
	MOVS R5, #0xb0
	LSLS R5, R5, #16
	ADD R4, R4, R5
	MOVS R5, #0xf1
	LSLS R5, R5, #8
	ADD R4, R4, R5
	MOVS R5, #0x1d
	ADD R4, R4, R5
	STM R0!, {R1-R4}
	MOVS R1, #0x44
	LSLS R1, R1, #24
	MOVS R5, #0xb0
	LSLS R5, R5, #16
	ADD R1, R1, R5
	MOVS R5, #0x95
	LSLS R5, R5, #8
	ADD R1, R1, R5
	MOVS R5, #0xf0
	ADD R1, R1, R5
	MOVS R2, #0x48
	LSLS R2, R2, #24
	MOVS R5, #0x81
	LSLS R5, R5, #16
	ADD R2, R2, R5
	MOVS R5, #0x12
	LSLS R5, R5, #8
	ADD R2, R2, R5
	MOVS R5, #0x54
	ADD R2, R2, R5
	MOVS R3, #0xb9
	LSLS R3, R3, #24
	MOVS R5, #0x66
	LSLS R5, R5, #16
	ADD R3, R3, R5
	MOVS R5, #0x2c
	LSLS R5, R5, #8
	ADD R3, R3, R5
	MOVS R5, #0xfb
	ADD R3, R3, R5
	MOVS R4, #0x4a
	LSLS R4, R4, #24
	MOVS R5, #0xa5
	LSLS R5, R5, #16
	ADD R4, R4, R5
	MOVS R5, #0x3d
	LSLS R5, R5, #8
	ADD R4, R4, R5
	MOVS R5, #0x54
	ADD R4, R4, R5
	STM R0!, {R1-R4}
	MOVS R1, #0x76
	LSLS R1, R1, #24
	MOVS R5, #0x4
	LSLS R5, R5, #16
	ADD R1, R1, R5
	MOVS R5, #0x26
	LSLS R5, R5, #8
	ADD R1, R1, R5
	MOVS R5, #0xe8
	ADD R1, R1, R5
	MOVS R2, #0x3c
	LSLS R2, R2, #24
	MOVS R5, #0x8b
	LSLS R5, R5, #16
	ADD R2, R2, R5
	MOVS R5, #0x37
	LSLS R5, R5, #8
	ADD R2, R2, R5
	MOVS R5, #0xb7
	ADD R2, R2, R5
	MOVS R3, #0x6a
	LSLS R3, R3, #24
	MOVS R5, #0xfd
	LSLS R5, R5, #16
	ADD R3, R3, R5
	MOVS R5, #0xd8
	LSLS R5, R5, #8
	ADD R3, R3, R5
	MOVS R5, #0x46
	ADD R3, R3, R5
	MOVS R4, #0x46
	LSLS R4, R4, #24
	MOVS R5, #0x7c
	LSLS R5, R5, #16
	ADD R4, R4, R5
	MOVS R5, #0x61
	LSLS R5, R5, #8
	ADD R4, R4, R5
	MOVS R5, #0x63
	ADD R4, R4, R5
	STM R0!, {R1-R4}
	MOVS R1, #0x46
	LSLS R1, R1, #24
	MOVS R5, #0xe9
	LSLS R5, R5, #16
	ADD R1, R1, R5
	MOVS R5, #0x7d
	LSLS R5, R5, #8
	ADD R1, R1, R5
	MOVS R5, #0xc3
	ADD R1, R1, R5
	MOVS R2, #0xd7
	LSLS R2, R2, #24
	MOVS R5, #0x6e
	LSLS R5, R5, #16
	ADD R2, R2, R5
	MOVS R5, #0x31
	LSLS R5, R5, #8
	ADD R2, R2, R5
	MOVS R5, #0x44
	ADD R2, R2, R5
	MOVS R3, #0x70
	LSLS R3, R3, #24
	MOVS R5, #0xba
	LSLS R5, R5, #16
	ADD R3, R3, R5
	MOVS R5, #0x6d
	LSLS R5, R5, #8
	ADD R3, R3, R5
	MOVS R5, #0xe1
	ADD R3, R3, R5
	MOVS R4, #0x2
	LSLS R4, R4, #24
	MOVS R5, #0xba
	LSLS R5, R5, #16
	ADD R4, R4, R5
	MOVS R5, #0x44
	LSLS R5, R5, #8
	ADD R4, R4, R5
	MOVS R5, #0xed
	ADD R4, R4, R5
	STM R0!, {R1-R4}
	POP {R1-R5}
	BX LR
	END
