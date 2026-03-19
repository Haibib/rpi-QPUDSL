#include "GoL-qpu.h"
#include "sw-uart.h"
#include "rpi.h"

#define READY_SIGNAL 0xAB
#define TOTAL_CELLS  (ROWS * COLS)
#define PACKED_BYTES ((TOTAL_CELLS + 7) / 8)

static void fill_values_from_uart(uint32_t *buf) {
    uint8_t packed[PACKED_BYTES];
    for (uint32_t i = 0; i < PACKED_BYTES; i++) {
        int v = uart_get8();
        packed[i] = (uint8_t)v;
    }
    for (uint32_t i = 0; i < PACKED_BYTES; i++) {
        for (int bit = 0; bit < 8; bit++) {
            uint32_t idx = i * 8 + bit;
            if (idx < TOTAL_CELLS)
                buf[GOL_A_OFF + idx] = (packed[i] >> bit) & 1;
        }
    }
}

static void send_values_to_uart(uint32_t *buf) {
    uint8_t packed[PACKED_BYTES];
    memset(packed, 0, sizeof packed);
    for (uint32_t i = 0; i < PACKED_BYTES; i++) {
        for (int bit = 0; bit < 8; bit++) {
            uint32_t idx = i * 8 + bit;
            if (idx < TOTAL_CELLS && buf[GOL_A_OFF + idx])
                packed[i] |= (1 << bit);
        }
    }
    for (uint32_t i = 0; i < PACKED_BYTES; i++)
        uart_put8(packed[i]);
}

static void flip_bits(uint32_t *buf) {
    for (uint32_t i = 0; i < ROWS * COLS; i++) {
        buf[GOL_B_OFF + i] = buf[GOL_A_OFF + i] ^ 1;
    }
}

void run_bitflip(uint32_t *buf) {
    flip_bits(buf);
    send_values_to_uart(buf);
}

void run_GoL(uint32_t *buf) {
    GoL_kernel(&buf[GOL_A_OFF], &buf[GOL_B_OFF]);
    for (uint32_t r = 1; r < ROWS - 1; r++) {
        for (uint32_t c = 1; c < COLS - 1; c++) {
            uint32_t i = r * COLS + c;
            // Alive if 3 neighbors or 2 neighbors and was previously alive
            buf[GOL_A_OFF + i] = (buf[GOL_B_OFF + i] == 3 || (buf[GOL_B_OFF + i] == 2&& buf[GOL_A_OFF + i])) ? 1 : 0;
        }
    }
    send_values_to_uart(buf);
}

void notmain(void)
{
    uint32_t *buf = GoL_init();
    uart_init();
    uart_put8(READY_SIGNAL);
    fill_values_from_uart(buf);
    while (1) {
        run_GoL(buf);
    }
}
