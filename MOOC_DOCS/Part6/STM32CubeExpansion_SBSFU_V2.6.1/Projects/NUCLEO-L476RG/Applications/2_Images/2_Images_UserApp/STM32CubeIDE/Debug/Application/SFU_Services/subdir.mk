################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_UserApp/Src/sfu_app_new_image.c 

OBJS += \
./Application/SFU_Services/sfu_app_new_image.o 

C_DEPS += \
./Application/SFU_Services/sfu_app_new_image.d 


# Each subdirectory must supply rules for building sources it contributes
Application/SFU_Services/sfu_app_new_image.o: E:/Workspace/STM32_SECURITY_LAB/MOOC_DOCS/Part6/STM32CubeExpansion_SBSFU_V2.6.1/Projects/NUCLEO-L476RG/Applications/2_Images/2_Images_UserApp/Src/sfu_app_new_image.c Application/SFU_Services/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DSTM32L476xx -DUSE_HAL_DRIVER -DUSE_STM32L4XX_NUCLEO -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../../../../../../../Drivers/STM32L4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32L4xx_Nucleo -I../../../../../../../Drivers/BSP/Components/Common -I../../../../../../../Middlewares/ST/STM32_Secure_Engine/Core -I../../../2_Images_SECoreBin/Inc -I../../../2_Images_SBSFU/SBSFU/App -I../../../../../../../Drivers/CMSIS/Include -I../../../Linker_Common/STM32CubeIDE -Og -ffunction-sections -Wall -Wno-format -Wno-strict-aliasing -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Application-2f-SFU_Services

clean-Application-2f-SFU_Services:
	-$(RM) ./Application/SFU_Services/sfu_app_new_image.d ./Application/SFU_Services/sfu_app_new_image.o ./Application/SFU_Services/sfu_app_new_image.su

.PHONY: clean-Application-2f-SFU_Services

