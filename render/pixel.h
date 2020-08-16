#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//RGB8
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Pixel;

#define PIXELBLACK (Pixel){0,0,0}
#define PIXELWHITE (Pixel){255,255,255}

#ifdef __cplusplus
}
#endif
