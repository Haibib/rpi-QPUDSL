PROGS = my-install.c
COMMON_SRC = put-code.c
CFLAGS += -I../2-pi-side

# these are local copies of libunix routines in case you want to 
# modify them.
COMMON_SRC += $(QPU_DSL_PATH)/libunix/handoff-to.c
COMMON_SRC += $(QPU_DSL_PATH)/libunix/get-exitcode.c
COMMON_SRC += pi-echo.c
COMMON_SRC += set-tty-8n1.c

include $(QPU_DSL_PATH)/libunix/mk/Makefile.unix

trace:
	./my-install --trace-control hello.bin

trace-all:
	./my-install --trace-all hello.bin

.PHONY: trace trace-all
