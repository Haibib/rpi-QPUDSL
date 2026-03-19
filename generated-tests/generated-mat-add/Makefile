LIBS    += $(QPU_DSL_PATH)/lib/libgcc.a $(QPU_DSL_PATH)/libpi/libpi.a
CFLAGS  += -I$(QPU_DSL_PATH)/libpi/src

PROGS := test-gen-mat-add.c

COMMON_SRC += mat-add-qpu.c
COMMON_SRC += mat-add.c
COMMON_SRC += $(QPU_DSL_PATH)/libpi/src/mailbox.c

STAFF_OBJS += $(QPU_DSL_PATH)/libpi/staff-objs/staff-hw-spi.o
STAFF_OBJS += $(QPU_DSL_PATH)/libpi/staff-objs/kmalloc.o

RUN = 1

BOOTLOADER = my-install
EXCLUDE    ?= grep -v simple_boot
GREP_STR   := 'HASH:\|ERROR:\|PANIC:\|SUCCESS:\|NRF:'
include $(QPU_DSL_PATH)/libpi/mk/Makefile.robust

mat-add.c mat-add.h: mat-add.qasm
	vc4asm -V -c mat-add.c -h mat-add.h $<
