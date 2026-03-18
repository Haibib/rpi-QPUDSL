LIBS    += $(QPU_DSL_PATH)/lib/libgcc.a $(QPU_DSL_PATH)/libpi/libpi.a
CFLAGS  += -I$(QPU_DSL_PATH)/libpi/src

PROGS := test-slice-add.c

COMMON_SRC += slice-add-qpu.c
COMMON_SRC += slice-add.c
COMMON_SRC += $(QPU_DSL_PATH)/libpi/src/mailbox.c

STAFF_OBJS += $(QPU_DSL_PATH)/libpi/staff-objs/staff-hw-spi.o
STAFF_OBJS += $(QPU_DSL_PATH)/libpi/staff-objs/kmalloc.o

RUN = 1

BOOTLOADER = my-install
EXCLUDE    ?= grep -v simple_boot
GREP_STR   := 'HASH:\|ERROR:\|PANIC:\|SUCCESS:\|NRF:'
include $(QPU_DSL_PATH)/libpi/mk/Makefile.robust

slice-add.c slice-add.h: slice-add.qasm
	vc4asm -V -c slice-add.c -h slice-add.h $<
