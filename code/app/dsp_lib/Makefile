
DSPBASE = .

include BasicMathFunctions/files.mk
include CommonTables/files.mk
include ComplexMathFunctions/files.mk
include ControllerFunctions/files.mk
include FastMathFunctions/files.mk
include FilteringFunctions/files.mk
include MatrixFunctions/files.mk
include StatisticsFunctions/files.mk
include SupportFunctions/files.mk
include TransformFunctions/files.mk

TOOLCHAIN := arm-none-eabi-
CC := $(TOOLCHAIN)gcc
AR := $(TOOLCHAIN)ar

RM := rm -f

C_OBJ  := $(DSPSRC:%=%.o)
ASM_OBJ  := $(DSPASM:%=%.o)

CFLAGS := -mcpu=cortex-m4 -Os -ggdb -fomit-frame-pointer -falign-functions=16 -Iinc -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -fsingle-precision-constant -ffunction-sections -fdata-sections -fno-common  -Wall -Wextra -Wstrict-prototypes  -DCORTEX_USE_FPU=TRUE -DARM_MATH_CM4 -D__FPU_PRESENT -DTHUMB_PRESENT -mno-thumb-interwork -DTHUMB_NO_INTERWORKING -mthumb -DTHUMB

default: all
all: libmath_cm4f.a

libmath_cm4f.a: $(C_OBJ) $(ASM_OBJ)
	$(AR) rcs $@ $(C_OBJ) $(ASM_OBJ)
	
%.S.o: %.S
	$(CC) -c $(CFLAGS) -o $@ $^

%.c.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $^

show:
	@echo $(C_OBJ) $(ASM_OBJ)
	
.PHONY: clean

clean:
	@$(RM) $(C_OBJ) $(ASM_OBJ)
	@echo "Clean complete!"

mrproper: distclean
dist-clean: distclean
distclean: clean
	@$(RM) libmath_cm4f.a
	@echo "Distclean complete!"
