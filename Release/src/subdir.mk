################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/alt_component_analyzer.cpp \
../src/component_analyzer.cpp \
../src/component_cache.cpp \
../src/component_management.cpp \
../src/instance.cpp \
../src/main.cpp \
../src/new_component_analyzer.cpp \
../src/solver.cpp \
../src/statistics.cpp 

OBJS += \
./src/alt_component_analyzer.o \
./src/component_analyzer.o \
./src/component_cache.o \
./src/component_management.o \
./src/instance.o \
./src/main.o \
./src/new_component_analyzer.o \
./src/solver.o \
./src/statistics.o 

CPP_DEPS += \
./src/alt_component_analyzer.d \
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
	g++-4.8 -I/opt/local/include -O3 -pg -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/component_cache.o: ../src/component_cache.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++-4.8 -I/usr/include/c++/4.8.1 -I/usr/include -O3 -pg -Wall -c -fmessage-length=0 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"src/component_cache.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


