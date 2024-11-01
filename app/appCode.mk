# This file contains compilation instructions of the application and FreeRTOS.
# THIS IS NOT A STANDALONE MAKE FILE. 
# ==========================================
# Possible improvements:
# * Make for an easy way to change the heap type 
# * Set a variable for when the processor has a Memory Protected Unit
# ==========================================
# Made by Jorge Pérez 4 Owltech <3

# I knew what to put in here from the folowing sources:
# https://www.freertos.org/Documentation/01-FreeRTOS-quick-start/01-Beginners-guide/03-Build-your-first-project
## Only 3 source files that are common to all RTOS ports, 
## and one microcontroller specific source file are required, 
## and its API is designed to be simple and intuitive.
# Required Source files
# * FreeRTOS/Source/tasks.c
# * FreeRTOS/Source/queue.c
# * FreeRTOS/Source/list.c
# * FreeRTOS/Source/portable/[compiler]/[architecture]/port.c.
# * FreeRTOS/Source/portable/MemMang/heap_x.c where 'x' is 1, 2, 3, 4 or 5.
# Required Header files
# * FreeRTOS/Source/include
# * FreeRTOS/Source/portable/[compiler]/[architecture].
# * Whichever directory contains the FreeRTOSConfig.h file to be used - see the Configuration File paragraph below.
# * Depending on the port, it may also be necessary for the same directories to be in the assembler's include path.

# Add FreeRTOS includes 
ifeq ($(USE_FREERTOS), 1)
C_INCLUDES += \
	-I$(FirmwarePath)/Middlewares/Third_Party/FreeRTOS/Source/include \
	-I$(FirmwarePath)/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS \
	-I$(FirmwarePath)/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/$(ARCH_ARM) \
	-Iapp/inc
endif

# Add FreeRTOS sources 
ifeq ($(USE_FREERTOS), 1)
	C_SOURCES += $(shell find $(FirmwarePath)/Middlewares/Third_Party/FreeRTOS/Source/ -maxdepth 1 -name *.c)
	C_SOURCES += $(shell find $(FirmwarePath)/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -name *.c)
	C_SOURCES += $(shell find $(FirmwarePath)/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/$(ARCH_ARM)/ -name *.c)
	C_SOURCES += $(shell find $(FirmwarePath)/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_1.c)
	C_SOURCES += $(shell find app/src/*.c)
# C_SOURCES += $(shell find $(FirmwarePath)/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3_MPU/ -name *.c)
endif

