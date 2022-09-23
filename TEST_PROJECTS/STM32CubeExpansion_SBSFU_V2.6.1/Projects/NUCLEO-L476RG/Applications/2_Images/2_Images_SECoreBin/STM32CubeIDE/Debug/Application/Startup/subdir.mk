################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Application/Startup/se_key.s \
E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Middlewares/ST/STM32_Secure_Engine/Core/se_stack_smuggler_GNU.s \
../Application/Startup/startup_stm32l476xx.s 

OBJS += \
./Application/Startup/se_key.o \
./Application/Startup/se_stack_smuggler_GNU.o \
./Application/Startup/startup_stm32l476xx.o 

S_DEPS += \
./Application/Startup/se_key.d \
./Application/Startup/se_stack_smuggler_GNU.d \
./Application/Startup/startup_stm32l476xx.d 


# Each subdirectory must supply rules for building sources it contributes
Application/Startup/%.o: ../Application/Startup/%.s Application/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -g3 -c -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -o "$@" "$<"
Application/Startup/se_stack_smuggler_GNU.o: E:/Workspace/STM32_SECURITY_LAB/TEST_PROJECTS/STM32CubeExpansion_SBSFU_V2.6.1/Middlewares/ST/STM32_Secure_Engine/Core/se_stack_smuggler_GNU.s Application/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -g3 -c -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -o "$@" "$<"

clean: clean-Application-2f-Startup

clean-Application-2f-Startup:
	-$(RM) ./Application/Startup/se_key.d ./Application/Startup/se_key.o ./Application/Startup/se_stack_smuggler_GNU.d ./Application/Startup/se_stack_smuggler_GNU.o ./Application/Startup/startup_stm32l476xx.d ./Application/Startup/startup_stm32l476xx.o

.PHONY: clean-Application-2f-Startup

