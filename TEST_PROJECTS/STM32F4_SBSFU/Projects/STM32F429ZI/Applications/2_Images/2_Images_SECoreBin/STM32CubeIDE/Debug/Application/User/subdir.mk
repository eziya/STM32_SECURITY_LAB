################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Application/User/data_init.c \
E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32F4_SBSFU/Projects/STM32F429ZI/Applications/2_Images/2_Images_SECoreBin/Src/se_crypto_bootloader.c \
E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32F4_SBSFU/Projects/STM32F429ZI/Applications/2_Images/2_Images_SECoreBin/Src/se_low_level.c 

OBJS += \
./Application/User/data_init.o \
./Application/User/se_crypto_bootloader.o \
./Application/User/se_low_level.o 

C_DEPS += \
./Application/User/data_init.d \
./Application/User/se_crypto_bootloader.d \
./Application/User/se_low_level.d 


# Each subdirectory must supply rules for building sources it contributes
Application/User/%.o Application/User/%.su: ../Application/User/%.c Application/User/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32F429xx -DUSE_HAL_DRIVER -DUSE_NUCLEO_F429ZI -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../../../../../../Drivers/CMSIS/Include -I../../../../../../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/NUCLEO-F429ZI -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Utilities/Fonts -I../../../../../../../Middlewares/ST/STM32_Cryptographic/Fw_Crypto/STM32F4/Inc -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Key -I../../../2_Images_SBSFU/SBSFU/App -I../../../2_Images_SBSFU/SBSFU/Target -I../../../Linker_Common/STM32CubeIDE -O0 -ffunction-sections -Wall -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -o "$@"
Application/User/se_crypto_bootloader.o: E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32F4_SBSFU/Projects/STM32F429ZI/Applications/2_Images/2_Images_SECoreBin/Src/se_crypto_bootloader.c Application/User/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32F429xx -DUSE_HAL_DRIVER -DUSE_NUCLEO_F429ZI -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../../../../../../Drivers/CMSIS/Include -I../../../../../../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/NUCLEO-F429ZI -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Utilities/Fonts -I../../../../../../../Middlewares/ST/STM32_Cryptographic/Fw_Crypto/STM32F4/Inc -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Key -I../../../2_Images_SBSFU/SBSFU/App -I../../../2_Images_SBSFU/SBSFU/Target -I../../../Linker_Common/STM32CubeIDE -O0 -ffunction-sections -Wall -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -o "$@"
Application/User/se_low_level.o: E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32F4_SBSFU/Projects/STM32F429ZI/Applications/2_Images/2_Images_SECoreBin/Src/se_low_level.c Application/User/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32F429xx -DUSE_HAL_DRIVER -DUSE_NUCLEO_F429ZI -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../../../../../../Drivers/CMSIS/Include -I../../../../../../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/NUCLEO-F429ZI -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Utilities/Fonts -I../../../../../../../Middlewares/ST/STM32_Cryptographic/Fw_Crypto/STM32F4/Inc -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Key -I../../../2_Images_SBSFU/SBSFU/App -I../../../2_Images_SBSFU/SBSFU/Target -I../../../Linker_Common/STM32CubeIDE -O0 -ffunction-sections -Wall -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -o "$@"

clean: clean-Application-2f-User

clean-Application-2f-User:
	-$(RM) ./Application/User/data_init.d ./Application/User/data_init.o ./Application/User/data_init.su ./Application/User/se_crypto_bootloader.d ./Application/User/se_crypto_bootloader.o ./Application/User/se_crypto_bootloader.su ./Application/User/se_low_level.d ./Application/User/se_low_level.o ./Application/User/se_low_level.su

.PHONY: clean-Application-2f-User

