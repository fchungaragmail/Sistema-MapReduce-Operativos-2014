################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/PackedSerialization.c \
../src/Serialization.c \
../src/SerializationFW.c 

OBJS += \
./src/PackedSerialization.o \
./src/Serialization.o \
./src/SerializationFW.o 

C_DEPS += \
./src/PackedSerialization.d \
./src/Serialization.d \
./src/SerializationFW.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


