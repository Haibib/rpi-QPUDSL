#include "grayscale-qpu.h"
#include "sw-uart.h"
#include "rpi.h"

#define ROWS 446
#define COLS 576
#define READY_SIGNAL 0xAB
#define TOTAL_CELLS  (ROWS * COLS)

static uint8_t ch_r[TOTAL_CELLS];
static uint8_t ch_g[TOTAL_CELLS];
static uint8_t ch_b[TOTAL_CELLS];

static void recv_channel(uint8_t *dst) {
    for (uint32_t i = 0; i < TOTAL_CELLS; i++)
        dst[i] = (uint8_t)uart_get8();
}

static void send_channel(const uint8_t *src) {
    for (uint32_t i = 0; i < TOTAL_CELLS; i++)
        uart_put8(src[i]);
}

void notmain(void)
{
    uint32_t *buf = grayscale_init();
    uart_init();
    uart_put8(READY_SIGNAL);
    recv_channel((uint8_t *)buf[GRAYSCALE_R_OFF]);
    recv_channel((uint8_t *)buf[GRAYSCALE_G_OFF]);
    recv_channel((uint8_t *)buf[GRAYSCALE_B_OFF]);
    grayscale_kernel((uint8_t *)buf[GRAYSCALE_R_OFF], (uint8_t *)buf[GRAYSCALE_G_OFF], (uint8_t *)buf[GRAYSCALE_B_OFF], (uint8_t *)buf[GRAYSCALE_GREY_OFF], 0.3, 0.59, 0.11);
    send_channel((uint8_t *)buf[GRAYSCALE_GREY_OFF]);
}
