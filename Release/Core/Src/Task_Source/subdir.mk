################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Task_Source/Collector_Math_Task.c 

OBJS += \
./Core/Src/Task_Source/Collector_Math_Task.o 

C_DEPS += \
./Core/Src/Task_Source/Collector_Math_Task.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Task_Source/%.o: ../Core/Src/Task_Source/%.c Core/Src/Task_Source/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"F:/STM32CubeIDE/Projects/Diplom_First_Step/Core/Inc/Task_Include" -I"F:/STM32CubeIDE/Projects/Diplom_First_Step/Library_FREERTOS_HAL/Include" -I"F:/STM32CubeIDE/Projects/Diplom_First_Step/Library_math/Include" -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

