################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/delay.c 

C_DEPS += \
./source/delay.d 

OBJS += \
./source/delay.o 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c source/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DCPU_MKL25Z128VLK4_cm0plus -DCPU_MKL25Z128VLK4 -DFSL_RTOS_BM -DSDK_OS_BAREMETAL -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"C:\Users\omarp\Documents\MCUXpressoIDE_11.9.0_2144\workspace\DELAY\board" -I"C:\Users\omarp\Documents\MCUXpressoIDE_11.9.0_2144\workspace\DELAY\source" -I"C:\Users\omarp\Documents\MCUXpressoIDE_11.9.0_2144\workspace\DELAY" -I"C:\Users\omarp\Documents\MCUXpressoIDE_11.9.0_2144\workspace\DELAY\drivers" -I"C:\Users\omarp\Documents\MCUXpressoIDE_11.9.0_2144\workspace\DELAY\utilities" -I"C:\Users\omarp\Documents\MCUXpressoIDE_11.9.0_2144\workspace\DELAY\CMSIS" -O0 -fno-common -g3 -gdwarf-4 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-source

clean-source:
	-$(RM) ./source/delay.d ./source/delay.o

.PHONY: clean-source

