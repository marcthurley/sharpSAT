################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/component_analyzer.cpp \
../src/component_cache.cpp \
../src/component_management.cpp \
../src/instance.cpp \
../src/main.cpp \
../src/new_component_analyzer.cpp \
../src/solver.cpp \
../src/statistics.cpp 

OBJS += \
./src/component_analyzer.o \
./src/component_cache.o \
./src/component_management.o \
./src/instance.o \
./src/main.o \
./src/new_component_analyzer.o \
./src/solver.o \
./src/statistics.o 

CPP_DEPS += \
./src/component_analyzer.d \
./src/component_cache.d \
./src/component_management.d \
./src/instance.d \
./src/main.d \
./src/new_component_analyzer.d \
./src/solver.d \
./src/statistics.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++-4.7 -DNDEBUG -O3 -Wall -std=c++11 -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


