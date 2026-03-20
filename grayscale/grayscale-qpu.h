#pragma once
#include "grayscale.h"
#include "rpi.h"
#include <stdint.h>

#ifndef GPU_MEM_FLG
#define GPU_MEM_FLG 0xC
#endif
#ifndef GPU_BASE
#define GPU_BASE    0x40000000
#endif

#define GRAYSCALE_R_N    (446*576)
#define GRAYSCALE_G_N    (446*576)
#define GRAYSCALE_B_N    (446*576)
#define GRAYSCALE_GREY_N    (446*576)

#define GRAYSCALE_R_OFF  0
#define GRAYSCALE_G_OFF  (GRAYSCALE_R_N)
#define GRAYSCALE_B_OFF  (GRAYSCALE_R_N + GRAYSCALE_G_N)
#define GRAYSCALE_GREY_OFF  (GRAYSCALE_R_N + GRAYSCALE_G_N + GRAYSCALE_B_N)
#define GRAYSCALE_BUF_N    (GRAYSCALE_R_N + GRAYSCALE_G_N + GRAYSCALE_B_N + GRAYSCALE_GREY_N)

#define GRAYSCALE_NUM_UNIFS 20

uint32_t *grayscale_init(void);

uint32_t grayscale_kernel(void *R, void *G, void *B, void *Grey, float alpha, float beta, float theta);

void grayscale_release(void);
