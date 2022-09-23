################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_SBSFU/SBSFU/Target/sfu_low_level.c \
E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_SBSFU/SBSFU/Target/sfu_low_level_flash.c \
E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_SBSFU/SBSFU/Target/sfu_low_level_flash_ext.c \
E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_SBSFU/SBSFU/Target/sfu_low_level_flash_int.c \
E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_SBSFU/SBSFU/Target/sfu_low_level_security.c 

OBJS += \
./Application/SBSFU/Target/sfu_low_level.o \
./Application/SBSFU/Target/sfu_low_level_flash.o \
./Application/SBSFU/Target/sfu_low_level_flash_ext.o \
./Application/SBSFU/Target/sfu_low_level_flash_int.o \
./Application/SBSFU/Target/sfu_low_level_security.o 

C_DEPS += \
./Application/SBSFU/Target/sfu_low_level.d \
./Application/SBSFU/Target/sfu_low_level_flash.d \
./Application/SBSFU/Target/sfu_low_level_flash_ext.d \
./Application/SBSFU/Target/sfu_low_level_flash_int.d \
./Application/SBSFU/Target/sfu_low_level_security.d 


# Each subdirectory must supply rules for building sources it contributes
Application/SBSFU/Target/sfu_low_level.o: E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_SBSFU/SBSFU/Target/sfu_low_level.c Application/SBSFU/Target/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L476xx -DUSE_HAL_DRIVER -DUSE_STM32L4XX_NUCLEO -c -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32L4xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../Core/Inc -I../../SBSFU/App -I../../SBSFU/Target -I../../../2_Images_SECoreBin/Inc -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Interface -I../../../../../../../Drivers/CMSIS/Include -I../../../Linker_Common/STM32CubeIDE -Og -ffunction-sections -Wall -Wno-format -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/SBSFU/Target/sfu_low_level_flash.o: E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_SBSFU/SBSFU/Target/sfu_low_level_flash.c Application/SBSFU/Target/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L476xx -DUSE_HAL_DRIVER -DUSE_STM32L4XX_NUCLEO -c -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32L4xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../Core/Inc -I../../SBSFU/App -I../../SBSFU/Target -I../../../2_Images_SECoreBin/Inc -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Interface -I../../../../../../../Drivers/CMSIS/Include -I../../../Linker_Common/STM32CubeIDE -Og -ffunction-sections -Wall -Wno-format -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/SBSFU/Target/sfu_low_level_flash_ext.o: E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_SBSFU/SBSFU/Target/sfu_low_level_flash_ext.c Application/SBSFU/Target/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L476xx -DUSE_HAL_DRIVER -DUSE_STM32L4XX_NUCLEO -c -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32L4xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../Core/Inc -I../../SBSFU/App -I../../SBSFU/Target -I../../../2_Images_SECoreBin/Inc -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Interface -I../../../../../../../Drivers/CMSIS/Include -I../../../Linker_Common/STM32CubeIDE -Og -ffunction-sections -Wall -Wno-format -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/SBSFU/Target/sfu_low_level_flash_int.o: E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_SBSFU/SBSFU/Target/sfu_low_level_flash_int.c Application/SBSFU/Target/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L476xx -DUSE_HAL_DRIVER -DUSE_STM32L4XX_NUCLEO -c -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32L4xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../Core/Inc -I../../SBSFU/App -I../../SBSFU/Target -I../../../2_Images_SECoreBin/Inc -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Interface -I../../../../../../../Drivers/CMSIS/Include -I../../../Linker_Common/STM32CubeIDE -Og -ffunction-sections -Wall -Wno-format -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/SBSFU/Target/sfu_low_level_security.o: E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_SBSFU/SBSFU/Target/sfu_low_level_security.c Application/SBSFU/Target/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L476xx -DUSE_HAL_DRIVER -DUSE_STM32L4XX_NUCLEO -c -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32L4xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../Core/Inc -I../../SBSFU/App -I../../SBSFU/Target -I../../../2_Images_SECoreBin/Inc -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Interface -I../../../../../../../Drivers/CMSIS/Include -I../../../Linker_Common/STM32CubeIDE -Og -ffunction-sections -Wall -Wno-format -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Application-2f-SBSFU-2f-Target

clean-Application-2f-SBSFU-2f-Target:
	-$(RM) ./Application/SBSFU/Target/sfu_low_level.d ./Application/SBSFU/Target/sfu_low_level.o ./Application/SBSFU/Target/sfu_low_level.su ./Application/SBSFU/Target/sfu_low_level_flash.d ./Application/SBSFU/Target/sfu_low_level_flash.o ./Application/SBSFU/Target/sfu_low_level_flash.su ./Application/SBSFU/Target/sfu_low_level_flash_ext.d ./Application/SBSFU/Target/sfu_low_level_flash_ext.o ./Application/SBSFU/Target/sfu_low_level_flash_ext.su ./Application/SBSFU/Target/sfu_low_level_flash_int.d ./Application/SBSFU/Target/sfu_low_level_flash_int.o ./Application/SBSFU/Target/sfu_low_level_flash_int.su ./Application/SBSFU/Target/sfu_low_level_security.d ./Application/SBSFU/Target/sfu_low_level_security.o ./Application/SBSFU/Target/sfu_low_level_security.su

.PHONY: clean-Application-2f-SBSFU-2f-Target

