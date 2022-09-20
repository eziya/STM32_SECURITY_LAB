################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-G071RB/Applications/2_Images/2_Images_UserApp/Src/system_stm32g0xx.c 

OBJS += \
./Drivers/CMSIS/system_stm32g0xx.o 

C_DEPS += \
./Drivers/CMSIS/system_stm32g0xx.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/CMSIS/system_stm32g0xx.o: E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-G071RB/Applications/2_Images/2_Images_UserApp/Src/system_stm32g0xx.c Drivers/CMSIS/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DSTM32G071xx -DUSE_STM32G0XX_NUCLEO -DUSE_HAL_DRIVER -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32G0xx/Include -I../../../../../../../Drivers/STM32G0xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32G0xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../2_Images_SECoreBin/Inc -I../../../2_Images_SBSFU/SBSFU/App -I../../../../../../../Drivers/CMSIS/Include -I../../../Linker_Common/STM32CubeIDE -Os -ffunction-sections -Wall -Wno-format -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-CMSIS

clean-Drivers-2f-CMSIS:
	-$(RM) ./Drivers/CMSIS/system_stm32g0xx.d ./Drivers/CMSIS/system_stm32g0xx.o ./Drivers/CMSIS/system_stm32g0xx.su

.PHONY: clean-Drivers-2f-CMSIS

