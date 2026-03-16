#include <stddef.h>
#include <string.h>
#include "gen-add.h"
#include "mailbox.h"
#include "genshader.h"

static void gpu_prepare(volatile struct genGPU **gpu)
{
    uint32_t handle, vc;
    volatile struct genGPU *ptr;

    if (qpu_enable(1))
        panic("Failed to enable GPU");

    handle = mem_alloc(sizeof(struct genGPU), 4096, GPU_MEM_FLG);
    if (!handle) {
        qpu_enable(0);
        panic("Failed to allocate GPU memory");
    }

    vc  = mem_lock(handle);
    ptr = (volatile struct genGPU *)(vc - GPU_BASE);
    if (!ptr) {
        mem_free(handle);
        mem_unlock(handle);
        qpu_enable(0);
        panic("Failed to map GPU memory");
    }

    ptr->handle  = handle;
    ptr->mail[0] = vc + offsetof(struct genGPU, code);
    ptr->mail[1] = vc + offsetof(struct genGPU, unif);
    *gpu = ptr;
}

uint32_t gen_add_execute(volatile struct genGPU *gpu)
{
    return gpu_fft_base_exec_direct(
        gpu->mail[0],
        (uint32_t *)gpu->unif_ptr,
        gpu->num_qpus);
}

void vec_gen_add_release(volatile struct genGPU *gpu)
{
    uint32_t handle = gpu->handle;
    mem_unlock(handle);
    mem_free(handle);
    qpu_enable(0);
}

void vec_gen_add_init(volatile struct genGPU **gpu, uint32_t n, uint32_t num_qpus)
{
    if (n > N) panic("Requested length %d > capacity %d", n, N);
    if (num_qpus < 1)       num_qpus = 1;
    if (num_qpus > MAX_QPUS) num_qpus = MAX_QPUS;

    gpu_prepare(gpu);
    volatile struct genGPU *ptr = *gpu;
    memcpy((void *)ptr->code, genshader, sizeof ptr->code);

    uint32_t vc          = ptr->mail[0] - offsetof(struct genGPU, code);
    uint32_t gpu_A_base  = vc + offsetof(struct genGPU, A);
    uint32_t gpu_B_base  = vc + offsetof(struct genGPU, B);
    uint32_t gpu_C_base  = vc + offsetof(struct genGPU, C);
    uint32_t gpu_unif_base = ptr->mail[1];

    ptr->num_qpus = num_qpus;

    for (uint32_t q = 0; q < MAX_QPUS; ++q) {
        if (q >= num_qpus) {
            ptr->unif_ptr[q] = 0;
            continue;
        }
        ptr->unif[q][0] = gpu_A_base;   // A base ptr
        ptr->unif[q][1] = gpu_B_base;   // B base ptr
        ptr->unif[q][2] = gpu_C_base;   // output base ptr
        ptr->unif[q][3] = 0;            // A_slice_start 
        ptr->unif[q][4] = 0;            // B_slice_start
        ptr->unif[q][5] = 0;            // Z_slice_start
        ptr->unif[q][6] = n;            // dim_size 
        ptr->unif[q][7] = n;            // slice_size 
        ptr->unif[q][8] = q;            // qpu_num
        ptr->unif_ptr[q] = gpu_unif_base + q * NUM_UNIFS * sizeof(uint32_t);
    }
}
