#include "mat-add-qpu.h"

static void fill_values(uint32_t *buf)
{
    for (uint32_t i = 0; i < MAT_ADD_A_N; i++) {
        buf[MAT_ADD_A_OFF + i] = 32 + i;
        buf[MAT_ADD_B_OFF + i] = 64 + i;
        buf[MAT_ADD_C_OFF + i] = 0;
    }
}

void test_mat_add(void)
{
    uint32_t rows = 1024;
    uint32_t cols = 4096;

    uint32_t *buf = mat_add_init();
    fill_values(buf);

    printk("\nTesting generated-compiler matrix addition on GPU (%dx%d)...\n",
           rows, cols);
    printk("Memory before: Z[0]=%x Z[1]=%x Z[2]=%x Z[3]=%x\n",
           buf[MAT_ADD_C_OFF + 0], buf[MAT_ADD_C_OFF + 1],
           buf[MAT_ADD_C_OFF + 2], buf[MAT_ADD_C_OFF + 3]);

    int start_time = timer_get_usec();
    mat_add_kernel(&buf[MAT_ADD_A_OFF], &buf[MAT_ADD_B_OFF], &buf[MAT_ADD_C_OFF]);
    int gpu_time = timer_get_usec() - start_time;

    printk("Memory after:  Z[0]=%d Z[1]=%d Z[2]=%d Z[3]=%d\n",
           buf[MAT_ADD_C_OFF + 0], buf[MAT_ADD_C_OFF + 1],
           buf[MAT_ADD_C_OFF + 2], buf[MAT_ADD_C_OFF + 3]);

    for (uint32_t r = 0; r < rows; r++) {
        for (uint32_t c = 0; c < cols; c++) {
            uint32_t idx      = r * cols + c;
            uint32_t a        = buf[MAT_ADD_A_OFF + idx];
            uint32_t b        = buf[MAT_ADD_B_OFF + idx];
            uint32_t expected = a + b;
            if (buf[MAT_ADD_C_OFF + idx] != expected)
                panic("Z[%d][%d] = %d, expected %d + %d = %d\n",
                      r, c, buf[MAT_ADD_C_OFF + idx], a, b, expected);
        }
        if (r % (rows / 8 ? rows / 8 : 1) == 0) {
            uint32_t idx = r * cols;
            printk("Row %d: Z[%d][0] = %d + %d = %d. CORRECT\n",
                   r, r,
                   buf[MAT_ADD_A_OFF + idx], buf[MAT_ADD_B_OFF + idx],
                   buf[MAT_ADD_C_OFF + idx]);
        }
    }

    start_time = timer_get_usec();
    for (uint32_t i = 0; i < rows * cols; i++)
        buf[MAT_ADD_C_OFF + i] = buf[MAT_ADD_A_OFF + i] + buf[MAT_ADD_B_OFF + i];
    int cpu_time = timer_get_usec() - start_time;

    printk("GPU time: %d us\n", gpu_time);
    printk("CPU time: %d us\n", cpu_time);
    printk("Speedup:  %dx\n", cpu_time / gpu_time);

    mat_add_release();
    printk("SUCCESS\n");
}

void notmain(void)
{
    test_mat_add();
}
