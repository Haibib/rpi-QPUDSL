#pragma once
#include "gen-add.h"
#include "rpi.h"
#include <stdint.h>

#ifndef GPU_MEM_FLG
#define GPU_MEM_FLG 0xC
#endif
#ifndef GPU_BASE
#define GPU_BASE    0x40000000
#endif

#define GEN_ADD_A_N    1048576
#define GEN_ADD_B_N    1048576
#define GEN_ADD_C_N      1048576

#define GEN_ADD_A_OFF  0
#define GEN_ADD_B_OFF  (GEN_ADD_A_N)
#define GEN_ADD_C_OFF  (GEN_ADD_A_N + GEN_ADD_B_N)
#define GEN_ADD_BUF_N    (GEN_ADD_A_N + GEN_ADD_B_N + GEN_ADD_C_N)

#define GEN_ADD_NUM_UNIFS 9

uint32_t *gen_add_init(void);

uint32_t gen_add_kernel(void *A, void *B, void *C);

void gen_add_release(void);
