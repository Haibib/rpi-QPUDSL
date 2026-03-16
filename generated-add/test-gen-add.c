#include "gen-add.h"

static void fill_values(volatile struct genGPU *gpu, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        gpu->A[i] = 32 + i;
        gpu->B[i] = 64 + i;
        gpu->C[i] = 0;
    }
}

void test_gen_add(void)
{
    uint32_t len      = 1024 * 1024;
    uint32_t num_qpus = 8;

    volatile struct genGPU *gpu;
    vec_gen_add_init(&gpu, len, num_qpus);
    fill_values(gpu, len);

    printk("\nTesting generated-compiler addition on GPU...\n");
    printk("Memory before: C[0]=%x C[1]=%x C[2]=%x C[3]=%x\n",
           gpu->C[0], gpu->C[1], gpu->C[2], gpu->C[3]);

    int start_time = timer_get_usec();
    gen_add_execute(gpu);
    int gpu_time = timer_get_usec() - start_time;

    printk("Memory after:  C[0]=%d C[1]=%d C[2]=%d C[3]=%d\n",
           gpu->C[0], gpu->C[1], gpu->C[2], gpu->C[3]);

    for (uint32_t i = 0; i < len; i++) {
        uint32_t expected = gpu->A[i] + gpu->B[i];
        if (gpu->C[i] != expected)
            panic("Iteration %d: %d + %d = %d, expected %d\n",
                  i, gpu->A[i], gpu->B[i], gpu->C[i], expected);
        if (i % (len / 8) == 0)
            printk("Iteration %d: %d + %d = %d. CORRECT\n",
                   i, gpu->A[i], gpu->B[i], gpu->C[i]);
    }

    start_time = timer_get_usec();
    for (uint32_t i = 0; i < len; i++)
        gpu->C[i] = gpu->A[i] + gpu->B[i];
    int cpu_time = timer_get_usec() - start_time;

    printk("GPU time: %d us\n", gpu_time);
    printk("CPU time: %d us\n", cpu_time);
    printk("Speedup:  %dx\n", cpu_time / gpu_time);

    vec_gen_add_release(gpu);
    printk("SUCCESS\n");
}

void notmain(void)
{
    test_gen_add();
}
