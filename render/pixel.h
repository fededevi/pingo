#pragma once

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

//What format to use:
//#define UINT8
#define UINT8
//#define RGBA8888

//Formats definitions:
#ifdef UINT8
typedef uint8_t Pixel;
#define PIXELBLACK 0
#define PIXELWHITE 255
#endif

#ifdef RGB888
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Pixel;

#define PIXELBLACK (Pixel){0,0,0}
#define PIXELWHITE (Pixel){255,255,255}
#endif

#ifdef RGBA8888
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Pixel;

#define PIXELBLACK (Pixel){0,0,0,255}
#define PIXELWHITE (Pixel){255,255,255,255}
#endif

extern Pixel pixelRandom();

#ifdef __cplusplus
}
#endif
