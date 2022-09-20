;*******************************************************************************
;* File Name          : startup_stm32l476xx.s
;* Author             : MCD Application Team
;* Description        : STM32L476xx Ultra Low Power Devices vector
;*                      This module performs:
;*                      - Set the initial SP
;*                      - Set the initial PC == _iar_program_start,
;*                      - Set the vector table entries with the exceptions ISR
;*                        address.
;*                      - Branches to main in the C library (which eventually
;*                        calls main()).
;*                      After Reset the Cortex-M4 processor is in Thread mode,
;*                      priority is Privileged, and the Stack is set to Main.
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
; The modules in this file are included in the libraries, and may be replaced
; by any user-defined modules that define the PUBLIC symbol _program_start or
; a user defined start symbol.
; To override the cstartup defined in the library, simply add your modified
; version to the workbench project.
;
; The vector table is normally located at address 0.
; When debugging in RAM, it can be located in RAM, aligned to at least 2^6.
; The name "__vector_table" has special meaning for C-SPY:
; it is where the SP start value is found, and the NVIC vector
; table register (VTOR) is initialized to this address if != 0.
;
; Cortex-M version
;

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)

        PUBLIC  __vector_table

        DATA
__vector_table
        DCD     0
        DCD     Reset_Handler             ; Reset Handler

        DCD     NMI_Handler               ; NMI Handler
        DCD     HardFault_Handler         ; Hard Fault Handler
        DCD     MemManage_Handler         ; MPU Fault Handler
        DCD     BusFault_Handler          ; Bus Fault Handler
        DCD     UsageFault_Handler        ; Usage Fault Handler
        DCD     0                         ; Reserved
        DCD     0                         ; Reserved
        DCD     0                         ; Reserved
        DCD     0                         ; Reserved
        DCD     SVC_Handler               ; SVCall Handler
        DCD     DebugMon_Handler          ; Debug Monitor Handler
        DCD     0                         ; Reserved
        DCD     PendSV_Handler            ; PendSV Handler
        DCD     SysTick_Handler           ; SysTick Handler

         ; External Interrupts
        DCD     WWDG_IRQHandler                   ; Window WatchDog
        DCD     PVD_PVM_IRQHandler                ; PVD/PVM1/PVM2/PVM3/PVM4 through EXTI Line detection
        DCD     TAMP_STAMP_IRQHandler             ; Tamper and TimeStamps through the EXTI line
        DCD     RTC_WKUP_IRQHandler               ; RTC Wakeup through the EXTI line
        DCD     FLASH_IRQHandler                  ; FLASH
        DCD     RCC_IRQHandler                    ; RCC
        DCD     EXTI0_IRQHandler                  ; EXTI Line0
        DCD     EXTI1_IRQHandler                  ; EXTI Line1
        DCD     EXTI2_IRQHandler                  ; EXTI Line2
        DCD     EXTI3_IRQHandler                  ; EXTI Line3
        DCD     EXTI4_IRQHandler                  ; EXTI Line4
        DCD     DMA1_Channel1_IRQHandler          ; DMA1 Channel 1
        DCD     DMA1_Channel2_IRQHandler          ; DMA1 Channel 2
        DCD     DMA1_Channel3_IRQHandler          ; DMA1 Channel 3
        DCD     DMA1_Channel4_IRQHandler          ; DMA1 Channel 4
        DCD     DMA1_Channel5_IRQHandler          ; DMA1 Channel 5
        DCD     DMA1_Channel6_IRQHandler          ; DMA1 Channel 6
        DCD     DMA1_Channel7_IRQHandler          ; DMA1 Channel 7
        DCD     ADC1_2_IRQHandler                 ; ADC1, ADC2
        DCD     CAN1_TX_IRQHandler                ; CAN1 TX
        DCD     CAN1_RX0_IRQHandler               ; CAN1 RX0
        DCD     CAN1_RX1_IRQHandler               ; CAN1 RX1
        DCD     CAN1_SCE_IRQHandler               ; CAN1 SCE
        DCD     EXTI9_5_IRQHandler                ; External Line[9:5]s
        DCD     TIM1_BRK_TIM15_IRQHandler         ; TIM1 Break and TIM15
        DCD     TIM1_UP_TIM16_IRQHandler          ; TIM1 Update and TIM16
        DCD     TIM1_TRG_COM_TIM17_IRQHandler     ; TIM1 Trigger and Commutation and TIM17
        DCD     TIM1_CC_IRQHandler                ; TIM1 Capture Compare
        DCD     TIM2_IRQHandler                   ; TIM2
        DCD     TIM3_IRQHandler                   ; TIM3
        DCD     TIM4_IRQHandler                   ; TIM4
        DCD     I2C1_EV_IRQHandler                ; I2C1 Event
        DCD     I2C1_ER_IRQHandler                ; I2C1 Error
        DCD     I2C2_EV_IRQHandler                ; I2C2 Event
        DCD     I2C2_ER_IRQHandler                ; I2C2 Error
        DCD     SPI1_IRQHandler                   ; SPI1
        DCD     SPI2_IRQHandler                   ; SPI2
        DCD     USART1_IRQHandler                 ; USART1
        DCD     USART2_IRQHandler                 ; USART2
        DCD     USART3_IRQHandler                 ; USART3
        DCD     EXTI15_10_IRQHandler              ; External Line[15:10]
        DCD     RTC_Alarm_IRQHandler              ; RTC Alarm (A and B) through EXTI Line
        DCD     DFSDM1_FLT3_IRQHandler            ; DFSDM1 Filter 3 global Interrupt
        DCD     TIM8_BRK_IRQHandler               ; TIM8 Break Interrupt
        DCD     TIM8_UP_IRQHandler                ; TIM8 Update Interrupt
        DCD     TIM8_TRG_COM_IRQHandler           ; TIM8 Trigger and Commutation Interrupt
        DCD     TIM8_CC_IRQHandler                ; TIM8 Capture Compare Interrupt
        DCD     ADC3_IRQHandler                   ; ADC3 global  Interrupt
        DCD     FMC_IRQHandler                    ; FMC
        DCD     SDMMC1_IRQHandler                 ; SDMMC1
        DCD     TIM5_IRQHandler                   ; TIM5
        DCD     SPI3_IRQHandler                   ; SPI3
        DCD     UART4_IRQHandler                  ; UART4
        DCD     UART5_IRQHandler                  ; UART5
        DCD     TIM6_DAC_IRQHandler               ; TIM6 and DAC1&2 underrun errors
        DCD     TIM7_IRQHandler                   ; TIM7
        DCD     DMA2_Channel1_IRQHandler          ; DMA2 Channel 1
        DCD     DMA2_Channel2_IRQHandler          ; DMA2 Channel 2
        DCD     DMA2_Channel3_IRQHandler          ; DMA2 Channel 3
        DCD     DMA2_Channel4_IRQHandler          ; DMA2 Channel 4
        DCD     DMA2_Channel5_IRQHandler          ; DMA2 Channel 5
        DCD     DFSDM1_FLT0_IRQHandler            ; DFSDM1 Filter 0 global Interrupt
        DCD     DFSDM1_FLT1_IRQHandler            ; DFSDM1 Filter 1 global Interrupt
        DCD     DFSDM1_FLT2_IRQHandler            ; DFSDM1 Filter 2 global Interrupt
        DCD     COMP_IRQHandler                   ; COMP Interrupt
        DCD     LPTIM1_IRQHandler                 ; LP TIM1 interrupt
        DCD     LPTIM2_IRQHandler                 ; LP TIM2 interrupt
        DCD     OTG_FS_IRQHandler                 ; USB OTG FS
        DCD     DMA2_Channel6_IRQHandler          ; DMA2 Channel 6
        DCD     DMA2_Channel7_IRQHandler          ; DMA2 Channel 7
        DCD     LPUART1_IRQHandler                ; LP UART 1 interrupt
        DCD     QUADSPI_IRQHandler                ; Quad SPI global interrupt
        DCD     I2C3_EV_IRQHandler                ; I2C3 event
        DCD     I2C3_ER_IRQHandler                ; I2C3 error
        DCD     SAI1_IRQHandler                   ; Serial Audio Interface 1 global interrupt
        DCD     SAI2_IRQHandler                   ; Serial Audio Interface 2 global interrupt
        DCD     SWPMI1_IRQHandler                 ; Serial Wire Interface global interrupt
        DCD     TSC_IRQHandler                    ; Touch Sense Controller global interrupt
        DCD     LCD_IRQHandler                    ; LCD global interrupt
        DCD     0                                 ; Reserved
        DCD     RNG_IRQHandler                    ; RNG global interrupt
        DCD     FPU_IRQHandler                    ; FPU

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
        THUMB
        PUBWEAK Reset_Handler
        SECTION .text:CODE:NOROOT:REORDER(2)
