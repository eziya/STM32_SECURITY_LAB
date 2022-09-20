################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_crc.c \
E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_crc_ex.c \
E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_flash.c \
E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_flash_ex.c 

OBJS += \
./Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_crc.o \
./Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_crc_ex.o \
./Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_flash.o \
./Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_flash_ex.o 

C_DEPS += \
./Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_crc.d \
./Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_crc_ex.d \
./Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_flash.d \
./Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_flash_ex.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_crc.o: E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_crc.c Drivers/STM32G0xx_HAL_Driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DSTM32G071xx -DUSE_HAL_DRIVER -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32G0xx/Include -I../../../../../../../Drivers/STM32G0xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32G0xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Key -I../../../2_Images_SBSFU/SBSFU/App -I../../../2_Images_SBSFU/SBSFU/Target -I../../../Linker_Common/STM32CubeIDE -I../../../../../../../Middlewares/ST/STM32_Cryptographic/Fw_Crypto/STM32G0/Inc -I../../../../../../../Drivers/CMSIS/Include -Os -ffunction-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_crc_ex.o: E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_crc_ex.c Drivers/STM32G0xx_HAL_Driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DSTM32G071xx -DUSE_HAL_DRIVER -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32G0xx/Include -I../../../../../../../Drivers/STM32G0xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32G0xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Key -I../../../2_Images_SBSFU/SBSFU/App -I../../../2_Images_SBSFU/SBSFU/Target -I../../../Linker_Common/STM32CubeIDE -I../../../../../../../Middlewares/ST/STM32_Cryptographic/Fw_Crypto/STM32G0/Inc -I../../../../../../../Drivers/CMSIS/Include -Os -ffunction-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_flash.o: E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_flash.c Drivers/STM32G0xx_HAL_Driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DSTM32G071xx -DUSE_HAL_DRIVER -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32G0xx/Include -I../../../../../../../Drivers/STM32G0xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32G0xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Key -I../../../2_Images_SBSFU/SBSFU/App -I../../../2_Images_SBSFU/SBSFU/Target -I../../../Linker_Common/STM32CubeIDE -I../../../../../../../Middlewares/ST/STM32_Cryptographic/Fw_Crypto/STM32G0/Inc -I../../../../../../../Drivers/CMSIS/Include -Os -ffunction-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_flash_ex.o: E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/STM32G0xx_HAL_Driver/Src/stm32g0xx_hal_flash_ex.c Drivers/STM32G0xx_HAL_Driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DSTM32G071xx -DUSE_HAL_DRIVER -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32G0xx/Include -I../../../../../../../Drivers/STM32G0xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32G0xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Key -I../../../2_Images_SBSFU/SBSFU/App -I../../../2_Images_SBSFU/SBSFU/Target -I../../../Linker_Common/STM32CubeIDE -I../../../../../../../Middlewares/ST/STM32_Cryptographic/Fw_Crypto/STM32G0/Inc -I../../../../../../../Drivers/CMSIS/Include -Os -ffunction-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-STM32G0xx_HAL_Driver

clean-Drivers-2f-STM32G0xx_HAL_Driver:
	-$(RM) ./Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_crc.d ./Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_crc.o ./Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_crc.su ./Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_crc_ex.d ./Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_crc_ex.o ./Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_crc_ex.su ./Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_flash.d ./Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_flash.o ./Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_flash.su ./Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_flash_ex.d ./Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_flash_ex.o ./Drivers/STM32G0xx_HAL_Driver/stm32g0xx_hal_flash_ex.su

.PHONY: clean-Drivers-2f-STM32G0xx_HAL_Driver

