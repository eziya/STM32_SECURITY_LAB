################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32F4_SBSFU/Middlewares/ST/STM32_Secure_Engine/Core/se_interface_application.c \
E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32F4_SBSFU/Middlewares/ST/STM32_Secure_Engine/Core/se_interface_bootloader.c \
E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32F4_SBSFU/Middlewares/ST/STM32_Secure_Engine/Core/se_interface_common.c 

OBJS += \
./Middlewares/STM32_Secure_Engine/se_interface_application.o \
./Middlewares/STM32_Secure_Engine/se_interface_bootloader.o \
./Middlewares/STM32_Secure_Engine/se_interface_common.o 

C_DEPS += \
./Middlewares/STM32_Secure_Engine/se_interface_application.d \
./Middlewares/STM32_Secure_Engine/se_interface_bootloader.d \
./Middlewares/STM32_Secure_Engine/se_interface_common.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/STM32_Secure_Engine/se_interface_application.o: E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32F4_SBSFU/Middlewares/ST/STM32_Secure_Engine/Core/se_interface_application.c Middlewares/STM32_Secure_Engine/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32F429xx -DUSE_HAL_DRIVER -DUSE_NUCLEO_F429ZI -c -I../../../../../../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../../../../../../Drivers/CMSIS/Include -I../../../../../../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/NUCLEO-F429ZI -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Utilities/Fonts -I../../Core/Inc -I../../SBSFU/App -I../../SBSFU/Target -I../../../2_Images_SECoreBin/Inc -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Interface -I../../../Linker_Common/STM32CubeIDE -O0 -ffunction-sections -Wall -Wno-format -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Middlewares/STM32_Secure_Engine/se_interface_bootloader.o: E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32F4_SBSFU/Middlewares/ST/STM32_Secure_Engine/Core/se_interface_bootloader.c Middlewares/STM32_Secure_Engine/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32F429xx -DUSE_HAL_DRIVER -DUSE_NUCLEO_F429ZI -c -I../../../../../../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../../../../../../Drivers/CMSIS/Include -I../../../../../../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/NUCLEO-F429ZI -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Utilities/Fonts -I../../Core/Inc -I../../SBSFU/App -I../../SBSFU/Target -I../../../2_Images_SECoreBin/Inc -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Interface -I../../../Linker_Common/STM32CubeIDE -O0 -ffunction-sections -Wall -Wno-format -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Middlewares/STM32_Secure_Engine/se_interface_common.o: E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32F4_SBSFU/Middlewares/ST/STM32_Secure_Engine/Core/se_interface_common.c Middlewares/STM32_Secure_Engine/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32F429xx -DUSE_HAL_DRIVER -DUSE_NUCLEO_F429ZI -c -I../../../../../../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../../../../../../Drivers/CMSIS/Include -I../../../../../../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/NUCLEO-F429ZI -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Utilities/Fonts -I../../Core/Inc -I../../SBSFU/App -I../../SBSFU/Target -I../../../2_Images_SECoreBin/Inc -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Interface -I../../../Linker_Common/STM32CubeIDE -O0 -ffunction-sections -Wall -Wno-format -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Middlewares-2f-STM32_Secure_Engine

clean-Middlewares-2f-STM32_Secure_Engine:
	-$(RM) ./Middlewares/STM32_Secure_Engine/se_interface_application.d ./Middlewares/STM32_Secure_Engine/se_interface_application.o ./Middlewares/STM32_Secure_Engine/se_interface_application.su ./Middlewares/STM32_Secure_Engine/se_interface_bootloader.d ./Middlewares/STM32_Secure_Engine/se_interface_bootloader.o ./Middlewares/STM32_Secure_Engine/se_interface_bootloader.su ./Middlewares/STM32_Secure_Engine/se_interface_common.d ./Middlewares/STM32_Secure_Engine/se_interface_common.o ./Middlewares/STM32_Secure_Engine/se_interface_common.su

.PHONY: clean-Middlewares-2f-STM32_Secure_Engine

