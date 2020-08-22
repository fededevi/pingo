#include "pixel.h"

#ifdef UINT8
extern Pixel pixelRandom() {
    return (Pixel){(uint8_t)rand()};
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


