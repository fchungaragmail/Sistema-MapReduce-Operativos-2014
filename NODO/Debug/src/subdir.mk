################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/aparearArchivos_new.c \
../src/nodo_fs_functions_new.c \
../src/nodo_job_functions_new.c \
../src/nodo_new.c \
../src/protocolo_new.c 

OBJS += \
./src/aparearArchivos_new.o \
./src/nodo_fs_functions_new.o \
./src/nodo_job_functions_new.o \
./src/nodo_new.o \
./src/protocolo_new.o 

C_DEPS += \
./src/aparearArchivos_new.d \
./src/nodo_fs_functions_new.d \
./src/nodo_job_functions_new.d \
./src/nodo_new.d \
./src/protocolo_new.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


