#include <stddef.h>
#include <string.h>
#include "gen-add-qpu.h"
#include "mailbox.h"
#include "gen-add.h"

struct GenAddGPU {
    uint32_t buf[GEN_ADD_BUF_N];
    uint32_t code[sizeof(gen_add) / sizeof(uint32_t)];
    uint32_t unif[8][GEN_ADD_NUM_UNIFS];
    uint32_t unif_ptr[8];
    uint32_t mail[2];
    uint32_t handle;
};

static volatile struct GenAddGPU *_gpu = NULL;

uint32_t *gen_add_init(void) {
    uint32_t handle, vc;
    volatile struct GenAddGPU *ptr;

    if (qpu_enable(1))
        panic("gen_add_init: failed to enable QPU");

    handle = mem_alloc(sizeof(struct GenAddGPU), 4096, GPU_MEM_FLG);
    if (!handle) {
        qpu_enable(0);
        panic("gen_add_init: failed to allocate GPU memory");
    }

    vc  = mem_lock(handle);
    ptr = (volatile struct GenAddGPU *)(vc - GPU_BASE);
    if (!ptr) {
        mem_free(handle);
        mem_unlock(handle);
        qpu_enable(0);
        panic("gen_add_init: failed to map GPU memory");
    }

    ptr->handle  = handle;
    ptr->mail[0] = vc + offsetof(struct GenAddGPU, code);
    ptr->mail[1] = vc + offsetof(struct GenAddGPU, unif);
    memcpy((void *)ptr->code, gen_add, sizeof(ptr->code));

    _gpu = ptr;
    return (uint32_t *)_gpu->buf;
}

uint32_t gen_add_kernel(void *A, void *B, void *C) {
    uint32_t vc_base = _gpu->mail[0] - offsetof(struct GenAddGPU, code);

    for (uint32_t q = 0; q < 8; q++) {
        int u = 0;
        _gpu->unif[q][u++] = vc_base + ((uint8_t *)A - (uint8_t *)_gpu);
        _gpu->unif[q][u++] = vc_base + ((uint8_t *)B - (uint8_t *)_gpu);
        _gpu->unif[q][u++] = vc_base + ((uint8_t *)C - (uint8_t *)_gpu);
        _gpu->unif[q][u++] = 0;
        _gpu->unif[q][u++] = 0;
        _gpu->unif[q][u++] = 0;
        _gpu->unif[q][u++] = 1048576;
        _gpu->unif[q][u++] = 1048576;
        _gpu->unif[q][u++] = q;
        _gpu->unif_ptr[q] = _gpu->mail[1] + q * GEN_ADD_NUM_UNIFS * sizeof(uint32_t);
    }

    return gpu_fft_base_exec_direct(_gpu->mail[0],
                                    (uint32_t *)_gpu->unif_ptr, 8);
}

void gen_add_release(void) {
    if (!_gpu) return;
    uint32_t handle = _gpu->handle;
    mem_unlock(handle);
    mem_free(handle);
    qpu_enable(0);
    _gpu = NULL;
}
