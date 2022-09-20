;*******************************************************************************
;* File Name          : se_key.s
;* Author             : MCD Application Team
;* Description        : ReadKey from PCROP area
;* <<< Use Configuration Wizard in Context Menu >>>
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

	AREA |.SE_Key_Data|, CODE
	EXPORT SE_ReadKey_Pairing
SE_ReadKey_Pairing
	PUSH {R1-R5}
	MOVW R1, #0x1100
	MOVT R1, #0x3322
	MOVW R2, #0x5544
	MOVT R2, #0x7766
	MOVW R3, #0x9988
	MOVT R3, #0xbbaa
	MOVW R4, #0xddcc
	MOVT R4, #0xffee
	STM R0, {R1-R4}
	ADD R0, R0,#16
	MOVW R1, #0x1111
	MOVT R1, #0x2222
	MOVW R2, #0x3333
	MOVT R2, #0x4444
	MOVW R3, #0x5555
	MOVT R3, #0x6666
	MOVW R4, #0x7777
	MOVT R4, #0x8888
	STM R0, {R1-R4}
	POP {R1-R5}
	BX LR

    END

