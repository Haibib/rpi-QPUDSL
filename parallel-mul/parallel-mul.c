#include <stddef.h>
#include <string.h>
#include "parallel-mul.h"
#include "mailbox.h"
#include "mulshader.h"

void mul_gpu_prepare(
	volatile struct mulGPU **gpu)
{
	uint32_t handle, vc;
	volatile struct mulGPU *ptr;

	/* TURN ON THE QPU */
	if (qpu_enable(1))
		panic("Failed to enable GPU");

	/* ALLOCATE MEMORY FOR THE STRUCT QPU */
	handle = mem_alloc(sizeof(struct mulGPU), 4096, GPU_MEM_FLG);
	if (!handle)
	{
		qpu_enable(0);
		panic("Failed to allocate GPU memory");
	}

	/* CLAIM THE BUS ADDRESS OF THE MEMORY */
	vc = mem_lock(handle);

	/* USE THE Pi 0 GPU OFFSET TO GET AN ADDRESS WE CAN READ/WRITE ON THE CPU */
	ptr = (volatile struct mulGPU *)(vc - GPU_BASE);
	if (ptr == NULL)
	{
		mem_free(handle);
		mem_unlock(handle);
		qpu_enable(0);
		panic("Failed to convert handle to GPU bus address");
	}

	/* INITIALIZE STRUCT QPU FIELDS*/
	ptr->handle = handle;
	ptr->mail[0] = vc + offsetof(struct mulGPU, code);
	ptr->mail[1] = vc + offsetof(struct mulGPU, unif);

	*gpu = ptr;
	return;
}

/* SEND THE CODE AND UNIFS TO THE GPU (see docs p. 89-91)*/
uint32_t mul_gpu_execute(volatile struct mulGPU *gpu)
{
	// printk("num_qpus: %d\n", gpu->num_qpus);
	// for (uint32_t q = 0; q < gpu->num_qpus; ++q) {
	// 	printk("q = %d, A addr: %x\n", q, gpu->unif[q][0]);
	// 	printk("q = %d, B addr: %x\n", q, gpu->unif[q][1]);
	// 	printk("q = %d, C addr: %x\n", q, gpu->unif[q][2]);
	// 	printk("q = %d, slice: %x\n", q, gpu->unif[q][3]);
	// 	printk("q = %d, ptr: %x\n", q, gpu->unif_ptr[q]);
	// }
	return gpu_fft_base_exec_direct(
		gpu->mail[0],
		(uint32_t *)gpu->unif_ptr,
		gpu->num_qpus);
}

/* RELEASE MEMORY AND TURN OFF QPU */
void vec_mul_release(volatile struct mulGPU *gpu)
{
	uint32_t handle = gpu->handle;
	mem_unlock(handle);
	mem_free(handle);
	qpu_enable(0);
}

void vec_mul_init(volatile struct mulGPU **gpu, uint32_t n, uint32_t num_qpus)
{
	if (n < 0) panic("n must be non-negative");
    if (n > N) panic("Requested length %d > capacity %d", n, N);
    if (num_qpus <= 0) num_qpus = 1;
    if (num_qpus > MAX_QPUS) num_qpus = MAX_QPUS;
	
	mul_gpu_prepare(gpu);
	volatile struct mulGPU *ptr = *gpu;
	memcpy((void *)ptr->code, mulshader, sizeof ptr->code);
	uint32_t num_vec32 = (n + 31) / 32;
	// If fewer vec entries than QPUs
	if (num_vec32 < num_qpus) {
		num_qpus = num_vec32;
	}
	ptr->num_qpus = num_qpus;

	uint32_t base = num_vec32 / num_qpus;
    uint32_t rem = num_vec32 % num_qpus;
    uint32_t offset = 0;

	uint32_t vc = ptr->mail[0] - offsetof(struct mulGPU, code);
	uint32_t gpu_A_base   = vc + offsetof(struct mulGPU, A);
    uint32_t gpu_B_base   = vc + offsetof(struct mulGPU, B);
    uint32_t gpu_C_base   = vc + offsetof(struct mulGPU, C);
	uint32_t gpu_unif_base = ptr->mail[1];

	for (uint32_t q = 0; q < MAX_QPUS; ++q) {
		// Fill unused slots with safe value
		if (q >= num_qpus) {
			ptr->unif_ptr[q] = 0;
            continue;
		}
		uint32_t slice = base + (q < rem ? 1 : 0);
		uint32_t padded_len = slice * 32;

		if (offset + padded_len > N) {
			panic("Not enough capacity %d to pad slices", N);
		}
		ptr->unif[q][0] = gpu_A_base + offset * sizeof(uint32_t);
        ptr->unif[q][1] = gpu_B_base + offset * sizeof(uint32_t);
        ptr->unif[q][2] = gpu_C_base + offset * sizeof(uint32_t);
		ptr->unif[q][3] = slice * 2;
		ptr->unif[q][4] = q;
		ptr->unif_ptr[q] = gpu_unif_base + (q * NUM_UNIFS * sizeof(uint32_t));
		offset += padded_len;
	}

	for (uint32_t i = n; i < offset; ++i) {
		ptr->A[i] = 0;
		ptr->B[i] = 0;
		ptr->C[i] = 0;
	}
}
