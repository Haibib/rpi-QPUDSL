#include "gen-add-qpu.h"

static void fill_values(uint32_t *buf, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        buf[GEN_ADD_A_OFF + i] = 32 + i;
        buf[GEN_ADD_B_OFF + i] = 64 + i;
        buf[GEN_ADD_C_OFF + i] = 0;
    }
}

void test_gen_add(void)
{
    uint32_t len = GEN_ADD_A_N;

    uint32_t *buf = gen_add_init();
    fill_values(buf, len);

    printk("\nTesting generated-compiler addition on GPU...\n");
    printk("Memory before: Z[0]=%x Z[1]=%x Z[2]=%x Z[3]=%x\n",
           buf[GEN_ADD_C_OFF + 0], buf[GEN_ADD_C_OFF + 1],
           buf[GEN_ADD_C_OFF + 2], buf[GEN_ADD_C_OFF + 3]);

    int start_time = timer_get_usec();
    gen_add_kernel(&buf[GEN_ADD_A_OFF], &buf[GEN_ADD_B_OFF], &buf[GEN_ADD_C_OFF]);
    int gpu_time = timer_get_usec() - start_time;

    printk("Memory after:  Z[0]=%d Z[1]=%d Z[2]=%d Z[3]=%d\n",
           buf[GEN_ADD_C_OFF + 0], buf[GEN_ADD_C_OFF + 1],
           buf[GEN_ADD_C_OFF + 2], buf[GEN_ADD_C_OFF + 3]);

    for (uint32_t i = 0; i < len; i++) {
        uint32_t a        = buf[GEN_ADD_A_OFF + i];
        uint32_t b        = buf[GEN_ADD_B_OFF + i];
        uint32_t expected = a + b;
        if (buf[GEN_ADD_C_OFF + i] != expected)
            panic("Iteration %d: %d + %d = %d, expected %d\n",
                  i, a, b, buf[GEN_ADD_C_OFF + i], expected);
        if (i % (len / 8) == 0)
            printk("Iteration %d: %d + %d = %d. CORRECT\n",
                   i, a, b, buf[GEN_ADD_C_OFF + i]);
    }

    start_time = timer_get_usec();
    for (uint32_t i = 0; i < len; i++)
        buf[GEN_ADD_C_OFF + i] = buf[GEN_ADD_A_OFF + i] + buf[GEN_ADD_B_OFF + i];
    int cpu_time = timer_get_usec() - start_time;

    printk("GPU time: %d us\n", gpu_time);
    printk("CPU time: %d us\n", cpu_time);
    printk("Speedup:  %dx\n", cpu_time / gpu_time);

    gen_add_release();
    printk("SUCCESS\n");
}

void notmain(void)
{
    test_gen_add();
}
