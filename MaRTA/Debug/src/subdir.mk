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
../src/Utilities.c 

OBJS += \
./src/ConexionCenter.o \
./src/FilesStatusCenter.o \
./src/MaRTA.o \
./src/PlannerCenter.o \
./src/Serializador.o \
./src/Utilities.o 

C_DEPS += \
./src/ConexionCenter.d \
./src/FilesStatusCenter.d \
./src/MaRTA.d \
./src/PlannerCenter.d \
./src/Serializador.d \
./src/Utilities.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


