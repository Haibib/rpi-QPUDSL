#pragma once
#include "slice-add.h"
#include "rpi.h"
#include <stdint.h>

#ifndef GPU_MEM_FLG
#define GPU_MEM_FLG 0xC
#endif
#ifndef GPU_BASE
#define GPU_BASE    0x40000000
#endif

#define SLICE_ADD_A_N    (258*258)
#define SLICE_ADD_B_N    (258*258)

#define SLICE_ADD_A_OFF  0
#define SLICE_ADD_B_OFF  (SLICE_ADD_A_N)
#define SLICE_ADD_BUF_N    (SLICE_ADD_A_N + SLICE_ADD_B_N)

#define SLICE_ADD_NUM_UNIFS 17
#define ROWS 258
#define COLS 258

uint32_t *slice_add_init(void);

uint32_t slice_add_kernel(void *A, void *B);

void slice_add_release(void);
