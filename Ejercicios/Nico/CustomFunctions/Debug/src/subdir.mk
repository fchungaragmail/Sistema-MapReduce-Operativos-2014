################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/CustomFunctions.c \
../src/MapFunctions.c \
../src/Pruebas.c \
../src/ScriptFunctions.c 

OBJS += \
./src/CustomFunctions.o \
./src/MapFunctions.o \
./src/Pruebas.o \
./src/ScriptFunctions.o 

C_DEPS += \
./src/CustomFunctions.d \
./src/MapFunctions.d \
./src/Pruebas.d \
./src/ScriptFunctions.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


