#include "game-of-life-qpu.h"

#define ROWS 258
#define COLS 258

static void fill_values(uint32_t *buf)
{
    for (uint32_t i = 0; i < ROWS * COLS; i++) {
        uint32_t value = i % 2 ? 1 : 0;
        buf[GAME_OF_LIFE_A_OFF + i] = value;
        buf[GAME_OF_LIFE_B_OFF + i] = 0;
    }
}

// void run_test(int N) {

// }

void test_game_of_life(void)
{
    uint32_t *buf = game_of_life_init();
    fill_values(buf);

    printk("\nTesting slice addition on GPU (%dx%d)...\n", ROWS, COLS);

    int N = 10;
    uint32_t gpu_time = 0;
    for (int i = 0; i < N; i++) {
        uint32_t start_time = timer_get_usec();
        game_of_life_kernel(&buf[GAME_OF_LIFE_A_OFF], &buf[GAME_OF_LIFE_B_OFF]);
        gpu_time += timer_get_usec() - start_time;
    }
    gpu_time /= N;

    printk("B[1][1]=%d B[1][2]=%d B[1][3]=%d B[128][128]=%d\n",
           buf[GAME_OF_LIFE_B_OFF + 1*COLS + 1],
           buf[GAME_OF_LIFE_B_OFF + 1*COLS + 2],
           buf[GAME_OF_LIFE_B_OFF + 1*COLS + 3],
           buf[GAME_OF_LIFE_B_OFF + 128*COLS + 128]);


    for (uint32_t r = 1; r < ROWS - 1; r++) {
        for (uint32_t c = 1; c < COLS - 1; c++) {
            uint32_t a0 = buf[GAME_OF_LIFE_A_OFF + (r-1)*COLS + (c-1)];  // A[0:-2, 0:-2]
            uint32_t a1 = buf[GAME_OF_LIFE_A_OFF + (r-1)*COLS + c    ];  // A[0:-2, 1:-1]
            uint32_t a2 = buf[GAME_OF_LIFE_A_OFF + (r-1)*COLS + (c+1)];  // A[0:-2, 2:0]
            uint32_t a3 = buf[GAME_OF_LIFE_A_OFF + r*COLS + (c-1)    ];  // A[:-2, 1:-1]
            uint32_t a4 = buf[GAME_OF_LIFE_A_OFF + r*COLS + (c+1)    ];  // A[:-2, 1:-1]
            uint32_t a5 = buf[GAME_OF_LIFE_A_OFF + (r+1)*COLS + (c-1)];  // A[:-2, 1:-1]
            uint32_t a6 = buf[GAME_OF_LIFE_A_OFF + (r+1)*COLS + c    ];  // A[:-2, 1:-1]
            uint32_t a7 = buf[GAME_OF_LIFE_A_OFF + (r+1)*COLS + (c+1)];  // A[:-2, 1:-1]
            uint32_t expected = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7;
            uint32_t got = buf[GAME_OF_LIFE_B_OFF + r*COLS + c];
            if (got != expected)
                panic("B[%d][%d] = %d, expected %d + %d + %d = %d\n",
                      r, c, got, a0, a1, a2, expected);
        }
    }

    int cpu_start = timer_get_usec();
    for (uint32_t r = 1; r < ROWS - 1; r++) {
        for (uint32_t c = 1; c < COLS - 1; c++) {
            uint32_t a0 = buf[GAME_OF_LIFE_A_OFF + (r-1)*COLS + (c-1)];  // A[0:-2, 0:-2]
            uint32_t a1 = buf[GAME_OF_LIFE_A_OFF + (r-1)*COLS + c    ];  // A[0:-2, 1:-1]
            uint32_t a2 = buf[GAME_OF_LIFE_A_OFF + (r-1)*COLS + (c+1)];  // A[0:-2, 2:0]
            uint32_t a3 = buf[GAME_OF_LIFE_A_OFF + r*COLS + (c-1)    ];  // A[:-2, 1:-1]
            uint32_t a4 = buf[GAME_OF_LIFE_A_OFF + r*COLS + (c+1)    ];  // A[:-2, 1:-1]
            uint32_t a5 = buf[GAME_OF_LIFE_A_OFF + (r+1)*COLS + (c-1)];  // A[:-2, 1:-1]
            uint32_t a6 = buf[GAME_OF_LIFE_A_OFF + (r+1)*COLS + c    ];  // A[:-2, 1:-1]
            uint32_t a7 = buf[GAME_OF_LIFE_A_OFF + (r+1)*COLS + (c+1)];  // A[:-2, 1:-1]
            uint32_t expected = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7;
            uint32_t got = buf[GAME_OF_LIFE_B_OFF + r*COLS + c];
            if (got != expected)
                panic("B[%d][%d] = %d, expected %d + %d + %d = %d\n",
                      r, c, got, a0, a1, a2, expected);
        }
    }
    int cpu_time = timer_get_usec() - cpu_start;

    printk("GPU time: %d us\n", gpu_time);
    printk("CPU time: %d us\n", cpu_time);
    printk("Speedup: %dx\n", cpu_time / gpu_time);
    game_of_life_release();
    printk("SUCCESS\n");
}

void notmain(void)
{
    test_game_of_life();
}
