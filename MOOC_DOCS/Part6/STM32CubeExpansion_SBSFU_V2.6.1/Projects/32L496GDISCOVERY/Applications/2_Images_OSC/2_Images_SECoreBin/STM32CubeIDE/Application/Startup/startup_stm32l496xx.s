/**
  ******************************************************************************
  * @file      startup_stm32l496xx.s
  * @author    MCD Application Team
  * @brief     STM32L496xx devices vector table GCC toolchain.
  *            This module performs:
  *                - Set the initial SP
  *                - Set the initial PC == Reset_Handler,
  *                - Set the vector table entries with the exceptions ISR address,
  *                - Configure the clock system
  *                - Branches to main in the C library (which eventually
  *                  calls main()).
  *            After Reset the Cortex-M4 processor is in Thread mode,
  *            priority is Privileged, and the Stack is set to Main.
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
  */

  .syntax unified
	.cpu cortex-m4
//	.fpu softvfp ==> VPUSHEQ instructions not available with softvfp mode
	.thumb

.global	g_pfnVectors
.global	Default_Handler
.global	SE_Handler

/* start address for the initialization values of the .data section.
defined in linker script */
.word	_sidata
/* start address for the .data section. defined in linker script */
.word	_sdata
/* end address for the .data section. defined in linker script */
.word	_edata
/* start address for the .bss section. defined in linker script */
.word	_sbss
/* end address for the .bss section. defined in linker script */
.word	_ebss

.equ  BootRAM,        0xF1E0F85F
/**
 * @brief  This is the code that gets called when the processor first
 *          starts execution following a reset event. Only the absolutely
 *          necessary set is performed, after which the application
 *          supplied main() routine is called.
 * @param  None
 * @retval : None
*/

    .section	.text.Reset_Handler
	.weak	Reset_Handler
	.type	Reset_Handler, %function
Reset_Handler:
    Infinite_Loop:
	b	Infinite_Loop
    .size	Reset_Handler, .-Reset_Handler

/**
 * @brief  This is the code that gets called when the processor receives an
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 *
 * @param  None
 * @retval : None
*/
    .section	.text.Default_Handler,"ax",%progbits
Default_Handler:
/*Infinite_Loop:*/
	b	Infinite_Loop
	.size	Default_Handler, .-Default_Handler

 /*******************************************
 * Function Name  : SE_Handler
 * Description    : switch SE to Appli context & call SE_UserHandlerWrapperAddr
 * input          : None
 * internal       : R0, R1, R2, R3
 * output         : R0 : @ Appli IT Handler
 *                : R1 : Primask value
 * return         : none
 *******************************************
 */
    .section .text.SE_Handler

    .global SeExcEntrySp
    .global IntHand
    .global SeExcReturn
    .global SeMsp
    .global AppliVectorsAddr
    .global AppliMsp
    .global ScbVtorAddr
    .global PrimaskValue
    .global FirewallCrAddr
    .global SE_UserHandlerWrapperAddr

SE_Handler:
    BX LR
// disable interrupts
    CPSID i
// save SE Active SP
    MOV R0, SP
    LDR R1, =SeExcEntrySp
    STR R0, [R1]
// save non scratch core registers in SE Stack
    PUSH {R4 - R11}
// save non scratch FPU registers in SE stack
    TST LR, #0x10 // test if FPCA = 1
    IT EQ
    VPUSHEQ {S16-S31}
// save Active SP to SeMsp
    MOV R0, SP
    LDR R1, =SeMsp
    STR R0, [R1]
// set interrupt handling flag
    MOV R0, #0x00000001UL
    LDR R1, =IntHand
    STR R0, [R1]
// save Handler Mode Exec Return
    LDR R1, =SeExcReturn
    STR LR, [R1]
// set Active SP to Appli MSP
    LDR R0, =AppliMsp
    LDR R0, [R0]
    MOV SP, R0
// set Core Vectors Table to Appli Vectors Table
    LDR R0, =AppliVectorsAddr
    LDR R0, [R0] // R0 = @ Appli Vectors Table
    LDR R1, =ScbVtorAddr
    LDR R1, [R1] // R1 = @ Core Vectors Table
    STR R0, [R1]
// set SE_UserHandlerWrapperAddr 1st input param: @ Appli IT Handler
    MRS R2, IPSR // R2 =  ISR number
    ISB
    LSL R2, R2, #2 // R2 = R2 * 4 (bytes offset)
    ADD R0, R0, R2 // R0 = offset of IT handler in Appli Vectors Table
    LDR R0, [R0] // R0 = @ Appli IT Handler
