################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32F4_SBSFU/Drivers/BSP/NUCLEO-F429ZI/nucleo_f429zi.c 

OBJS += \
./Drivers/BSP/NUCLEO-F429ZI/nucleo_f429zi.o 

C_DEPS += \
./Drivers/BSP/NUCLEO-F429ZI/nucleo_f429zi.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/NUCLEO-F429ZI/nucleo_f429zi.o: E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32F4_SBSFU/Drivers/BSP/NUCLEO-F429ZI/nucleo_f429zi.c Drivers/BSP/NUCLEO-F429ZI/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32F429xx -DUSE_HAL_DRIVER -DUSE_NUCLEO_F429ZI -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../../../../../../Drivers/CMSIS/Include -I../../../../../../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/NUCLEO-F429ZI -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Utilities/Fonts -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../2_Images_SECoreBin/Inc -I../../../2_Images_SBSFU/SBSFU/App -I../../../Linker_Common/STM32CubeIDE -O0 -ffunction-sections -Wall -Wno-format -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-NUCLEO-2d-F429ZI

clean-Drivers-2f-BSP-2f-NUCLEO-2d-F429ZI:
	-$(RM) ./Drivers/BSP/NUCLEO-F429ZI/nucleo_f429zi.d ./Drivers/BSP/NUCLEO-F429ZI/nucleo_f429zi.o ./Drivers/BSP/NUCLEO-F429ZI/nucleo_f429zi.su

.PHONY: clean-Drivers-2f-BSP-2f-NUCLEO-2d-F429ZI

