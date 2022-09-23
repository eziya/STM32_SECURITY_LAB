################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_crc.c \
E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_crc_ex.c \
E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_firewall.c \
E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash.c \
E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.c \
E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c \
E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.c 

OBJS += \
./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_crc.o \
./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_crc_ex.o \
./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_firewall.o \
./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_flash.o \
./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_flash_ex.o \
./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_rcc.o \
./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_rcc_ex.o 

C_DEPS += \
./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_crc.d \
./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_crc_ex.d \
./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_firewall.d \
./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_flash.d \
./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_flash_ex.d \
./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_rcc.d \
./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_rcc_ex.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_crc.o: E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_crc.c Drivers/STM32L4xx_HAL_Driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L476xx -DUSE_HAL_DRIVER -DUSE_STM32L4XX_NUCLEO -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32L4xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Middlewares/ST/STM32_Cryptographic/Fw_Crypto/STM32L4/Inc -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Key -I../../../2_Images_SBSFU/SBSFU/App -I../../../2_Images_SBSFU/SBSFU/Target -I../../../Linker_Common/STM32CubeIDE -I../../../../../../../Drivers/CMSIS/Include -Og -ffunction-sections -Wall -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -o "$@"
Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_crc_ex.o: E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_crc_ex.c Drivers/STM32L4xx_HAL_Driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L476xx -DUSE_HAL_DRIVER -DUSE_STM32L4XX_NUCLEO -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32L4xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Middlewares/ST/STM32_Cryptographic/Fw_Crypto/STM32L4/Inc -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Key -I../../../2_Images_SBSFU/SBSFU/App -I../../../2_Images_SBSFU/SBSFU/Target -I../../../Linker_Common/STM32CubeIDE -I../../../../../../../Drivers/CMSIS/Include -Og -ffunction-sections -Wall -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -o "$@"
Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_firewall.o: E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_firewall.c Drivers/STM32L4xx_HAL_Driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L476xx -DUSE_HAL_DRIVER -DUSE_STM32L4XX_NUCLEO -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32L4xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Middlewares/ST/STM32_Cryptographic/Fw_Crypto/STM32L4/Inc -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Key -I../../../2_Images_SBSFU/SBSFU/App -I../../../2_Images_SBSFU/SBSFU/Target -I../../../Linker_Common/STM32CubeIDE -I../../../../../../../Drivers/CMSIS/Include -Og -ffunction-sections -Wall -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -o "$@"
Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_flash.o: E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash.c Drivers/STM32L4xx_HAL_Driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L476xx -DUSE_HAL_DRIVER -DUSE_STM32L4XX_NUCLEO -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32L4xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Middlewares/ST/STM32_Cryptographic/Fw_Crypto/STM32L4/Inc -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Key -I../../../2_Images_SBSFU/SBSFU/App -I../../../2_Images_SBSFU/SBSFU/Target -I../../../Linker_Common/STM32CubeIDE -I../../../../../../../Drivers/CMSIS/Include -Og -ffunction-sections -Wall -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -o "$@"
Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_flash_ex.o: E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.c Drivers/STM32L4xx_HAL_Driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L476xx -DUSE_HAL_DRIVER -DUSE_STM32L4XX_NUCLEO -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32L4xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Middlewares/ST/STM32_Cryptographic/Fw_Crypto/STM32L4/Inc -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Key -I../../../2_Images_SBSFU/SBSFU/App -I../../../2_Images_SBSFU/SBSFU/Target -I../../../Linker_Common/STM32CubeIDE -I../../../../../../../Drivers/CMSIS/Include -Og -ffunction-sections -Wall -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -o "$@"
Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_rcc.o: E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c Drivers/STM32L4xx_HAL_Driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L476xx -DUSE_HAL_DRIVER -DUSE_STM32L4XX_NUCLEO -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32L4xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Middlewares/ST/STM32_Cryptographic/Fw_Crypto/STM32L4/Inc -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Key -I../../../2_Images_SBSFU/SBSFU/App -I../../../2_Images_SBSFU/SBSFU/Target -I../../../Linker_Common/STM32CubeIDE -I../../../../../../../Drivers/CMSIS/Include -Og -ffunction-sections -Wall -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -o "$@"
Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_rcc_ex.o: E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.c Drivers/STM32L4xx_HAL_Driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L476xx -DUSE_HAL_DRIVER -DUSE_STM32L4XX_NUCLEO -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32L4xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Middlewares/ST/STM32_Cryptographic/Fw_Crypto/STM32L4/Inc -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Key -I../../../2_Images_SBSFU/SBSFU/App -I../../../2_Images_SBSFU/SBSFU/Target -I../../../Linker_Common/STM32CubeIDE -I../../../../../../../Drivers/CMSIS/Include -Og -ffunction-sections -Wall -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -o "$@"

clean: clean-Drivers-2f-STM32L4xx_HAL_Driver

clean-Drivers-2f-STM32L4xx_HAL_Driver:
	-$(RM) ./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_crc.d ./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_crc.o ./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_crc.su ./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_crc_ex.d ./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_crc_ex.o ./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_crc_ex.su ./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_firewall.d ./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_firewall.o ./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_firewall.su ./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_flash.d ./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_flash.o ./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_flash.su ./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_flash_ex.d ./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_flash_ex.o ./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_flash_ex.su ./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_rcc.d ./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_rcc.o ./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_rcc.su ./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_rcc_ex.d ./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_rcc_ex.o ./Drivers/STM32L4xx_HAL_Driver/stm32l4xx_hal_rcc_ex.su

.PHONY: clean-Drivers-2f-STM32L4xx_HAL_Driver