// set SE_UserHandlerWrapperAddr 2nd input param: Primask
    LDR R1, =PrimaskValue
    LDR R1, [R1] // R1 = Primask value
// clean core registers
    // do not clean R0 & R1: they are used to call SE Handler Wrapper
    // no need to clean R2, R3: they are used after
    MOV R4, #0
    MOV R5, #0
    MOV R6, #0
    MOV R7, #0
    MOV R8, #0
    MOV R9, #0
    MOV R10, #0
    MOV R11, #0
    TST LR, #0x10 // test if FPCA = 1
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
CLOSE_FWALL_JUMP_USER_HANDLER_WRAPPER:
// set Firewall FPA to 1
    LDR R2, =FirewallCrAddr
    LDR R2, [R2] // R2 = address of FIREWALL->CR
    LDR R3, [R2] // R3 = content of FIREWALL->CR
    ORR R3, R3, #0x00000001UL
    STR R3, [R2]
    DMB
    LDR R3, [R2] // dummy read of FPA register to ensure it is written
    MOV R2, #0   // clean register
    MOV R3, #0   // clean register
// jump to to SE Handler Wrapper
    LDR R2, =SE_UserHandlerWrapperAddr
    LDR R2, [R2]
    BX R2
// we shall not raise this point
    // B Reset_Handler
    .size SE_Handler, .-SE_Handler

/******************************************************************************
*
* The minimal vector table for a Cortex-M4.  Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
*
******************************************************************************/
 	.section	.isr_vector,"a",%progbits
	.type	g_pfnVectors, %object
	.size	g_pfnVectors, .-g_pfnVectors


g_pfnVectors:
	.word	0
	.word	Reset_Handler
	.word	NMI_Handler
	.word	HardFault_Handler
	.word	MemManage_Handler
	.word	BusFault_Handler
	.word	UsageFault_Handler
	.word	0
	.word	0
	.word	0
	.word	0
	.word	SVC_Handler
	.word	DebugMon_Handler
	.word	0
	.word	PendSV_Handler
	.word	SysTick_Handler
	.word	WWDG_IRQHandler
	.word	PVD_PVM_IRQHandler
	.word	TAMP_STAMP_IRQHandler
	.word	RTC_WKUP_IRQHandler
	.word	FLASH_IRQHandler
	.word	RCC_IRQHandler
	.word	EXTI0_IRQHandler
	.word	EXTI1_IRQHandler
	.word	EXTI2_IRQHandler
	.word	EXTI3_IRQHandler
	.word	EXTI4_IRQHandler
	.word	DMA1_Channel1_IRQHandler
	.word	DMA1_Channel2_IRQHandler
	.word	DMA1_Channel3_IRQHandler
	.word	DMA1_Channel4_IRQHandler
	.word	DMA1_Channel5_IRQHandler
	.word	DMA1_Channel6_IRQHandler
	.word	DMA1_Channel7_IRQHandler
	.word	ADC1_2_IRQHandler
	.word	CAN1_TX_IRQHandler
	.word	CAN1_RX0_IRQHandler
	.word	CAN1_RX1_IRQHandler
	.word	CAN1_SCE_IRQHandler
	.word	EXTI9_5_IRQHandler
	.word	TIM1_BRK_TIM15_IRQHandler
	.word	TIM1_UP_TIM16_IRQHandler
	.word	TIM1_TRG_COM_TIM17_IRQHandler
	.word	TIM1_CC_IRQHandler
	.word	TIM2_IRQHandler
	.word	TIM3_IRQHandler
	.word	TIM4_IRQHandler
	.word	I2C1_EV_IRQHandler
	.word	I2C1_ER_IRQHandler
	.word	I2C2_EV_IRQHandler
	.word	I2C2_ER_IRQHandler
	.word	SPI1_IRQHandler
	.word	SPI2_IRQHandler
	.word	USART1_IRQHandler
	.word	USART2_IRQHandler
	.word	USART3_IRQHandler
	.word	EXTI15_10_IRQHandler
	.word	RTC_Alarm_IRQHandler
	.word	DFSDM1_FLT3_IRQHandler
	.word	TIM8_BRK_IRQHandler
	.word	TIM8_UP_IRQHandler
	.word	TIM8_TRG_COM_IRQHandler
	.word	TIM8_CC_IRQHandler
	.word	ADC3_IRQHandler
	.word	FMC_IRQHandler
	.word	SDMMC1_IRQHandler
	.word	TIM5_IRQHandler
	.word	SPI3_IRQHandler
	.word	UART4_IRQHandler
	.word	UART5_IRQHandler
	.word	TIM6_DAC_IRQHandler
	.word	TIM7_IRQHandler
	.word	DMA2_Channel1_IRQHandler
	.word	DMA2_Channel2_IRQHandler
	.word	DMA2_Channel3_IRQHandler
	.word	DMA2_Channel4_IRQHandler
	.word	DMA2_Channel5_IRQHandler
	.word	DFSDM1_FLT0_IRQHandler
	.word	DFSDM1_FLT1_IRQHandler
	.word	DFSDM1_FLT2_IRQHandler
	.word	COMP_IRQHandler
	.word	LPTIM1_IRQHandler
	.word	LPTIM2_IRQHandler
	.word	OTG_FS_IRQHandler
	.word	DMA2_Channel6_IRQHandler
	.word	DMA2_Channel7_IRQHandler
	.word	LPUART1_IRQHandler
	.word	QUADSPI_IRQHandler
	.word	I2C3_EV_IRQHandler
	.word	I2C3_ER_IRQHandler
	.word	SAI1_IRQHandler
	.word	SAI2_IRQHandler
	.word	SWPMI1_IRQHandler
	.word	TSC_IRQHandler
	.word	LCD_IRQHandler
	.word 0
	.word	RNG_IRQHandler
	.word	FPU_IRQHandler
	.word	CRS_IRQHandler
	.word	I2C4_EV_IRQHandler
	.word	I2C4_ER_IRQHandler
	.word	DCMI_IRQHandler
	.word	CAN2_TX_IRQHandler
	.word	CAN2_RX0_IRQHandler
	.word	CAN2_RX1_IRQHandler
	.word	CAN2_SCE_IRQHandler
	.word	DMA2D_IRQHandler


