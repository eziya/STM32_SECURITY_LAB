################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32F4_SBSFU/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_crc.c \
E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32F4_SBSFU/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c \
E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32F4_SBSFU/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c 

OBJS += \
./Drivers/STM32F4xx_HAL_Driver/stm32f4xx_hal_crc.o \
./Drivers/STM32F4xx_HAL_Driver/stm32f4xx_hal_flash.o \
./Drivers/STM32F4xx_HAL_Driver/stm32f4xx_hal_flash_ex.o 

C_DEPS += \
./Drivers/STM32F4xx_HAL_Driver/stm32f4xx_hal_crc.d \
./Drivers/STM32F4xx_HAL_Driver/stm32f4xx_hal_flash.d \
./Drivers/STM32F4xx_HAL_Driver/stm32f4xx_hal_flash_ex.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/STM32F4xx_HAL_Driver/stm32f4xx_hal_crc.o: E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32F4_SBSFU/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_crc.c Drivers/STM32F4xx_HAL_Driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32F429xx -DUSE_HAL_DRIVER -DUSE_NUCLEO_F429ZI -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../../../../../../Drivers/CMSIS/Include -I../../../../../../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/NUCLEO-F429ZI -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Utilities/Fonts -I../../../../../../../Middlewares/ST/STM32_Cryptographic/Fw_Crypto/STM32F4/Inc -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Key -I../../../2_Images_SBSFU/SBSFU/App -I../../../2_Images_SBSFU/SBSFU/Target -I../../../Linker_Common/STM32CubeIDE -O0 -ffunction-sections -Wall -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -o "$@"
Drivers/STM32F4xx_HAL_Driver/stm32f4xx_hal_flash.o: E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32F4_SBSFU/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c Drivers/STM32F4xx_HAL_Driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32F429xx -DUSE_HAL_DRIVER -DUSE_NUCLEO_F429ZI -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../../../../../../Drivers/CMSIS/Include -I../../../../../../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/NUCLEO-F429ZI -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Utilities/Fonts -I../../../../../../../Middlewares/ST/STM32_Cryptographic/Fw_Crypto/STM32F4/Inc -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Key -I../../../2_Images_SBSFU/SBSFU/App -I../../../2_Images_SBSFU/SBSFU/Target -I../../../Linker_Common/STM32CubeIDE -O0 -ffunction-sections -Wall -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -o "$@"
Drivers/STM32F4xx_HAL_Driver/stm32f4xx_hal_flash_ex.o: E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32F4_SBSFU/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c Drivers/STM32F4xx_HAL_Driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32F429xx -DUSE_HAL_DRIVER -DUSE_NUCLEO_F429ZI -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../../../../../../Drivers/CMSIS/Include -I../../../../../../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/NUCLEO-F429ZI -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Utilities/Fonts -I../../../../../../../Middlewares/ST/STM32_Cryptographic/Fw_Crypto/STM32F4/Inc -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Key -I../../../2_Images_SBSFU/SBSFU/App -I../../../2_Images_SBSFU/SBSFU/Target -I../../../Linker_Common/STM32CubeIDE -O0 -ffunction-sections -Wall -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -o "$@"

clean: clean-Drivers-2f-STM32F4xx_HAL_Driver

clean-Drivers-2f-STM32F4xx_HAL_Driver:
	-$(RM) ./Drivers/STM32F4xx_HAL_Driver/stm32f4xx_hal_crc.d ./Drivers/STM32F4xx_HAL_Driver/stm32f4xx_hal_crc.o ./Drivers/STM32F4xx_HAL_Driver/stm32f4xx_hal_crc.su ./Drivers/STM32F4xx_HAL_Driver/stm32f4xx_hal_flash.d ./Drivers/STM32F4xx_HAL_Driver/stm32f4xx_hal_flash.o ./Drivers/STM32F4xx_HAL_Driver/stm32f4xx_hal_flash.su ./Drivers/STM32F4xx_HAL_Driver/stm32f4xx_hal_flash_ex.d ./Drivers/STM32F4xx_HAL_Driver/stm32f4xx_hal_flash_ex.o ./Drivers/STM32F4xx_HAL_Driver/stm32f4xx_hal_flash_ex.su

.PHONY: clean-Drivers-2f-STM32F4xx_HAL_Driver

