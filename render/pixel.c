#include "pixel.h"

#ifdef PINGO_PIXEL_UINT8

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

extern Pixel pixelMul(Pixel p, float f)
{
    return (Pixel){p.g*f};
}

extern Pixel pixelFromRGBA( uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return (Pixel){((r + g + b) / 3)};
}
#endif

#ifdef PINGO_PIXEL_RGB888
extern Pixel pixelRandom() {
    return (Pixel){(uint8_t)rand(),(uint8_t)rand(),(uint8_t)rand()};
}

uint32_t pixelToRGBA(Pixel * p)
{
    uint8_t g = p->g;
    uint32_t a = p->r | p->g <<8 | p->b<<16| 255<<24;
    return a;
}

extern Pixel pixelMul(Pixel p, float f)
{
    return (Pixel){p.r*f,p.g*f,p.b*f};
}

extern Pixel pixelFromUInt8( uint8_t g){
    return (Pixel){g,g,g};
}

extern uint8_t pixelToUInt8( Pixel * p){
    return (p->r + p->g + p->b) / 3;
}

extern Pixel pixelFromRGBA( uint8_t r, uint8_t g, uint8_t b, uint8_t a){
    return (Pixel){r,g,b};
}
#endif

#ifdef PINGO_PIXEL_RGBA8888
extern Pixel pixelRandom() {
    return (Pixel){(uint8_t)rand(),(uint8_t)rand(),(uint8_t)rand(),255};
}

extern Pixel pixelFromUInt8( uint8_t g){
    return (Pixel){g,g,g, 255};
}
extern uint8_t pixelToUInt8( Pixel * p){
    return (p->r + p->g + p->b) / 3;
}

extern Pixel pixelFromRGBA( uint8_t r, uint8_t g, uint8_t b, uint8_t a){
    return (Pixel){r,g,b,a};
}

extern Pixel pixelMul(Pixel p, float f)
{
    return (Pixel){p.r*f,p.g*f,p.b*f,p.a};
}

#endif


#ifdef PINGO_PIXEL_BGRA8888
extern Pixel pixelRandom() {
    return (Pixel){(uint8_t)rand(),(uint8_t)rand(),(uint8_t)rand(),255};
}

extern Pixel pixelFromUInt8( uint8_t g){
    return (Pixel){g,g,g, 255};
}

extern uint8_t pixelToUInt8( Pixel * p){
    return (p->r + p->g + p->b) / 3;
}

extern Pixel pixelFromRGBA( uint8_t r, uint8_t g, uint8_t b, uint8_t a){
    return (Pixel){b,g,r,a};
}

extern Pixel pixelMul(Pixel p, float f)
{
    return (Pixel){p.b*f,p.g*f,p.r*f,p.a};
}

#endif
