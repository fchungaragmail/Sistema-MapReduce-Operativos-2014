################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ConexionCenter.c \
../src/FilesStatusCenter.c \
../src/MaRTA.c \
../src/PlannerCenter.c \
../src/Serializador.c \
../src/Simulador.c \
../src/Utilities.c \
../src/protocolo.c 

OBJS += \
./src/ConexionCenter.o \
./src/FilesStatusCenter.o \
./src/MaRTA.o \
./src/PlannerCenter.o \
./src/Serializador.o \
./src/Simulador.o \
./src/Utilities.o \
./src/protocolo.o 

C_DEPS += \
./src/ConexionCenter.d \
./src/FilesStatusCenter.d \
./src/MaRTA.d \
./src/PlannerCenter.d \
./src/Serializador.d \
./src/Simulador.d \
./src/Utilities.d \
./src/protocolo.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


