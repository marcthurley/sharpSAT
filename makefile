RM := rm -rf
SUBDIRS := \
src \

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS := \
./src/basic_types.cpp \
./src/component_management.cpp \
./src/component_types.cpp \
./src/instance.cpp \
./src/main.cpp \
./src/solver.cpp 

OBJS := \
basic_types.o \
component_management.o \
component_types.o \
instance.o \
main.o \
solver.o 

CPP_DEPS := \
basic_types.d \
component_management.d \
component_types.d \
instance.d \
main.d \
solver.d 

%.o: ./src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++-4.7 -DNDEBUG -O3 -std=c++11 -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

LIBS := -lgmpxx -lgmp


ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

# All Target
all: sharpSAT

# Tool invocations
sharpSAT: $(OBJS) 
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++-4.7 -L/usr/lib/ -o "sharpSAT" $(OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(CPP_DEPS) sharpSAT
	-@echo ' '

.PHONY: all clean dependents
