#include <stddef.h>
#include <string.h>
#include "grayscale-qpu.h"
#include "mailbox.h"
#include "grayscale.h"

struct GrayscaleGPU {
    uint32_t buf[GRAYSCALE_BUF_N];
    uint32_t code[sizeof(grayscale) / sizeof(uint32_t)];
    uint32_t unif[8][GRAYSCALE_NUM_UNIFS];
    uint32_t unif_ptr[8];
    uint32_t mail[2];
    uint32_t handle;
};

static volatile struct GrayscaleGPU *_gpu = NULL;

uint32_t *grayscale_init(void) {
    uint32_t handle, vc;
    volatile struct GrayscaleGPU *ptr;

    if (qpu_enable(1))
        panic("grayscale_init: failed to enable QPU");

    handle = mem_alloc(sizeof(struct GrayscaleGPU), 4096, GPU_MEM_FLG);
    if (!handle) {
        qpu_enable(0);
        panic("grayscale_init: failed to allocate GPU memory");
    }

    vc  = mem_lock(handle);
    ptr = (volatile struct GrayscaleGPU *)(vc - GPU_BASE);
    if (!ptr) {
        mem_free(handle);
        mem_unlock(handle);
        qpu_enable(0);
        panic("grayscale_init: failed to map GPU memory");
    }

    ptr->handle  = handle;
    ptr->mail[0] = vc + offsetof(struct GrayscaleGPU, code);
    ptr->mail[1] = vc + offsetof(struct GrayscaleGPU, unif);
    memcpy((void *)ptr->code, grayscale, sizeof(ptr->code));

    _gpu = ptr;
    return (uint32_t *)_gpu->buf;
}

uint32_t grayscale_kernel(void *R, void *G, void *B, void *Grey, float alpha, float beta, float theta) {
    uint32_t vc_base = _gpu->mail[0] - offsetof(struct GrayscaleGPU, code);

    for (uint32_t q = 0; q < 8; q++) {
        int u = 0;
        _gpu->unif[q][u++] = vc_base + ((uint8_t *)R - (uint8_t *)_gpu);
        _gpu->unif[q][u++] = vc_base + ((uint8_t *)G - (uint8_t *)_gpu);
        _gpu->unif[q][u++] = vc_base + ((uint8_t *)B - (uint8_t *)_gpu);
        _gpu->unif[q][u++] = vc_base + ((uint8_t *)Grey - (uint8_t *)_gpu);
        _gpu->unif[q][u++] = 0;
        _gpu->unif[q][u++] = 0;
        _gpu->unif[q][u++] = 0;
        _gpu->unif[q][u++] = 0;
        _gpu->unif[q][u++] = 0;
        _gpu->unif[q][u++] = 0;
        _gpu->unif[q][u++] = 0;
        _gpu->unif[q][u++] = 0;
        _gpu->unif[q][u++] = 576;
        _gpu->unif[q][u++] = 446;
        _gpu->unif[q][u++] = 576;
        _gpu->unif[q][u++] = 446;
        _gpu->unif[q][u++] = *(const uint32_t *)&alpha;
        _gpu->unif[q][u++] = *(const uint32_t *)&beta;
        _gpu->unif[q][u++] = *(const uint32_t *)&theta;
        _gpu->unif[q][u++] = q;
        _gpu->unif_ptr[q] = _gpu->mail[1] + q * GRAYSCALE_NUM_UNIFS * sizeof(uint32_t);
    }

    return gpu_fft_base_exec_direct(_gpu->mail[0],
                                    (uint32_t *)_gpu->unif_ptr, 8);
}

void grayscale_release(void) {
    if (!_gpu) return;
    uint32_t handle = _gpu->handle;
    mem_unlock(handle);
    mem_free(handle);
    qpu_enable(0);
    _gpu = NULL;
}
