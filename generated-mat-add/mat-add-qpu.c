#include <stddef.h>
#include <string.h>
#include "mat-add-qpu.h"
#include "mailbox.h"
#include "mat-add.h"

struct MatAddGPU {
    uint32_t buf[MAT_ADD_BUF_N];
    uint32_t code[sizeof(mat_add) / sizeof(uint32_t)];
    uint32_t unif[8][MAT_ADD_NUM_UNIFS];
    uint32_t unif_ptr[8];
    uint32_t mail[2];
    uint32_t handle;
};

static volatile struct MatAddGPU *_gpu = NULL;

uint32_t *mat_add_init(void) {
    uint32_t handle, vc;
    volatile struct MatAddGPU *ptr;

    if (qpu_enable(1))
        panic("mat_add_init: failed to enable QPU");

    handle = mem_alloc(sizeof(struct MatAddGPU), 4096, GPU_MEM_FLG);
    if (!handle) {
        qpu_enable(0);
        panic("mat_add_init: failed to allocate GPU memory");
    }

    vc  = mem_lock(handle);
    ptr = (volatile struct MatAddGPU *)(vc - GPU_BASE);
    if (!ptr) {
        mem_free(handle);
        mem_unlock(handle);
        qpu_enable(0);
        panic("mat_add_init: failed to map GPU memory");
    }

    ptr->handle  = handle;
    ptr->mail[0] = vc + offsetof(struct MatAddGPU, code);
    ptr->mail[1] = vc + offsetof(struct MatAddGPU, unif);
    memcpy((void *)ptr->code, mat_add, sizeof(ptr->code));

    _gpu = ptr;
    return (uint32_t *)_gpu->buf;
}

uint32_t mat_add_kernel(void *A, void *B, void *C) {
    uint32_t vc_base = _gpu->mail[0] - offsetof(struct MatAddGPU, code);

    for (uint32_t q = 0; q < 8; q++) {
        int u = 0;
        _gpu->unif[q][u++] = vc_base + ((uint8_t *)A - (uint8_t *)_gpu);
        _gpu->unif[q][u++] = vc_base + ((uint8_t *)B - (uint8_t *)_gpu);
        _gpu->unif[q][u++] = vc_base + ((uint8_t *)C - (uint8_t *)_gpu);
        _gpu->unif[q][u++] = 0;
        _gpu->unif[q][u++] = 0;
        _gpu->unif[q][u++] = 0;
        _gpu->unif[q][u++] = 0;
        _gpu->unif[q][u++] = 0;
        _gpu->unif[q][u++] = 0;
        _gpu->unif[q][u++] = 4096;
        _gpu->unif[q][u++] = 1024;
        _gpu->unif[q][u++] = 4096;
        _gpu->unif[q][u++] = 1024;
        _gpu->unif[q][u++] = q;
        _gpu->unif_ptr[q] = _gpu->mail[1] + q * MAT_ADD_NUM_UNIFS * sizeof(uint32_t);
    }

    return gpu_fft_base_exec_direct(_gpu->mail[0],
                                    (uint32_t *)_gpu->unif_ptr, 8);
}

void mat_add_release(void) {
    if (!_gpu) return;
    uint32_t handle = _gpu->handle;
    mem_unlock(handle);
    mem_free(handle);
    qpu_enable(0);
    _gpu = NULL;
}
