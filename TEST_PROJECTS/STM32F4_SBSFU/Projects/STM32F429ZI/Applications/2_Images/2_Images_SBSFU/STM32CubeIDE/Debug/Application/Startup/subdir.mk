################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Application/Startup/sfu_se_mpu.s \
../Application/Startup/startup_stm32f429zitx.s 

OBJS += \
./Application/Startup/sfu_se_mpu.o \
./Application/Startup/startup_stm32f429zitx.o 

S_DEPS += \
./Application/Startup/sfu_se_mpu.d \
./Application/Startup/startup_stm32f429zitx.d 


# Each subdirectory must supply rules for building sources it contributes
Application/Startup/%.o: ../Application/Startup/%.s Application/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -g3 -c -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Application-2f-Startup

clean-Application-2f-Startup:
	-$(RM) ./Application/Startup/sfu_se_mpu.d ./Application/Startup/sfu_se_mpu.o ./Application/Startup/startup_stm32f429zitx.d ./Application/Startup/startup_stm32f429zitx.o

.PHONY: clean-Application-2f-Startup

