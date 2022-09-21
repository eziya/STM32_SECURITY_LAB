################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_UserApp/Src/com.c \
E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_UserApp/Src/common.c \
E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_UserApp/Src/flash_if.c \
E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_UserApp/Src/fw_update_app.c \
E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_UserApp/Src/main.c \
E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_UserApp/Src/se_user_code.c \
E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_UserApp/Src/stm32l4xx_it.c \
../Application/User/syscalls.c \
E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_UserApp/Src/test_protections.c \
E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_UserApp/Src/ymodem.c 

OBJS += \
./Application/User/com.o \
./Application/User/common.o \
./Application/User/flash_if.o \
./Application/User/fw_update_app.o \
./Application/User/main.o \
./Application/User/se_user_code.o \
./Application/User/stm32l4xx_it.o \
./Application/User/syscalls.o \
./Application/User/test_protections.o \
./Application/User/ymodem.o 

C_DEPS += \
./Application/User/com.d \
./Application/User/common.d \
./Application/User/flash_if.d \
./Application/User/fw_update_app.d \
./Application/User/main.d \
./Application/User/se_user_code.d \
./Application/User/stm32l4xx_it.d \
./Application/User/syscalls.d \
./Application/User/test_protections.d \
./Application/User/ymodem.d 


# Each subdirectory must supply rules for building sources it contributes
Application/User/com.o: E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_UserApp/Src/com.c Application/User/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L476xx -DUSE_HAL_DRIVER -DUSE_STM32L4XX_NUCLEO -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32L4xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../2_Images_SECoreBin/Inc -I../../../2_Images_SBSFU/SBSFU/App -I../../../../../../../Drivers/CMSIS/Include -I../../../Linker_Common/STM32CubeIDE -Og -ffunction-sections -Wall -Wno-format -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/common.o: E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_UserApp/Src/common.c Application/User/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L476xx -DUSE_HAL_DRIVER -DUSE_STM32L4XX_NUCLEO -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32L4xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../2_Images_SECoreBin/Inc -I../../../2_Images_SBSFU/SBSFU/App -I../../../../../../../Drivers/CMSIS/Include -I../../../Linker_Common/STM32CubeIDE -Og -ffunction-sections -Wall -Wno-format -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/flash_if.o: E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_UserApp/Src/flash_if.c Application/User/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L476xx -DUSE_HAL_DRIVER -DUSE_STM32L4XX_NUCLEO -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32L4xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../2_Images_SECoreBin/Inc -I../../../2_Images_SBSFU/SBSFU/App -I../../../../../../../Drivers/CMSIS/Include -I../../../Linker_Common/STM32CubeIDE -Og -ffunction-sections -Wall -Wno-format -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/fw_update_app.o: E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_UserApp/Src/fw_update_app.c Application/User/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L476xx -DUSE_HAL_DRIVER -DUSE_STM32L4XX_NUCLEO -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32L4xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../2_Images_SECoreBin/Inc -I../../../2_Images_SBSFU/SBSFU/App -I../../../../../../../Drivers/CMSIS/Include -I../../../Linker_Common/STM32CubeIDE -Og -ffunction-sections -Wall -Wno-format -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/main.o: E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_UserApp/Src/main.c Application/User/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L476xx -DUSE_HAL_DRIVER -DUSE_STM32L4XX_NUCLEO -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32L4xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../2_Images_SECoreBin/Inc -I../../../2_Images_SBSFU/SBSFU/App -I../../../../../../../Drivers/CMSIS/Include -I../../../Linker_Common/STM32CubeIDE -Og -ffunction-sections -Wall -Wno-format -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/se_user_code.o: E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_UserApp/Src/se_user_code.c Application/User/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L476xx -DUSE_HAL_DRIVER -DUSE_STM32L4XX_NUCLEO -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32L4xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../2_Images_SECoreBin/Inc -I../../../2_Images_SBSFU/SBSFU/App -I../../../../../../../Drivers/CMSIS/Include -I../../../Linker_Common/STM32CubeIDE -Og -ffunction-sections -Wall -Wno-format -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/stm32l4xx_it.o: E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_UserApp/Src/stm32l4xx_it.c Application/User/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L476xx -DUSE_HAL_DRIVER -DUSE_STM32L4XX_NUCLEO -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32L4xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../2_Images_SECoreBin/Inc -I../../../2_Images_SBSFU/SBSFU/App -I../../../../../../../Drivers/CMSIS/Include -I../../../Linker_Common/STM32CubeIDE -Og -ffunction-sections -Wall -Wno-format -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/%.o Application/User/%.su: ../Application/User/%.c Application/User/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L476xx -DUSE_HAL_DRIVER -DUSE_STM32L4XX_NUCLEO -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32L4xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../2_Images_SECoreBin/Inc -I../../../2_Images_SBSFU/SBSFU/App -I../../../../../../../Drivers/CMSIS/Include -I../../../Linker_Common/STM32CubeIDE -Og -ffunction-sections -Wall -Wno-format -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/test_protections.o: E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_UserApp/Src/test_protections.c Application/User/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L476xx -DUSE_HAL_DRIVER -DUSE_STM32L4XX_NUCLEO -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32L4xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../2_Images_SECoreBin/Inc -I../../../2_Images_SBSFU/SBSFU/App -I../../../../../../../Drivers/CMSIS/Include -I../../../Linker_Common/STM32CubeIDE -Og -ffunction-sections -Wall -Wno-format -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/ymodem.o: E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_UserApp/Src/ymodem.c Application/User/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L476xx -DUSE_HAL_DRIVER -DUSE_STM32L4XX_NUCLEO -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32L4xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../2_Images_SECoreBin/Inc -I../../../2_Images_SBSFU/SBSFU/App -I../../../../../../../Drivers/CMSIS/Include -I../../../Linker_Common/STM32CubeIDE -Og -ffunction-sections -Wall -Wno-format -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Application-2f-User

clean-Application-2f-User:
	-$(RM) ./Application/User/com.d ./Application/User/com.o ./Application/User/com.su ./Application/User/common.d ./Application/User/common.o ./Application/User/common.su ./Application/User/flash_if.d ./Application/User/flash_if.o ./Application/User/flash_if.su ./Application/User/fw_update_app.d ./Application/User/fw_update_app.o ./Application/User/fw_update_app.su ./Application/User/main.d ./Application/User/main.o ./Application/User/main.su ./Application/User/se_user_code.d ./Application/User/se_user_code.o ./Application/User/se_user_code.su ./Application/User/stm32l4xx_it.d ./Application/User/stm32l4xx_it.o ./Application/User/stm32l4xx_it.su ./Application/User/syscalls.d ./Application/User/syscalls.o ./Application/User/syscalls.su ./Application/User/test_protections.d ./Application/User/test_protections.o ./Application/User/test_protections.su ./Application/User/ymodem.d ./Application/User/ymodem.o ./Application/User/ymodem.su

.PHONY: clean-Application-2f-User

