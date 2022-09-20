;/******************************************************************************
;* File Name          : sfu_se_mpu.s
;* Author             : MCD Application Team
;* Description        : Wrapper for SE isolation with MPU.
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
      PRESERVE8
      THUMB
      AREA    |.text|, CODE, READONLY
      EXPORT SVC_Handler
SVC_Handler
        IMPORT MPU_SVC_Handler
        MRS r0, PSP
        B MPU_SVC_Handler

      EXPORT jump_to_function
jump_to_function
        LDR SP, [R0]
        LDR PC, [R0,#4]

      EXPORT launch_application
launch_application
        ; return from exception to application launch function
        ; R0: application vector address
        ; R1: exit function address
        ; push interrupt context R0 R1 R2 R3 R12 LR PC xPSR
        MOV R2, #0x01000000 ; xPSR activate Thumb bit
        PUSH {R2} ; FLAGS=0
        MOV R2, #1
        BIC R1, R1, R2  ; clear least significant bit of exit function
        PUSH {R1}  ; return address = application entry point
        MOV R1, #0 ; clear other context registers
        PUSH {R1} ; LR =0
        PUSH {R1} ; R12 =0
        PUSH {R1} ; R3 = 0
        PUSH {R1} ; R2 = 0
        PUSH {R1} ; R1 = 0
        ; set R0 to application entry point
        PUSH {R0} ; R0 = application entry point
        ; set LR to return to thread mode with main stack
        MOV LR, #0xFFFFFFF9
        ; return from interrupt
        BX LR

        ALIGN
        END