/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the Default_Handler.
* As they are weak aliases, any function with the same name will override
* this definition.
*
*******************************************************************************/

  .weak	NMI_Handler
	.thumb_set NMI_Handler,Default_Handler

  .weak	HardFault_Handler
	.thumb_set HardFault_Handler,Default_Handler

  .weak	MemManage_Handler
	.thumb_set MemManage_Handler,Default_Handler

  .weak	BusFault_Handler
	.thumb_set BusFault_Handler,Default_Handler

	.weak	UsageFault_Handler
	.thumb_set UsageFault_Handler,Default_Handler

	.weak	SVC_Handler
	.thumb_set SVC_Handler,Default_Handler

	.weak	DebugMon_Handler
	.thumb_set DebugMon_Handler,Default_Handler

	.weak	PendSV_Handler
	.thumb_set PendSV_Handler,Default_Handler

	.weak	SysTick_Handler
	.thumb_set SysTick_Handler,SE_Handler

	.weak	WWDG_IRQHandler
	.thumb_set WWDG_IRQHandler,SE_Handler

	.weak	PVD_PVM_IRQHandler
	.thumb_set PVD_PVM_IRQHandler,SE_Handler

	.weak	TAMP_STAMP_IRQHandler
	.thumb_set TAMP_STAMP_IRQHandler,SE_Handler

	.weak	RTC_WKUP_IRQHandler
	.thumb_set RTC_WKUP_IRQHandler,SE_Handler

	.weak	FLASH_IRQHandler
	.thumb_set FLASH_IRQHandler,SE_Handler

	.weak	RCC_IRQHandler
	.thumb_set RCC_IRQHandler,SE_Handler

	.weak	EXTI0_IRQHandler
	.thumb_set EXTI0_IRQHandler,SE_Handler

	.weak	EXTI1_IRQHandler
	.thumb_set EXTI1_IRQHandler,SE_Handler

	.weak	EXTI2_IRQHandler
	.thumb_set EXTI2_IRQHandler,SE_Handler

	.weak	EXTI3_IRQHandler
	.thumb_set EXTI3_IRQHandler,SE_Handler

	.weak	EXTI4_IRQHandler
	.thumb_set EXTI4_IRQHandler,SE_Handler

	.weak	DMA1_Channel1_IRQHandler
	.thumb_set DMA1_Channel1_IRQHandler,SE_Handler

	.weak	DMA1_Channel2_IRQHandler
	.thumb_set DMA1_Channel2_IRQHandler,SE_Handler

	.weak	DMA1_Channel3_IRQHandler
	.thumb_set DMA1_Channel3_IRQHandler,SE_Handler

	.weak	DMA1_Channel4_IRQHandler
	.thumb_set DMA1_Channel4_IRQHandler,SE_Handler

	.weak	DMA1_Channel5_IRQHandler
	.thumb_set DMA1_Channel5_IRQHandler,SE_Handler

	.weak	DMA1_Channel6_IRQHandler
	.thumb_set DMA1_Channel6_IRQHandler,SE_Handler

	.weak	DMA1_Channel7_IRQHandler
	.thumb_set DMA1_Channel7_IRQHandler,SE_Handler

	.weak	ADC1_2_IRQHandler
	.thumb_set ADC1_2_IRQHandler,SE_Handler

	.weak	CAN1_TX_IRQHandler
	.thumb_set CAN1_TX_IRQHandler,SE_Handler

	.weak	CAN1_RX0_IRQHandler
	.thumb_set CAN1_RX0_IRQHandler,SE_Handler

	.weak	CAN1_RX1_IRQHandler
	.thumb_set CAN1_RX1_IRQHandler,SE_Handler

	.weak	CAN1_SCE_IRQHandler
	.thumb_set CAN1_SCE_IRQHandler,SE_Handler

	.weak	EXTI9_5_IRQHandler
	.thumb_set EXTI9_5_IRQHandler,SE_Handler

	.weak	TIM1_BRK_TIM15_IRQHandler
	.thumb_set TIM1_BRK_TIM15_IRQHandler,SE_Handler

	.weak	TIM1_UP_TIM16_IRQHandler
	.thumb_set TIM1_UP_TIM16_IRQHandler,SE_Handler

	.weak	TIM1_TRG_COM_TIM17_IRQHandler
	.thumb_set TIM1_TRG_COM_TIM17_IRQHandler,SE_Handler

	.weak	TIM1_CC_IRQHandler
	.thumb_set TIM1_CC_IRQHandler,SE_Handler

	.weak	TIM2_IRQHandler
	.thumb_set TIM2_IRQHandler,SE_Handler

	.weak	TIM3_IRQHandler
	.thumb_set TIM3_IRQHandler,SE_Handler

	.weak	TIM4_IRQHandler
	.thumb_set TIM4_IRQHandler,SE_Handler

	.weak	I2C1_EV_IRQHandler
	.thumb_set I2C1_EV_IRQHandler,SE_Handler

	.weak	I2C1_ER_IRQHandler
	.thumb_set I2C1_ER_IRQHandler,SE_Handler

	.weak	I2C2_EV_IRQHandler
	.thumb_set I2C2_EV_IRQHandler,SE_Handler

	.weak	I2C2_ER_IRQHandler
	.thumb_set I2C2_ER_IRQHandler,SE_Handler

	.weak	SPI1_IRQHandler
	.thumb_set SPI1_IRQHandler,SE_Handler

	.weak	SPI2_IRQHandler
	.thumb_set SPI2_IRQHandler,SE_Handler

	.weak	USART1_IRQHandler
	.thumb_set USART1_IRQHandler,SE_Handler

	.weak	USART2_IRQHandler
	.thumb_set USART2_IRQHandler,SE_Handler

	.weak	USART3_IRQHandler
	.thumb_set USART3_IRQHandler,SE_Handler

	.weak	EXTI15_10_IRQHandler
	.thumb_set EXTI15_10_IRQHandler,SE_Handler

	.weak	RTC_Alarm_IRQHandler
	.thumb_set RTC_Alarm_IRQHandler,SE_Handler

	.weak	DFSDM1_FLT3_IRQHandler
	.thumb_set DFSDM1_FLT3_IRQHandler,SE_Handler

	.weak	TIM8_BRK_IRQHandler
	.thumb_set TIM8_BRK_IRQHandler,SE_Handler

	.weak	TIM8_UP_IRQHandler
	.thumb_set TIM8_UP_IRQHandler,SE_Handler

	.weak	TIM8_TRG_COM_IRQHandler
	.thumb_set TIM8_TRG_COM_IRQHandler,SE_Handler

	.weak	TIM8_CC_IRQHandler
	.thumb_set TIM8_CC_IRQHandler,SE_Handler

	.weak	ADC3_IRQHandler
	.thumb_set ADC3_IRQHandler,SE_Handler

	.weak	FMC_IRQHandler
	.thumb_set FMC_IRQHandler,SE_Handler

	.weak	SDMMC1_IRQHandler
	.thumb_set SDMMC1_IRQHandler,SE_Handler

	.weak	TIM5_IRQHandler
	.thumb_set TIM5_IRQHandler,SE_Handler

	.weak	SPI3_IRQHandler
	.thumb_set SPI3_IRQHandler,SE_Handler

	.weak	UART4_IRQHandler
	.thumb_set UART4_IRQHandler,SE_Handler

	.weak	UART5_IRQHandler
	.thumb_set UART5_IRQHandler,SE_Handler

	.weak	TIM6_DAC_IRQHandler
	.thumb_set TIM6_DAC_IRQHandler,SE_Handler

	.weak	TIM7_IRQHandler
	.thumb_set TIM7_IRQHandler,SE_Handler

	.weak	DMA2_Channel1_IRQHandler
	.thumb_set DMA2_Channel1_IRQHandler,SE_Handler

	.weak	DMA2_Channel2_IRQHandler
	.thumb_set DMA2_Channel2_IRQHandler,SE_Handler

	.weak	DMA2_Channel3_IRQHandler
	.thumb_set DMA2_Channel3_IRQHandler,SE_Handler

	.weak	DMA2_Channel4_IRQHandler
	.thumb_set DMA2_Channel4_IRQHandler,SE_Handler

	.weak	DMA2_Channel5_IRQHandler
	.thumb_set DMA2_Channel5_IRQHandler,SE_Handler

	.weak	DFSDM1_FLT0_IRQHandler
	.thumb_set DFSDM1_FLT0_IRQHandler,SE_Handler

	.weak	DFSDM1_FLT1_IRQHandler
	.thumb_set DFSDM1_FLT1_IRQHandler,SE_Handler

	.weak	DFSDM1_FLT2_IRQHandler
	.thumb_set DFSDM1_FLT2_IRQHandler,SE_Handler

	.weak	COMP_IRQHandler
	.thumb_set COMP_IRQHandler,SE_Handler

	.weak	LPTIM1_IRQHandler
	.thumb_set LPTIM1_IRQHandler,SE_Handler

	.weak	LPTIM2_IRQHandler
	.thumb_set LPTIM2_IRQHandler,SE_Handler

	.weak	OTG_FS_IRQHandler
	.thumb_set OTG_FS_IRQHandler,SE_Handler

	.weak	DMA2_Channel6_IRQHandler
	.thumb_set DMA2_Channel6_IRQHandler,SE_Handler

	.weak	DMA2_Channel7_IRQHandler
	.thumb_set DMA2_Channel7_IRQHandler,SE_Handler

	.weak	LPUART1_IRQHandler
	.thumb_set LPUART1_IRQHandler,SE_Handler

	.weak	QUADSPI_IRQHandler
	.thumb_set QUADSPI_IRQHandler,SE_Handler

	.weak	I2C3_EV_IRQHandler
	.thumb_set I2C3_EV_IRQHandler,SE_Handler

	.weak	I2C3_ER_IRQHandler
	.thumb_set I2C3_ER_IRQHandler,SE_Handler

	.weak	SAI1_IRQHandler
	.thumb_set SAI1_IRQHandler,SE_Handler

	.weak	SAI2_IRQHandler
	.thumb_set SAI2_IRQHandler,SE_Handler

	.weak	SWPMI1_IRQHandler
	.thumb_set SWPMI1_IRQHandler,SE_Handler

	.weak	TSC_IRQHandler
	.thumb_set TSC_IRQHandler,SE_Handler

	.weak	LCD_IRQHandler
	.thumb_set LCD_IRQHandler,SE_Handler

	.weak	RNG_IRQHandler
	.thumb_set RNG_IRQHandler,SE_Handler

	.weak	FPU_IRQHandler
	.thumb_set FPU_IRQHandler,SE_Handler

	.weak	CRS_IRQHandler
	.thumb_set CRS_IRQHandler,SE_Handler

	.weak	I2C4_EV_IRQHandler
	.thumb_set I2C4_EV_IRQHandler,SE_Handler

	.weak	I2C4_ER_IRQHandler
	.thumb_set I2C4_ER_IRQHandler,SE_Handler

	.weak	DCMI_IRQHandler
	.thumb_set DCMI_IRQHandler,SE_Handler

	.weak	CAN2_TX_IRQHandler
	.thumb_set CAN2_TX_IRQHandler,SE_Handler

	.weak	CAN2_RX0_IRQHandler
	.thumb_set CAN2_RX0_IRQHandler,SE_Handler

	.weak	CAN2_RX1_IRQHandler
	.thumb_set CAN2_RX1_IRQHandler,SE_Handler

	.weak	CAN2_SCE_IRQHandler
	.thumb_set CAN2_SCE_IRQHandler,SE_Handler

	.weak	DMA2D_IRQHandler
	.thumb_set FPU_IRQHandler,SE_Handler

.end

