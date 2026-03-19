#include "slice-add-qpu.h"
#include "sw-uart.h"

static void fill_values_from_uart(uint32_t *buf, sw_uart_t* uart) {
    for(uint32_t r = 0, c = 0; r < ROWS; c++) {
        if(c == COLS) {
            c = 0;
            r++;
        }
        buf[SLICE_ADD_A_OFF + r * COLS + c] = uart_get8();
    }
}

void test_slice_add(void)
{
    hw_uart_disable();
    // use pin 14 for tx, 15 for rx
    sw_uart_t uart = sw_uart_init(14,15, 115200);
    uint32_t *buf = slice_add_init();
    fill_values_from_uart(buf, &uart);

    printk("\nTesting slice addition on GPU (%dx%d)...\n", ROWS, COLS);

    int start_time = timer_get_usec();
    slice_add_kernel(&buf[SLICE_ADD_A_OFF], &buf[SLICE_ADD_B_OFF]);
    int gpu_time = timer_get_usec() - start_time;

    printk("B[1][1]=%d B[1][2]=%d B[128][128]=%d\n",
           buf[SLICE_ADD_B_OFF + 1*COLS + 1],
           buf[SLICE_ADD_B_OFF + 1*COLS + 2],
           buf[SLICE_ADD_B_OFF + 128*COLS + 128]);


    for (uint32_t r = 1; r < ROWS - 1; r++) {
        for (uint32_t c = 1; c < COLS - 1; c++) {
            uint32_t a0 = buf[SLICE_ADD_A_OFF + (r+1)*COLS + (c-1)];  // A[2:, :-2]
            uint32_t a1 = buf[SLICE_ADD_A_OFF + r*COLS     + c    ];  // A[1:-1, 1:-1]
            uint32_t a2 = buf[SLICE_ADD_A_OFF + (r-1)*COLS + c    ];  // A[:-2, 1:-1]
            uint32_t expected = a0 + a1 + a2;
            uint32_t got = buf[SLICE_ADD_B_OFF + r*COLS + c];
            if (got != expected)
                panic("B[%d][%d] = %d, expected %d + %d + %d = %d\n",
                      r, c, got, a0, a1, a2, expected);
        }
    }

    int cpu_start = timer_get_usec();
    for (uint32_t r = 1; r < ROWS - 1; r++) {
        for (uint32_t c = 1; c < COLS - 1; c++) {
            buf[SLICE_ADD_B_OFF + r*COLS + c] = buf[SLICE_ADD_A_OFF + (r+1)*COLS + (c-1)]
                              + buf[SLICE_ADD_A_OFF + r*COLS     + c    ]
                              + buf[SLICE_ADD_A_OFF + (r-1)*COLS + c    ];
        }
    }
    int cpu_time = timer_get_usec() - cpu_start;

    printk("GPU time: %d us\n", gpu_time);
    printk("CPU time: %d us\n", cpu_time);
    printk("Speedup: %dx\n", cpu_time / gpu_time);
    slice_add_release();
    printk("SUCCESS\n");
}

void notmain(void)
{
    test_slice_add();
}
