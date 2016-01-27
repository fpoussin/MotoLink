# List of the ChibiOS generic STM32F3xx startup and CMSIS files.
STARTUPSRC = $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC/crt1.c \
             ../common/vectors.c
          
STARTUPASM = $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC/crt0_v7m.s

STARTUPINC = $(CHIBIOS)/os/common/ports/ARMCMx/devices/STM32F3xx \
             $(CHIBIOS)/os/ext/CMSIS/ST/STM32F3xx \
             $(CHIBIOS)/os/ext/CMSIS/include

STARTUPLD  = $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC/ld