Reset_Handler
        B Reset_Handler

        PUBWEAK NMI_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
NMI_Handler
        B NMI_Handler

        PUBWEAK HardFault_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
HardFault_Handler
        B HardFault_Handler

        PUBWEAK MemManage_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
MemManage_Handler
        B MemManage_Handler

        PUBWEAK BusFault_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
BusFault_Handler
        B BusFault_Handler

        PUBWEAK UsageFault_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
UsageFault_Handler
        B UsageFault_Handler

        PUBWEAK SVC_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
SVC_Handler
        B SVC_Handler

        PUBWEAK DebugMon_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
DebugMon_Handler
        B DebugMon_Handler

        PUBWEAK PendSV_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
PendSV_Handler
        B PendSV_Handler

        PUBWEAK SysTick_Handler
        SECTION .text:CODE:NOROOT:REORDER(1)
SysTick_Handler
        B SE_Handler

        PUBWEAK WWDG_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
WWDG_IRQHandler
        B SE_Handler

        PUBWEAK PVD_PVM_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
PVD_PVM_IRQHandler
        B SE_Handler

        PUBWEAK TAMP_STAMP_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TAMP_STAMP_IRQHandler
        B SE_Handler

        PUBWEAK RTC_WKUP_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
RTC_WKUP_IRQHandler
        B SE_Handler

        PUBWEAK FLASH_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
