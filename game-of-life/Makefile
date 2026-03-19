LIBS    += $(QPU_DSL_PATH)/lib/libgcc.a $(QPU_DSL_PATH)/libpi/libpi.a
CFLAGS  += -I$(QPU_DSL_PATH)/libpi/src

PROGS += run-game-of-life.c
PROGS += test-game-of-life.c

COMMON_SRC += game-of-life-qpu.c
COMMON_SRC += game-of-life.c
COMMON_SRC += sw-uart.c
COMMON_SRC += $(QPU_DSL_PATH)/libpi/src/mailbox.c
	LIB_SRC  += $(QPU_DSL_PATH)/libpi/src/gpio.c 
    LIB_SRC  += $(QPU_DSL_PATH)/libpi/src/uart.c

STAFF_OBJS += $(QPU_DSL_PATH)/libpi/staff-objs/staff-hw-spi.o
STAFF_OBJS += $(QPU_DSL_PATH)/libpi/staff-objs/kmalloc.o

RUN = 0

BOOTLOADER = my-install
EXCLUDE    ?= grep -v simple_boot
GREP_STR   := 'HASH:\|ERROR:\|PANIC:\|SUCCESS:\|NRF:'
include $(QPU_DSL_PATH)/libpi/mk/Makefile.robust

game-of-life.c game-of-life.h: game-of-life.qasm
	vc4asm -V -c game-of-life.c -h game-of-life.h $<
