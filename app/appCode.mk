# This file contains compilation instructions dependent on the development board.
# THIS IS NOT A STANDALONE MAKE FILE. 
# ==========================================
# Possible improvements:
# * Don't compile everything, only the enabled modules at stm32f1xx_hal_conf.h
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
ifdef USE_FREERTOS
C_INCLUDES += \
	-I$(FirmwarePath)/Middlewares/Third_Party/FreeRTOS/Source/include \
	-I$(FirmwarePath)/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS \
	-I$(FirmwarePath)/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 \
	-Iapp/inc
endif

# Add FreeRTOS sources 
ifdef USE_FREERTOS

# TODO: know what to use in memmang, and in cm3 with or without mpu 
C_SOURCES += \
	C_SOURCES += $(shell find $(FirmwarePath)/Middlewares/Third_Party/FreeRTOS/Source/ -maxdepth 1 -name *.c) \
	C_SOURCES += $(shell find $(FirmwarePath)/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/ -name *.c) \
	C_SOURCES += $(shell find $(FirmwarePath)/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3/ -name *.c) \
	C_SOURCES += $(shell find $(FirmwarePath)/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/ -name *.c)
# C_SOURCES += $(shell find $(FirmwarePath)/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3_MPU/ -name *.c)

endif

