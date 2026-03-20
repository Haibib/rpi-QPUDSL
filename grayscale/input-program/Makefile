PROGS += grayscale-unix.c

RUN = 0

BOOTLOADER = my-install
EXCLUDE    ?= grep -v simple_boot
GREP_STR   := 'HASH:\|ERROR:\|PANIC:\|SUCCESS:\|NRF:'
include $(QPU_DSL_PATH)/libunix/mk/Makefile.unix

grayscale-unix.o: grayscale-unix.c
	$(CC) $(CFLAGS) -c $< -o $@