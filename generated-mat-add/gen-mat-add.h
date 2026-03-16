#pragma once
#include "mat_add_shader.h"
#include "rpi.h"
#include <stdint.h>

#define GPU_MEM_FLG 0xC
#define GPU_BASE    0x40000000

#ifndef ROWS
#define ROWS 9
#endif
#ifndef COLS
#define COLS 6400
#endif
#define N (ROWS * COLS)

#define MAX_QPUS  8

// Uniform layout
//   [0]  A_ptr              GPU base address of A 
//   [1]  B_ptr              GPU base address of B
//   [2]  output_ptr         GPU base address of output
//   [3]  A.slice_start[0]   column offset
//   [4]  A.slice_start[1]   row offset
//   [5]  B.slice_start[0]
//   [6]  B.slice_start[1]
//   [7]  output.slice_start[0]
//   [8]  output.slice_start[1]
//   [9]  dim_size[0]         number of columns
//   [10] dim_size[1]         number of rows
//   [11] slice_size[0]       columns to process
//   [12] slice_size[1]       rows to process
//   [13] qpu_num           
#define NUM_UNIFS 14

struct matGPU {
    uint32_t A[N];
    uint32_t B[N];
    uint32_t C[N];
    uint32_t code[sizeof(mat_add_shader) / sizeof(uint32_t)];
    uint32_t unif[MAX_QPUS][NUM_UNIFS];
    uint32_t unif_ptr[MAX_QPUS];
    uint32_t mail[2];
    uint32_t handle;
    uint32_t num_qpus;
};

void vec_mat_add_init(volatile struct matGPU **gpu,
                      uint32_t rows, uint32_t cols, uint32_t num_qpus);
uint32_t mat_add_execute(volatile struct matGPU *gpu);
void vec_mat_add_release(volatile struct matGPU *gpu);
