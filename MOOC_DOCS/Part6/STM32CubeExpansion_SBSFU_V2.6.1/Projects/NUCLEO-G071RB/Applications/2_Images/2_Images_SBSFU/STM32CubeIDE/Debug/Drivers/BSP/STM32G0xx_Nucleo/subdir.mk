################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/BSP/STM32G0xx_Nucleo/stm32g0xx_nucleo.c 

OBJS += \
./Drivers/BSP/STM32G0xx_Nucleo/stm32g0xx_nucleo.o 

C_DEPS += \
./Drivers/BSP/STM32G0xx_Nucleo/stm32g0xx_nucleo.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/STM32G0xx_Nucleo/stm32g0xx_nucleo.o: E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/BSP/STM32G0xx_Nucleo/stm32g0xx_nucleo.c Drivers/BSP/STM32G0xx_Nucleo/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DSTM32G071xx -DUSE_STM32G0XX_NUCLEO -DUSE_HAL_DRIVER -c -I../../../../../../../Drivers/CMSIS/Device/ST/STM32G0xx/Include -I../../../../../../../Drivers/STM32G0xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32G0xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../Core/Inc -I../../SBSFU/App -I../../SBSFU/Target -I../../../2_Images_SECoreBin/Inc -I../../../Linker_Common/STM32CubeIDE -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Interface -I../../../../../../../Drivers/CMSIS/Include -Os -ffunction-sections -Wall -Wno-format -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-STM32G0xx_Nucleo

clean-Drivers-2f-BSP-2f-STM32G0xx_Nucleo:
	-$(RM) ./Drivers/BSP/STM32G0xx_Nucleo/stm32g0xx_nucleo.d ./Drivers/BSP/STM32G0xx_Nucleo/stm32g0xx_nucleo.o ./Drivers/BSP/STM32G0xx_Nucleo/stm32g0xx_nucleo.su

.PHONY: clean-Drivers-2f-BSP-2f-STM32G0xx_Nucleo

