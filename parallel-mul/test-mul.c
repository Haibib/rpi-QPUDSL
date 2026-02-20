#include "parallel-mul.h"

void fill_values(volatile struct mulGPU *mul_gpu, uint32_t len) {
    for (uint32_t i = 0; i < len; i++){
        mul_gpu->A[i] = 32 + i;
        mul_gpu->B[i] = 64 + i;
        mul_gpu->C[i] = 0;
    }
}

/* SIMPLE TEST TO COMPARE YOUR KERNEL WITH CPU OUTPUT */
void test_mul(void)
{
    int len = 2097143;
    int num_qpus = 8;
    int i, j;
    volatile struct mulGPU *mul_gpu;
    vec_mul_init(&mul_gpu, len, num_qpus);
    fill_values(mul_gpu, len);

    // Test multiplication
    printk("\nTesting multiplication on GPU...\n");
    printk("Memory before multiplication: %x %x %x %x\n", mul_gpu->C[0], mul_gpu->C[1], mul_gpu->C[2], mul_gpu->C[3]);

    int start_time = timer_get_usec();
    int iret_mul = mul_gpu_execute(mul_gpu);
    int end_time = timer_get_usec();
    int gpu_mul_time = end_time - start_time;

    printk("Memory after multiplicaiton:  %d %d %d %d\n", mul_gpu->C[0], mul_gpu->C[1], mul_gpu->C[2], mul_gpu->C[3]);

    for (i = 0; i < len; i++)
    {
        if (mul_gpu->C[i] != (32 + i) * (64 + i))
        {
            panic("Mul Iteration %d: %d * %d = %d. INCORRECT\n", i, mul_gpu->A[i], mul_gpu->B[i], mul_gpu->C[i]);
        }
        else if (i*4 % N == 0)
        {
            printk("Mul Iteration %d: %d * %d = %d. CORRECT\n", i, mul_gpu->A[i], mul_gpu->B[i], mul_gpu->C[i]);
        }
    }
    start_time = timer_get_usec();
    for (i = 0; i < len; i++)
        mul_gpu->C[i] = mul_gpu->A[i] * mul_gpu->B[i];
    end_time = timer_get_usec();
    int cpu_mul_time = end_time - start_time;

    printk("CPU Addition Time: %d us\n", cpu_mul_time);
    printk("GPU Addition Time: %d us\n", gpu_mul_time);

    printk("Speedup: %dx\n", cpu_mul_time / gpu_mul_time);

    vec_mul_release(mul_gpu);
}

void notmain(void)
{
    printk("Testing multiplication on GPU...\n");
    test_mul();
}