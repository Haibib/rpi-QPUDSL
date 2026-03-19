#include <stddef.h>
#include <string.h>
#include "GoL-qpu.h"
#include "mailbox.h"
#include "GoL.h"

struct GoLGPU {
    uint32_t buf[GOL_BUF_N];
    uint32_t code[sizeof(GoL) / sizeof(uint32_t)];
    uint32_t unif[8][GOL_NUM_UNIFS];
    uint32_t unif_ptr[8];
    uint32_t mail[2];
    uint32_t handle;
};

static volatile struct GoLGPU *_gpu = NULL;

uint32_t *GoL_init(void) {
    uint32_t handle, vc;
    volatile struct GoLGPU *ptr;

    if (qpu_enable(1))
        panic("GoL_init: failed to enable QPU");

    handle = mem_alloc(sizeof(struct GoLGPU), 4096, GPU_MEM_FLG);
    if (!handle) {
        qpu_enable(0);
        panic("GoL_init: failed to allocate GPU memory");
    }

    vc  = mem_lock(handle);
    ptr = (volatile struct GoLGPU *)(vc - GPU_BASE);
    if (!ptr) {
        mem_free(handle);
        mem_unlock(handle);
        qpu_enable(0);
        panic("GoL_init: failed to map GPU memory");
    }

    ptr->handle  = handle;
    ptr->mail[0] = vc + offsetof(struct GoLGPU, code);
    ptr->mail[1] = vc + offsetof(struct GoLGPU, unif);
    memcpy((void *)ptr->code, GoL, sizeof(ptr->code));

    _gpu = ptr;
    return (uint32_t *)_gpu->buf;
}

uint32_t GoL_kernel(void *A, void *B) {
    uint32_t vc_base = _gpu->mail[0] - offsetof(struct GoLGPU, code);

    for (uint32_t q = 0; q < 8; q++) {
        int u = 0;
        _gpu->unif[q][u++] = vc_base + ((uint8_t *)A - (uint8_t *)_gpu);
        _gpu->unif[q][u++] = vc_base + ((uint8_t *)A - (uint8_t *)_gpu);
        _gpu->unif[q][u++] = vc_base + ((uint8_t *)A - (uint8_t *)_gpu);
        _gpu->unif[q][u++] = vc_base + ((uint8_t *)A - (uint8_t *)_gpu);
        _gpu->unif[q][u++] = vc_base + ((uint8_t *)A - (uint8_t *)_gpu);
        _gpu->unif[q][u++] = vc_base + ((uint8_t *)A - (uint8_t *)_gpu);
        _gpu->unif[q][u++] = vc_base + ((uint8_t *)A - (uint8_t *)_gpu);
        _gpu->unif[q][u++] = vc_base + ((uint8_t *)A - (uint8_t *)_gpu);
        _gpu->unif[q][u++] = vc_base + ((uint8_t *)B - (uint8_t *)_gpu);
        _gpu->unif[q][u++] = 0;
        _gpu->unif[q][u++] = 0;
        _gpu->unif[q][u++] = 1;
        _gpu->unif[q][u++] = 0;
        _gpu->unif[q][u++] = 2;
        _gpu->unif[q][u++] = 0;
        _gpu->unif[q][u++] = 0;
        _gpu->unif[q][u++] = 1;
        _gpu->unif[q][u++] = 2;
        _gpu->unif[q][u++] = 1;
        _gpu->unif[q][u++] = 0;
        _gpu->unif[q][u++] = 2;
        _gpu->unif[q][u++] = 1;
        _gpu->unif[q][u++] = 2;
        _gpu->unif[q][u++] = 2;
        _gpu->unif[q][u++] = 2;
        _gpu->unif[q][u++] = 1;
        _gpu->unif[q][u++] = 1;
        _gpu->unif[q][u++] = 258;
        _gpu->unif[q][u++] = 258;
        _gpu->unif[q][u++] = 256;
        _gpu->unif[q][u++] = 256;
        _gpu->unif[q][u++] = q;
        _gpu->unif_ptr[q] = _gpu->mail[1] + q * GOL_NUM_UNIFS * sizeof(uint32_t);
    }

    return gpu_fft_base_exec_direct(_gpu->mail[0],
                                    (uint32_t *)_gpu->unif_ptr, 8);
}

void GoL_release(void) {
    if (!_gpu) return;
    uint32_t handle = _gpu->handle;
    mem_unlock(handle);
    mem_free(handle);
    qpu_enable(0);
    _gpu = NULL;
}
