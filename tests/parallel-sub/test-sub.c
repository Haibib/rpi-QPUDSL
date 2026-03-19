#include "parallel-sub.h"

void fill_values(volatile struct subGPU *sub_gpu, uint32_t len) {
    for (uint32_t i = 0; i < len; i++){
        sub_gpu->A[i] = 64 + i;
        sub_gpu->B[i] = 32 - i;
        sub_gpu->C[i] = 0;
    }
}

/* SIMPLE TEST TO COMPARE YOUR KERNEL WITH CPU OUTPUT */
void test_sub(void)
{
    int len = 2097143;
    int num_qpus = 8;
    int i, j;
    volatile struct subGPU *sub_gpu;
    vec_sub_init(&sub_gpu, len, num_qpus);
    fill_values(sub_gpu, len);

    // Test subtraction
    printk("\nTesting subtraction on GPU...\n");
    printk("Memory before subtraction: %x %x %x %x\n", sub_gpu->C[0], sub_gpu->C[1], sub_gpu->C[2], sub_gpu->C[3]);

    int start_time = timer_get_usec();
    int iret_sub = sub_gpu_execute(sub_gpu);
    int end_time = timer_get_usec();
    int gpu_sub_time = end_time - start_time;

    printk("Memory after subtiplicaiton:  %d %d %d %d\n", sub_gpu->C[0], sub_gpu->C[1], sub_gpu->C[2], sub_gpu->C[3]);

    for (i = 0; i < len; i++)
    {
        if (sub_gpu->C[i] != (64 + i) - (32 - i))
        {
            panic("sub Iteration %d: %d - %d = %d. INCORRECT\n", i, sub_gpu->A[i], sub_gpu->B[i], sub_gpu->C[i]);
        }
        else if (i*4 % N == 0)
        {
            printk("sub Iteration %d: %d - %d = %d. CORRECT\n", i, sub_gpu->A[i], sub_gpu->B[i], sub_gpu->C[i]);
        }
    }
    start_time = timer_get_usec();
    for (i = 0; i < len; i++)
        sub_gpu->C[i] = sub_gpu->A[i] * sub_gpu->B[i];
    end_time = timer_get_usec();
    int cpu_sub_time = end_time - start_time;

    printk("CPU Addition Time: %d us\n", cpu_sub_time);
    printk("GPU Addition Time: %d us\n", gpu_sub_time);

    printk("Speedup: %dx\n", cpu_sub_time / gpu_sub_time);

    vec_sub_release(sub_gpu);
}

void notmain(void)
{
    printk("Testing subtraction on GPU...\n");
    test_sub();
}