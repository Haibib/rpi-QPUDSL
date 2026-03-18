#pragma once
#include "mat-add.h"
#include "rpi.h"
#include <stdint.h>

#ifndef GPU_MEM_FLG
#define GPU_MEM_FLG 0xC
#endif
#ifndef GPU_BASE
#define GPU_BASE    0x40000000
#endif

#define MAT_ADD_A_N    (1024*4096)
#define MAT_ADD_B_N    (1024*4096)
#define MAT_ADD_C_N      (1024*4096)

#define MAT_ADD_A_OFF  0
#define MAT_ADD_B_OFF  (MAT_ADD_A_N)
#define MAT_ADD_C_OFF  (MAT_ADD_A_N + MAT_ADD_B_N)
#define MAT_ADD_BUF_N    (MAT_ADD_A_N + MAT_ADD_B_N + MAT_ADD_C_N)

#define MAT_ADD_NUM_UNIFS 14

uint32_t *mat_add_init(void);

uint32_t mat_add_kernel(void *A, void *B, void *C);

void mat_add_release(void);