FLASH_IRQHandler
        B SE_Handler

        PUBWEAK RCC_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
RCC_IRQHandler
        B SE_Handler

        PUBWEAK EXTI0_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
EXTI0_IRQHandler
        B SE_Handler

        PUBWEAK EXTI1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
EXTI1_IRQHandler
        B SE_Handler

        PUBWEAK EXTI2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
EXTI2_IRQHandler
        B SE_Handler

        PUBWEAK EXTI3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
EXTI3_IRQHandler
        B SE_Handler

        PUBWEAK EXTI4_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
EXTI4_IRQHandler
        B SE_Handler

        PUBWEAK DMA1_Channel1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA1_Channel1_IRQHandler
        B SE_Handler

        PUBWEAK DMA1_Channel2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA1_Channel2_IRQHandler
        B SE_Handler

        PUBWEAK DMA1_Channel3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA1_Channel3_IRQHandler
        B SE_Handler

        PUBWEAK DMA1_Channel4_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA1_Channel4_IRQHandler
        B SE_Handler

        PUBWEAK DMA1_Channel5_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA1_Channel5_IRQHandler
        B SE_Handler

        PUBWEAK DMA1_Channel6_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA1_Channel6_IRQHandler
        B SE_Handler

        PUBWEAK DMA1_Channel7_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA1_Channel7_IRQHandler
        B SE_Handler

        PUBWEAK ADC1_2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
