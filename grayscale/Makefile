LIBS    += $(QPU_DSL_PATH)/lib/libgcc.a $(QPU_DSL_PATH)/libpi/libpi.a
CFLAGS  += -I$(QPU_DSL_PATH)/libpi/src

PROGS += run-grayscale.c

COMMON_SRC += grayscale-qpu.c
COMMON_SRC += grayscale.c
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

grayscale.c grayscale.h: grayscale.qasm
	vc4asm -V -c grayscale.c -h grayscale.h $<
