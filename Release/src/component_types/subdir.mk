################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/component_types/base_packed_component.cpp \
../src/component_types/component_archetype.cpp 

OBJS += \
./src/component_types/base_packed_component.o \
./src/component_types/component_archetype.o 

CPP_DEPS += \
./src/component_types/base_packed_component.d \
./src/component_types/component_archetype.d 


# Each subdirectory must supply rules for building sources it contributes
src/component_types/%.o: ../src/component_types/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++-4.8 -I/opt/local/include -O3 -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


