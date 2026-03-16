#include <stddef.h>
#include <string.h>
#include "gen-mat-add.h"
#include "mailbox.h"
#include "mat_add_shader.h"

static void gpu_prepare(volatile struct matGPU **gpu)
{
    uint32_t handle, vc;
    volatile struct matGPU *ptr;

    if (qpu_enable(1))
        panic("Failed to enable GPU");

    handle = mem_alloc(sizeof(struct matGPU), 4096, GPU_MEM_FLG);
    if (!handle) {
        qpu_enable(0);
        panic("Failed to allocate GPU memory");
    }

    vc  = mem_lock(handle);
    ptr = (volatile struct matGPU *)(vc - GPU_BASE);
    if (!ptr) {
        mem_free(handle);
        mem_unlock(handle);
        qpu_enable(0);
        panic("Failed to map GPU memory");
    }

    ptr->handle  = handle;
    ptr->mail[0] = vc + offsetof(struct matGPU, code);
    ptr->mail[1] = vc + offsetof(struct matGPU, unif);
    *gpu = ptr;
}

uint32_t mat_add_execute(volatile struct matGPU *gpu)
{
    return gpu_fft_base_exec_direct(
        gpu->mail[0],
        (uint32_t *)gpu->unif_ptr,
        gpu->num_qpus);
}

void vec_mat_add_release(volatile struct matGPU *gpu)
{
    uint32_t handle = gpu->handle;
    mem_unlock(handle);
    mem_free(handle);
    qpu_enable(0);
}

void vec_mat_add_init(volatile struct matGPU **gpu,
                      uint32_t rows, uint32_t cols, uint32_t num_qpus)
{
    if (rows * cols > N)
        panic("Matrix %dx%d exceeds capacity %d", rows, cols, N);
    if (num_qpus < 1)        num_qpus = 1;
    if (num_qpus > MAX_QPUS) num_qpus = MAX_QPUS;

    gpu_prepare(gpu);
    volatile struct matGPU *ptr = *gpu;
    memcpy((void *)ptr->code, mat_add_shader, sizeof ptr->code);

    uint32_t vc          = ptr->mail[0] - offsetof(struct matGPU, code);
    uint32_t gpu_A_base  = vc + offsetof(struct matGPU, A);
    uint32_t gpu_B_base  = vc + offsetof(struct matGPU, B);
    uint32_t gpu_C_base  = vc + offsetof(struct matGPU, C);
    uint32_t gpu_unif_base = ptr->mail[1];

    ptr->num_qpus = num_qpus;

    for (uint32_t q = 0; q < MAX_QPUS; ++q) {
        if (q >= num_qpus) {
            ptr->unif_ptr[q] = 0;
            continue;
        }
        ptr->unif[q][0]  = gpu_A_base;   // A ptr
        ptr->unif[q][1]  = gpu_B_base;   // B ptr
        ptr->unif[q][2]  = gpu_C_base;   // output ptr
        ptr->unif[q][3]  = 0;            // A.slice_start[0] 
        ptr->unif[q][4]  = 0;            // A.slice_start[1] 
        ptr->unif[q][5]  = 0;            // B.slice_start[0]
        ptr->unif[q][6]  = 0;            // B.slice_start[1]
        ptr->unif[q][7]  = 0;            // output.slice_start[0]
        ptr->unif[q][8]  = 0;            // output.slice_start[1]
        ptr->unif[q][9]  = cols;         // dim_size[0] 
        ptr->unif[q][10] = rows;         // dim_size[1] 
        ptr->unif[q][11] = cols;         // slice_size[0] 
        ptr->unif[q][12] = rows;         // slice_size[1] 
        ptr->unif[q][13] = q;            // qpu_num
        ptr->unif_ptr[q] = gpu_unif_base + q * NUM_UNIFS * sizeof(uint32_t);
    }
}
