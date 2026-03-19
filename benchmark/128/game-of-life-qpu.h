#pragma once
#include "game-of-life.h"
#include "rpi.h"
#include <stdint.h>

#ifndef GPU_MEM_FLG
#define GPU_MEM_FLG 0xC
#endif
#ifndef GPU_BASE
#define GPU_BASE    0x40000000
#endif

#define GAME_OF_LIFE_A_N    (130*130)
#define GAME_OF_LIFE_B_N    (130*130)

#define GAME_OF_LIFE_A_OFF  0
#define GAME_OF_LIFE_B_OFF  (GAME_OF_LIFE_A_N)
#define GAME_OF_LIFE_BUF_N    (GAME_OF_LIFE_A_N + GAME_OF_LIFE_B_N)

#define GAME_OF_LIFE_NUM_UNIFS 32

uint32_t *game_of_life_init(void);

uint32_t game_of_life_kernel(void *A, void *B);

void game_of_life_release(void);
