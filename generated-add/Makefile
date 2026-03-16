LIBS    += $(QPU_DSL_PATH)/lib/libgcc.a $(QPU_DSL_PATH)/libpi/libpi.a
CFLAGS  += -I$(QPU_DSL_PATH)/libpi/src

PROGS := test-gen-add.c

COMMON_SRC += gen-add.c
COMMON_SRC += genshader.c
COMMON_SRC += $(QPU_DSL_PATH)/libpi/src/mailbox.c

STAFF_OBJS += $(QPU_DSL_PATH)/libpi/staff-objs/staff-hw-spi.o
STAFF_OBJS += $(QPU_DSL_PATH)/libpi/staff-objs/kmalloc.o

RUN = 1

BOOTLOADER = my-install
EXCLUDE    ?= grep -v simple_boot
GREP_STR   := 'HASH:\|ERROR:\|PANIC:\|SUCCESS:\|NRF:'
include $(QPU_DSL_PATH)/libpi/mk/Makefile.robust

genshader.c genshader.h: genshader.qasm
	vc4asm -V -c genshader.c -h genshader.h -I $(QPU_DSL_PATH)/share/vc4inc/ $<
