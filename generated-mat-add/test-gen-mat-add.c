#include "gen-mat-add.h"

static uint32_t g_rows;
static uint32_t g_cols;

static void fill_values(volatile struct matGPU *gpu)
{
    for (uint32_t i = 0; i < g_rows * g_cols; i++) {
        gpu->A[i] = 32 + i;
        gpu->B[i] = 64 + i;
        gpu->C[i] = 0;
    }
}

void test_mat_add(void)
{
    g_rows    = ROWS;
    g_cols    = COLS;
    uint32_t num_qpus = 8;

    volatile struct matGPU *gpu;
    vec_mat_add_init(&gpu, g_rows, g_cols, num_qpus);
    fill_values(gpu);

    printk("\nTesting generated-compiler matrix addition on GPU (%dx%d)...\n",
           g_rows, g_cols);
    printk("Memory before: C[0]=%x C[1]=%x C[2]=%x C[3]=%x\n",
           gpu->C[0], gpu->C[1], gpu->C[2], gpu->C[3]);

    int start_time = timer_get_usec();
    mat_add_execute(gpu);
    int gpu_time = timer_get_usec() - start_time;

    printk("Memory after:  C[0]=%d C[1]=%d C[2]=%d C[3]=%d\n",
           gpu->C[0], gpu->C[1], gpu->C[2], gpu->C[3]);

    for (uint32_t r = 0; r < g_rows; r++) {
        for (uint32_t c = 0; c < g_cols; c++) {
            uint32_t idx      = r * g_cols + c;
            uint32_t expected = gpu->A[idx] + gpu->B[idx];
            if (gpu->C[idx] != expected)
                panic("C[%d][%d] = %d, expected %d + %d = %d\n",
                      r, c, gpu->C[idx], gpu->A[idx], gpu->B[idx], expected);
        }
        if (r % (g_rows / 8 ? g_rows / 8 : 1) == 0) {
            uint32_t idx = r * g_cols;
            printk("Row %d: C[%d][0] = %d + %d = %d. CORRECT\n",
                   r, r, gpu->A[idx], gpu->B[idx], gpu->C[idx]);
        }
    }

    start_time = timer_get_usec();
    for (uint32_t i = 0; i < g_rows * g_cols; i++)
        gpu->C[i] = gpu->A[i] + gpu->B[i];
    int cpu_time = timer_get_usec() - start_time;

    printk("GPU time: %d us\n", gpu_time);
    printk("CPU time: %d us\n", cpu_time);
    printk("Speedup:  %dx\n", cpu_time / gpu_time);

    vec_mat_add_release(gpu);
    printk("SUCCESS\n");
}

void notmain(void)
{
    test_mat_add();
}
