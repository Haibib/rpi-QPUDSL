LIBS    += $(QPU_DSL_PATH)/lib/libgcc.a $(QPU_DSL_PATH)/libpi/libpi.a
CFLAGS  += -I$(QPU_DSL_PATH)/libpi/src

PROGS := test-gen-mat-add.c

COMMON_SRC += gen-mat-add.c
COMMON_SRC += mat_add_shader.c
COMMON_SRC += $(QPU_DSL_PATH)/libpi/src/mailbox.c

STAFF_OBJS += $(QPU_DSL_PATH)/libpi/staff-objs/staff-hw-spi.o
STAFF_OBJS += $(QPU_DSL_PATH)/libpi/staff-objs/kmalloc.o

RUN = 1

BOOTLOADER = my-install
EXCLUDE    ?= grep -v simple_boot
GREP_STR   := 'HASH:\|ERROR:\|PANIC:\|SUCCESS:\|NRF:'
include $(QPU_DSL_PATH)/libpi/mk/Makefile.robust

mat_add_shader.c mat_add_shader.h: mat_add_shader.qasm
	vc4asm -V -c mat_add_shader.c -h mat_add_shader.h $<