ADC1_2_IRQHandler
        B SE_Handler

        PUBWEAK CAN1_TX_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
CAN1_TX_IRQHandler
        B SE_Handler

        PUBWEAK CAN1_RX0_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
CAN1_RX0_IRQHandler
        B SE_Handler

        PUBWEAK CAN1_RX1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
CAN1_RX1_IRQHandler
        B SE_Handler

        PUBWEAK CAN1_SCE_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
CAN1_SCE_IRQHandler
        B SE_Handler

        PUBWEAK EXTI9_5_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
EXTI9_5_IRQHandler
        B SE_Handler

        PUBWEAK TIM1_BRK_TIM15_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM1_BRK_TIM15_IRQHandler
        B SE_Handler

        PUBWEAK TIM1_UP_TIM16_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM1_UP_TIM16_IRQHandler
        B SE_Handler

        PUBWEAK TIM1_TRG_COM_TIM17_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM1_TRG_COM_TIM17_IRQHandler
        B SE_Handler

        PUBWEAK TIM1_CC_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM1_CC_IRQHandler
        B SE_Handler

        PUBWEAK TIM2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM2_IRQHandler
        B SE_Handler

        PUBWEAK TIM3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM3_IRQHandler
        B SE_Handler

        PUBWEAK TIM4_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM4_IRQHandler
        B SE_Handler

        PUBWEAK I2C1_EV_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
I2C1_EV_IRQHandler
        B SE_Handler

        PUBWEAK I2C1_ER_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
I2C1_ER_IRQHandler
        B SE_Handler

        PUBWEAK I2C2_EV_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
I2C2_EV_IRQHandler
        B SE_Handler

        PUBWEAK I2C2_ER_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
I2C2_ER_IRQHandler
        B SE_Handler

        PUBWEAK SPI1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
SPI1_IRQHandler
        B SE_Handler

        PUBWEAK SPI2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
SPI2_IRQHandler
        B SE_Handler

        PUBWEAK USART1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
USART1_IRQHandler
        B SE_Handler

        PUBWEAK USART2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
USART2_IRQHandler
        B SE_Handler

        PUBWEAK USART3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
USART3_IRQHandler
        B SE_Handler

        PUBWEAK EXTI15_10_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
EXTI15_10_IRQHandler
        B SE_Handler

        PUBWEAK RTC_Alarm_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
RTC_Alarm_IRQHandler
        B SE_Handler

        PUBWEAK DFSDM1_FLT3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DFSDM1_FLT3_IRQHandler
        B SE_Handler

        PUBWEAK TIM8_BRK_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM8_BRK_IRQHandler
        B SE_Handler

        PUBWEAK TIM8_UP_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM8_UP_IRQHandler
        B SE_Handler

        PUBWEAK TIM8_TRG_COM_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM8_TRG_COM_IRQHandler
        B SE_Handler

        PUBWEAK TIM8_CC_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM8_CC_IRQHandler
        B SE_Handler

        PUBWEAK ADC3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
ADC3_IRQHandler
        B SE_Handler

        PUBWEAK FMC_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
FMC_IRQHandler
        B SE_Handler

        PUBWEAK SDMMC1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
SDMMC1_IRQHandler
        B SE_Handler

        PUBWEAK TIM5_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM5_IRQHandler
        B SE_Handler

        PUBWEAK SPI3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
SPI3_IRQHandler
        B SE_Handler

        PUBWEAK UART4_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
UART4_IRQHandler
        B SE_Handler

        PUBWEAK UART5_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
UART5_IRQHandler
        B SE_Handler

        PUBWEAK TIM6_DAC_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM6_DAC_IRQHandler
        B SE_Handler

        PUBWEAK TIM7_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
