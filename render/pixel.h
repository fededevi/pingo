#pragma once

#include "fwd.h"

#include <stdint.h>
#include <stdlib.h>

// Define one of the available formats
// #define PINGO_PIXEL_UINT8
// #define PINGO_PIXEL_RGB565
// #define PINGO_PIXEL_RGBA8888
#define PINGO_PIXEL_BGRA8888
// #define PINGO_PIXEL_RGB888

// Formats definitions:
#ifdef PINGO_PIXEL_UINT8
struct Pixel {
  uint8_t g;
};
#define PIXELBLACK                                                             \
  (Pixel) { 0 }
#define PIXELWHITE                                                             \
  (Pixel) { 255 }
#endif

#ifdef PINGO_PIXEL_RGB565
struct Pixel {
  uint8_t red : 5;
  uint8_t green : 6;
  uint8_t blue : 5;
};
#define PIXELBLACK                                                             \
  (Pixel) { 0 }
#define PIXELWHITE                                                             \
  (Pixel) { 255 }
#endif

#ifdef PINGO_PIXEL_RGB888
struct Pixel {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

#define PIXELBLACK                                                             \
  (Pixel) { 0, 0, 0 }
#define PIXELWHITE                                                             \
  (Pixel) { 255, 255, 255 }
#endif

#ifdef PINGO_PIXEL_RGBA8888
struct Pixel {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
};

#define PIXELBLACK                                                             \
  (Pixel) { 0, 0, 0, 255 }
#define PIXELWHITE                                                             \
  (Pixel) { 255, 255, 255, 255 }
#endif

#ifdef PINGO_PIXEL_BGRA8888
struct Pixel {
  uint8_t b;
  uint8_t g;
  uint8_t r;
  uint8_t a;
};

#define PIXELBLACK                                                             \
  (Pixel) { 0, 0, 0, 255 }
#define PIXELWHITE                                                             \
  (Pixel) { 255, 255, 255, 255 }
#endif

// Interface
extern Pixel pixelRandom();
extern uint8_t pixelToUInt8(Pixel *);
extern Pixel pixelFromRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

// Inline: the rasterizer calls both of these once per pixel it draws, and out
// of line that is a call the optimizer cannot see through unless link-time
// optimization is available and the build is static. A profile put pixelMul at
// 19% of the run over 195 million calls.
#ifdef PINGO_PIXEL_UINT8
static inline Pixel pixelFromUInt8(uint8_t g) { return (Pixel){g}; }
static inline Pixel pixelMul(Pixel p, float f) { return (Pixel){p.g * f}; }
#endif

#ifdef PINGO_PIXEL_RGB888
static inline Pixel pixelFromUInt8(uint8_t g) { return (Pixel){g, g, g}; }
static inline Pixel pixelMul(Pixel p, float f) {
  return (Pixel){p.r * f, p.g * f, p.b * f};
}
#endif

#ifdef PINGO_PIXEL_RGBA8888
static inline Pixel pixelFromUInt8(uint8_t g) { return (Pixel){g, g, g, 255}; }
static inline Pixel pixelMul(Pixel p, float f) {
  return (Pixel){p.r * f, p.g * f, p.b * f, p.a};
}
#endif

#ifdef PINGO_PIXEL_BGRA8888
static inline Pixel pixelFromUInt8(uint8_t g) { return (Pixel){g, g, g, 255}; }
static inline Pixel pixelMul(Pixel p, float f) {
  return (Pixel){p.b * f, p.g * f, p.r * f, p.a};
}
#endif

#ifdef PINGO_PIXEL_RGB565
static inline Pixel pixelFromUInt8(uint8_t g) { return (Pixel){g, g, g}; }
static inline Pixel pixelMul(Pixel p, float f) {
  return (Pixel){p.red * f, p.green * f, p.blue * f};
}
#endif
