#include "mulshader.h"
#include "rpi.h"
#include <stdint.h>

#define GPU_MEM_FLG 0xC // cached=0xC; direct=0x4
#define GPU_BASE 0x40000000

#ifndef N
#define N 2097152
#endif

#define MAX_QPUS 12
#define NUM_UNIFS 5

struct mulGPU
{
	uint32_t A[N];
	uint32_t B[N];
	uint32_t C[N];
	uint32_t code[sizeof(mulshader) / sizeof(uint32_t)];
	uint32_t unif[MAX_QPUS][NUM_UNIFS];
	uint32_t unif_ptr[MAX_QPUS];
	uint32_t mail[2];
	uint32_t handle;
	uint32_t num_qpus;
};

void vec_mul_init(volatile struct mulGPU **gpu, uint32_t n, uint32_t num_qpus);

uint32_t mul_gpu_execute(volatile struct mulGPU *gpu);

void vec_mul_release(volatile struct mulGPU *gpu);