TIM7_IRQHandler
        B SE_Handler

        PUBWEAK DMA2_Channel1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA2_Channel1_IRQHandler
        B SE_Handler

        PUBWEAK DMA2_Channel2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA2_Channel2_IRQHandler
        B SE_Handler

        PUBWEAK DMA2_Channel3_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA2_Channel3_IRQHandler
        B SE_Handler

        PUBWEAK DMA2_Channel4_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA2_Channel4_IRQHandler
        B SE_Handler

        PUBWEAK DMA2_Channel5_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA2_Channel5_IRQHandler
        B SE_Handler

        PUBWEAK DFSDM1_FLT0_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DFSDM1_FLT0_IRQHandler
        B SE_Handler

        PUBWEAK DFSDM1_FLT1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DFSDM1_FLT1_IRQHandler
        B SE_Handler

        PUBWEAK DFSDM1_FLT2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DFSDM1_FLT2_IRQHandler
        B SE_Handler

        PUBWEAK COMP_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
COMP_IRQHandler
        B SE_Handler

        PUBWEAK LPTIM1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
LPTIM1_IRQHandler
        B SE_Handler

        PUBWEAK LPTIM2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
LPTIM2_IRQHandler
        B SE_Handler

        PUBWEAK OTG_FS_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
OTG_FS_IRQHandler
        B SE_Handler

        PUBWEAK DMA2_Channel6_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA2_Channel6_IRQHandler
        B SE_Handler

        PUBWEAK DMA2_Channel7_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
DMA2_Channel7_IRQHandler
        B SE_Handler

        PUBWEAK LPUART1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
LPUART1_IRQHandler
        B SE_Handler

        PUBWEAK QUADSPI_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
QUADSPI_IRQHandler
        B SE_Handler

        PUBWEAK I2C3_EV_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
I2C3_EV_IRQHandler
        B SE_Handler

        PUBWEAK I2C3_ER_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
I2C3_ER_IRQHandler
        B SE_Handler

        PUBWEAK SAI1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
SAI1_IRQHandler
        B SE_Handler

       PUBWEAK SAI2_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
SAI2_IRQHandler
        B SE_Handler

        PUBWEAK SWPMI1_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
SWPMI1_IRQHandler
        B SE_Handler

       PUBWEAK TSC_IRQHandler
       SECTION .text:CODE:NOROOT:REORDER(1)
TSC_IRQHandler
        B SE_Handler

       PUBWEAK LCD_IRQHandler
       SECTION .text:CODE:NOROOT:REORDER(1)
LCD_IRQHandler
        B SE_Handler

        PUBWEAK RNG_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
RNG_IRQHandler
        B SE_Handler

        PUBWEAK FPU_IRQHandler
        SECTION .text:CODE:NOROOT:REORDER(1)
FPU_IRQHandler
        B SE_Handler

        SECTION .text:CODE:NOROOT:REORDER(2)
        IMPORT SeExcEntrySp
        IMPORT IntHand
        IMPORT SeExcReturn
        IMPORT SeMsp
        IMPORT AppliVectorsAddr
        IMPORT AppliMsp
        IMPORT ScbVtorAddr
        IMPORT PrimaskValue
        IMPORT FirewallCrAddr
        IMPORT SE_UserHandlerWrapperAddr

; ******************************************
; Function Name  : SE_Handler
; Description    : switch SE to Appli context & call SE_UserHandlerWrapperAddr
; input          : None
; internal       : R0, R1, R2, R3
; output         : R0 : @ Appli IT Handler
;                : R1 : Primask value
; return         : none
; ******************************************
SE_Handler
; disable interrupts
        CPSID i
; save SE Active SP
        MOV R0, SP
        LDR R1, =SeExcEntrySp
        STR R0, [R1]
; save non scratch core registers in SE Stack
        PUSH {R4 - R11}
; save non scratch FPU registers in SE stack
        TST LR, #0x10 ; test if FPCA = 1
        IT EQ
        VPUSHEQ {S16-S31}
; save Active SP to SeMsp
        MOV R0, SP
        LDR R1, =SeMsp
        STR R0, [R1]
