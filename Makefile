##########################################################################################################################
# Makfile to compile an Owltech project
# For more on this topic, see: 
# https://metebalci.com/blog/demystifying-arm-gnu-toolchain-specs-nano-and-nosys/
##########################################################################################################################

######################################
# target
######################################
TARGET = owltech

#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
#	-x assembler-with-cpp -> Indicates that the project includes C/C++ giles
# 		also, enables our assembler to understand preprocesor tags
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)g++
AS = $(GCC_PATH)/$(PREFIX)g++ -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)g++
AS = $(PREFIX)g++ -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og
# C defines
C_DEFS = \
-DUSE_HAL_DRIVER

#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES = 

# C includes
C_INCLUDES = 

# ASM sources
ASMM_SOURCES = 

# macros for gcc
# AS defines
AS_DEFS = 

# AS includes
## TODO: Leave? or remove
# Seems to be good without it
AS_INCLUDES = #\
-IrobotConfig/inc

# NOTE: You need to define that you are using RTOS 
USE_FREERTOS = 1

# Import proyect dependencies
include robotConfig/robotConfig.mk
# Import proyect dependencies
include app/appCode.mk

# TODO: SHOULD WE SELECT -std=gnu11??
# compile gcc flags
# 	-Wall -> all warnings
#	-fdata-sections -> Place each function or data item into its own section in the output file
#	-ffunction-sections -> Optimization

ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

#######################################
# LDFLAGS
#######################################

# libraries
#	-lc Link the standard C library
#	-lm Link the math library
#	-lnosys This flag tells the linker to not to use any system-specific libraries or startup code
# -lstdc++ \
# 				-libstdc++ \
# 				-libm
# --gc-sections -static 
# --start-group -lc -lm -lstdc++ -lsupc++ -Wl,--end-group
LIBS = -lc -lm -lstdc++ -lsupc++
LIBDIR = 
LDFLAGS = $(MCU)  --specs=nosys.specs --specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################


# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASMM_SOURCES:.S=.o)))
vpath %.S $(sort $(dir $(ASMM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@
$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR)
	$(CXX) -c $(CXXFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.cpp=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@
$(BUILD_DIR)/%.o: %.S Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@
# Make .hex files from all .elf files in the build dir
$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

# Make .bin files from all .elf files in the build dir
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
# Create build dir 
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***