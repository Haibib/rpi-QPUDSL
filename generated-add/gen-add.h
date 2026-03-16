#pragma once
#include "genshader.h"
#include "rpi.h"
#include <stdint.h>

#define GPU_MEM_FLG 0xC  
#define GPU_BASE    0x40000000

#ifndef N
#define N (1024 * 1024)
#endif

#define MAX_QPUS  8
// Uniform layout:
//   [0] A_ptr          base GPU address of A
//   [1] B_ptr          base GPU address of B
//   [2] Z_ptr          base GPU address of output Z
//   [3] A_slice_start  element offset where A's slice begins 
//   [4] B_slice_start  element offset where B's slice begins
//   [5] Z_slice_start  element offset where Z's slice begins
//   [6] dim_size       full array length 
//   [7] slice_size     total elements to process across all QPUs 
//   [8] qpu_num      
#define NUM_UNIFS 9

struct genGPU {
    uint32_t A[N];
    uint32_t B[N];
    uint32_t C[N];
    uint32_t code[sizeof(genshader) / sizeof(uint32_t)];
    uint32_t unif[MAX_QPUS][NUM_UNIFS];
    uint32_t unif_ptr[MAX_QPUS];
    uint32_t mail[2];
    uint32_t handle;
    uint32_t num_qpus;
};

void vec_gen_add_init(volatile struct genGPU **gpu, uint32_t n, uint32_t num_qpus);
uint32_t gen_add_execute(volatile struct genGPU *gpu);
void vec_gen_add_release(volatile struct genGPU *gpu);
