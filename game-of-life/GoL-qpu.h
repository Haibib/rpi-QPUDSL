#pragma once
#include "GoL.h"
#include "rpi.h"
#include <stdint.h>

#ifndef GPU_MEM_FLG
#define GPU_MEM_FLG 0xC
#endif
#ifndef GPU_BASE
#define GPU_BASE    0x40000000
#endif

#define GOL_A_N    (258*258)
#define GOL_B_N    (258*258)

#define GOL_A_OFF  0
#define GOL_B_OFF  (GOL_A_N)
#define GOL_BUF_N    (GOL_A_N + GOL_B_N)

#define GOL_NUM_UNIFS 32

uint32_t *GoL_init(void);

uint32_t GoL_kernel(void *A, void *B);

void GoL_release(void);
