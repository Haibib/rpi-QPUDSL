#include <stddef.h>
#include <string.h>
#include "game-of-life-qpu.h"
#include "mailbox.h"
#include "game-of-life.h"

struct GameOfLifeGPU {
    uint32_t buf[GAME_OF_LIFE_BUF_N];
    uint32_t code[sizeof(game_of_life) / sizeof(uint32_t)];
    uint32_t unif[8][GAME_OF_LIFE_NUM_UNIFS];
    uint32_t unif_ptr[8];
    uint32_t mail[2];
    uint32_t handle;
};

static volatile struct GameOfLifeGPU *_gpu = NULL;

uint32_t *game_of_life_init(void) {
    uint32_t handle, vc;
    volatile struct GameOfLifeGPU *ptr;

    if (qpu_enable(1))
        panic("game_of_life_init: failed to enable QPU");

    handle = mem_alloc(sizeof(struct GameOfLifeGPU), 4096, GPU_MEM_FLG);
    if (!handle) {
        qpu_enable(0);
        panic("game_of_life_init: failed to allocate GPU memory");
    }

    vc  = mem_lock(handle);
    ptr = (volatile struct GameOfLifeGPU *)(vc - GPU_BASE);
    if (!ptr) {
        mem_free(handle);
        mem_unlock(handle);
        qpu_enable(0);
        panic("game_of_life_init: failed to map GPU memory");
    }

    ptr->handle  = handle;
    ptr->mail[0] = vc + offsetof(struct GameOfLifeGPU, code);
    ptr->mail[1] = vc + offsetof(struct GameOfLifeGPU, unif);
    memcpy((void *)ptr->code, game_of_life, sizeof(ptr->code));

    _gpu = ptr;
    return (uint32_t *)_gpu->buf;
}

uint32_t game_of_life_kernel(void *A, void *B) {
    uint32_t vc_base = _gpu->mail[0] - offsetof(struct GameOfLifeGPU, code);

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
        _gpu->unif[q][u++] = 1026;
        _gpu->unif[q][u++] = 1026;
        _gpu->unif[q][u++] = 1024;
        _gpu->unif[q][u++] = 1024;
        _gpu->unif[q][u++] = q;
        _gpu->unif_ptr[q] = _gpu->mail[1] + q * GAME_OF_LIFE_NUM_UNIFS * sizeof(uint32_t);
    }

    return gpu_fft_base_exec_direct(_gpu->mail[0],
                                    (uint32_t *)_gpu->unif_ptr, 8);
}

void game_of_life_release(void) {
    if (!_gpu) return;
    uint32_t handle = _gpu->handle;
    mem_unlock(handle);
    mem_free(handle);
    qpu_enable(0);
    _gpu = NULL;
}
