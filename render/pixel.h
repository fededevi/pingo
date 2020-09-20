#pragma once

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

//What format to use:
#define RGBA8888 // [ UINT8 | RGBA8888 | RGB888 ]

//Formats definitions:
#ifdef UINT8
typedef struct Pixel {
    uint8_t g;
}Pixel;
#define PIXELBLACK (Pixel){0}
#define PIXELWHITE (Pixel){255}
#endif

#ifdef RGB888
typedef struct Pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Pixel;

#define PIXELBLACK (Pixel){0,0,0}
#define PIXELWHITE (Pixel){255,255,255}
#endif

#ifdef RGBA8888
typedef struct Pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Pixel;

#define PIXELBLACK (Pixel){0,0,0,255}
#define PIXELWHITE (Pixel){255,255,255,255}
#endif

//Interface
extern Pixel pixelRandom();
extern Pixel pixelFromUInt8( uint8_t);
extern uint8_t pixelToUInt8( Pixel *);
extern uint32_t pixelToRGBA( Pixel *);
extern Pixel pixelFromRGBA( uint8_t r, uint8_t g, uint8_t b, uint8_t a);
extern Pixel pixelMul( Pixel p, float f);

#ifdef __cplusplus
}
#endif
