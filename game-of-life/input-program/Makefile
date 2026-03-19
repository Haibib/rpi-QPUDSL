PROGS += game-of-life-unix.c

RUN = 0

BOOTLOADER = my-install
EXCLUDE    ?= grep -v simple_boot
GREP_STR   := 'HASH:\|ERROR:\|PANIC:\|SUCCESS:\|NRF:'
include $(QPU_DSL_PATH)/libunix/mk/Makefile.unix

game-of-life-unix.o: game-of-life-unix.c
	$(CC) $(CFLAGS) -c $< -o $@