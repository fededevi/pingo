#include "pixel.h"

#ifdef UINT8

extern Pixel pixelRandom() {
    return (Pixel){(uint8_t)rand()};
}

uint8_t pixelToUInt8(Pixel * p)
{
    return p->g;
}


extern Pixel pixelFromUInt8( uint8_t g){
    return (Pixel){g};
}

uint32_t pixelToRGBA(Pixel * p)
{
    uint8_t g = p->g;
    uint32_t a = g | g<<8 | g<<16;
    return a;
}

#endif

#ifdef RGB888
extern Pixel pixelRandom() {
    return (Pixel){(uint8_t)rand(),(uint8_t)rand(),(uint8_t)rand()};
}
#endif

#ifdef RGBA8888
extern Pixel pixelRandom() {
    return (Pixel){(uint8_t)rand(),(uint8_t)rand(),(uint8_t)rand(),255};
}
#endif





