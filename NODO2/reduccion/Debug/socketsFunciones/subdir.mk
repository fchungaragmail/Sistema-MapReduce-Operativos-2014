################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../socketsFunciones/LecturaYEscritura.c \
../socketsFunciones/sockets.c 

OBJS += \
./socketsFunciones/LecturaYEscritura.o \
./socketsFunciones/sockets.o 

C_DEPS += \
./socketsFunciones/LecturaYEscritura.d \
./socketsFunciones/sockets.d 


# Each subdirectory must supply rules for building sources it contributes
socketsFunciones/%.o: ../socketsFunciones/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


