################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/BatteryControlTask.c \
../Core/Src/DebugInterface.c \
../Core/Src/DisplayTask.c \
../Core/Src/GatekeeperTask.c \
../Core/Src/PrechargerTask.c \
../Core/Src/RxGateKeeperTask.c \
../Core/Src/ShutoffTask.c \
../Core/Src/StartupTask.c \
../Core/Src/TxGateKeeperTask.c \
../Core/Src/main.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c 

OBJS += \
./Core/Src/BatteryControlTask.o \
./Core/Src/DebugInterface.o \
./Core/Src/DisplayTask.o \
./Core/Src/GatekeeperTask.o \
./Core/Src/PrechargerTask.o \
./Core/Src/RxGateKeeperTask.o \
./Core/Src/ShutoffTask.o \
./Core/Src/StartupTask.o \
./Core/Src/TxGateKeeperTask.o \
./Core/Src/main.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o 

C_DEPS += \
./Core/Src/BatteryControlTask.d \
./Core/Src/DebugInterface.d \
./Core/Src/DisplayTask.d \
./Core/Src/GatekeeperTask.d \
./Core/Src/PrechargerTask.d \
./Core/Src/RxGateKeeperTask.d \
./Core/Src/ShutoffTask.d \
./Core/Src/StartupTask.d \
./Core/Src/TxGateKeeperTask.d \
./Core/Src/main.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/BatteryControlTask.cyclo ./Core/Src/BatteryControlTask.d ./Core/Src/BatteryControlTask.o ./Core/Src/BatteryControlTask.su ./Core/Src/DebugInterface.cyclo ./Core/Src/DebugInterface.d ./Core/Src/DebugInterface.o ./Core/Src/DebugInterface.su ./Core/Src/DisplayTask.cyclo ./Core/Src/DisplayTask.d ./Core/Src/DisplayTask.o ./Core/Src/DisplayTask.su ./Core/Src/GatekeeperTask.cyclo ./Core/Src/GatekeeperTask.d ./Core/Src/GatekeeperTask.o ./Core/Src/GatekeeperTask.su ./Core/Src/PrechargerTask.cyclo ./Core/Src/PrechargerTask.d ./Core/Src/PrechargerTask.o ./Core/Src/PrechargerTask.su ./Core/Src/RxGateKeeperTask.cyclo ./Core/Src/RxGateKeeperTask.d ./Core/Src/RxGateKeeperTask.o ./Core/Src/RxGateKeeperTask.su ./Core/Src/ShutoffTask.cyclo ./Core/Src/ShutoffTask.d ./Core/Src/ShutoffTask.o ./Core/Src/ShutoffTask.su ./Core/Src/StartupTask.cyclo ./Core/Src/StartupTask.d ./Core/Src/StartupTask.o ./Core/Src/StartupTask.su ./Core/Src/TxGateKeeperTask.cyclo ./Core/Src/TxGateKeeperTask.d ./Core/Src/TxGateKeeperTask.o ./Core/Src/TxGateKeeperTask.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su

.PHONY: clean-Core-2f-Src