; set interrupt handling flag
        MOV R0, #0x00000001UL
        LDR R1, =IntHand
        STR R0, [R1]
; save Handler Mode Exec Return
        LDR R1, =SeExcReturn
        STR LR, [R1]
; set Active SP to Appli MSP
        LDR R0, =AppliMsp
        LDR R0, [R0]
        MOV SP, R0
; set Core Vectors Table to Appli Vectors Table
        LDR R0, =AppliVectorsAddr
        LDR R0, [R0] ; R0 = @ Appli Vectors Table
        LDR R1, =ScbVtorAddr
        LDR R1, [R1] ; R1 = @ Core Vectors Table
        STR R0, [R1]
; set SE_UserHandlerWrapperAddr 1st input param: @ Appli IT Handler
        MRS R2, IPSR ; R2 =  ISR number
        LSL R2, R2, #2 ; R2 = R2 * 4 (bytes offset)
        ADD R0, R0, R2 ; R0 = offset of IT handler in Appli Vectors Table
        LDR R0, [R0] ; R0 = @ Appli IT Handler
; set SE_UserHandlerWrapperAddr 2nd input param: Primask
        LDR R1, =PrimaskValue
        LDR R1, [R1] ; R1 = Primask value
; clean core registers
        ; do not clean R0 & R1: they are used to call SE Handler Wrapper
        ; no need to clean R2, R3: they are used after
        MOV R4, #0
        MOV R5, #0
        MOV R6, #0
        MOV R7, #0
        MOV R8, #0
        MOV R9, #0
        MOV R10, #0
        MOV R11, #0
        TST LR, #0x10 ; test if FPCA = 1
        BNE CLOSE_FWALL_JUMP_USER_HANDLER_WRAPPER
        VMOV.F32 S0, #1.0
        VMOV.F32 S1, #1.0
        VMOV.F32 S2, #1.0
        VMOV.F32 S3, #1.0
        VMOV.F32 S4, #1.0
        VMOV.F32 S5, #1.0
        VMOV.F32 S6, #1.0
        VMOV.F32 S7, #1.0
        VMOV.F32 S8, #1.0
        VMOV.F32 S9, #1.0
        VMOV.F32 S10, #1.0
        VMOV.F32 S11, #1.0
        VMOV.F32 S12, #1.0
        VMOV.F32 S13, #1.0
        VMOV.F32 S14, #1.0
        VMOV.F32 S15, #1.0
        VMOV.F32 S16, #1.0
        VMOV.F32 S17, #1.0
        VMOV.F32 S18, #1.0
        VMOV.F32 S19, #1.0
        VMOV.F32 S20, #1.0
        VMOV.F32 S21, #1.0
        VMOV.F32 S22, #1.0
        VMOV.F32 S23, #1.0
        VMOV.F32 S24, #1.0
        VMOV.F32 S25, #1.0
        VMOV.F32 S26, #1.0
        VMOV.F32 S27, #1.0
        VMOV.F32 S28, #1.0
        VMOV.F32 S29, #1.0
        VMOV.F32 S30, #1.0
        VMOV.F32 S31, #1.0
CLOSE_FWALL_JUMP_USER_HANDLER_WRAPPER
; set Firewall FPA to 1
        LDR R2, =FirewallCrAddr
        LDR R2, [R2] ; R2 = address of FIREWALL->CR
        LDR R3, [R2] ; R3 = content of FIREWALL->CR
        ORR R3, R3, #0x00000001UL
        STR R3, [R2]
        DMB
        LDR R3, [R2] ; dummy read of FPA register to ensure it is written
        MOV R2, #0   ; clean register
        MOV R3, #0   ; clean register
; jump to to SE Handler Wrapper
        LDR R2, =SE_UserHandlerWrapperAddr
        LDR R2, [R2]
        BX R2
; we shall not raise this point
        ; B Reset_Handler

        END
