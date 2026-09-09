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
#define PIXEL_BLACK                                                             \
  (Pixel) { 0 }
#define PIXEL_WHITE                                                             \
  (Pixel) { 255 }
#endif

#ifdef PINGO_PIXEL_RGB565
struct Pixel {
  uint8_t red : 5;
  uint8_t green : 6;
  uint8_t blue : 5;
};
#define PIXEL_BLACK                                                             \
  (Pixel) { 0 }
#define PIXEL_WHITE                                                             \
  (Pixel) { 255 }
#endif

#ifdef PINGO_PIXEL_RGB888
struct Pixel {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

#define PIXEL_BLACK                                                             \
  (Pixel) { 0, 0, 0 }
#define PIXEL_WHITE                                                             \
  (Pixel) { 255, 255, 255 }
#endif

#ifdef PINGO_PIXEL_RGBA8888
struct Pixel {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
};

#define PIXEL_BLACK                                                             \
  (Pixel) { 0, 0, 0, 255 }
#define PIXEL_WHITE                                                             \
  (Pixel) { 255, 255, 255, 255 }
#endif

#ifdef PINGO_PIXEL_BGRA8888
struct Pixel {
  uint8_t b;
  uint8_t g;
  uint8_t r;
  uint8_t a;
};

#define PIXEL_BLACK                                                             \
  (Pixel) { 0, 0, 0, 255 }
#define PIXEL_WHITE                                                             \
  (Pixel) { 255, 255, 255, 255 }
#endif

// Interface
extern Pixel pixel_random();
extern uint8_t pixel_to_uint8(Pixel *);
extern Pixel pixel_from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

// Inline: the rasterizer calls both of these once per pixel it draws, and out
// of line that is a call the optimizer cannot see through unless link-time
// optimization is available and the build is static. A profile put pixel_mul at
// 19% of the run over 195 million calls.

// pixel_mul costs three int-to-float conversions, three multiplies and three
// float-to-int conversions for every pixel drawn - a profile put it at 23% of
// the run. The shade factor is fixed for a whole triangle, so for a triangle
// covering appreciably more pixels than the table has entries it is cheaper
// to tabulate it. Each entry uses the same expression pixel_mul uses, so the
// result is identical by construction and not merely close.
typedef struct PixelShadeTable {
  uint8_t v[256];
} PixelShadeTable;

static inline void pixel_shade_table_init(PixelShadeTable *t, float f) {
  for (int i = 0; i < 256; i++) {
    t->v[i] = (uint8_t)(i * f);
  }
}

#ifdef PINGO_PIXEL_UINT8
static inline Pixel pixel_from_uint8(uint8_t g) { return (Pixel){g}; }
static inline Pixel pixel_mul(Pixel p, float f) { return (Pixel){p.g * f}; }
static inline Pixel pixel_mul_table(Pixel p, const PixelShadeTable *t) {
  return (Pixel){t->v[p.g]};
}
#endif

#ifdef PINGO_PIXEL_RGB888
static inline Pixel pixel_from_uint8(uint8_t g) { return (Pixel){g, g, g}; }
static inline Pixel pixel_mul(Pixel p, float f) {
  return (Pixel){p.r * f, p.g * f, p.b * f};
}
static inline Pixel pixel_mul_table(Pixel p, const PixelShadeTable *t) {
  return (Pixel){t->v[p.r], t->v[p.g], t->v[p.b]};
}
#endif

#ifdef PINGO_PIXEL_RGBA8888
static inline Pixel pixel_from_uint8(uint8_t g) { return (Pixel){g, g, g, 255}; }
static inline Pixel pixel_mul(Pixel p, float f) {
  return (Pixel){p.r * f, p.g * f, p.b * f, p.a};
}
static inline Pixel pixel_mul_table(Pixel p, const PixelShadeTable *t) {
  return (Pixel){t->v[p.r], t->v[p.g], t->v[p.b], p.a};
}
#endif

#ifdef PINGO_PIXEL_BGRA8888
static inline Pixel pixel_from_uint8(uint8_t g) { return (Pixel){g, g, g, 255}; }
static inline Pixel pixel_mul(Pixel p, float f) {
  return (Pixel){p.b * f, p.g * f, p.r * f, p.a};
}
static inline Pixel pixel_mul_table(Pixel p, const PixelShadeTable *t) {
  return (Pixel){t->v[p.b], t->v[p.g], t->v[p.r], p.a};
}
#endif

#ifdef PINGO_PIXEL_RGB565
static inline Pixel pixel_from_uint8(uint8_t g) { return (Pixel){g, g, g}; }
static inline Pixel pixel_mul(Pixel p, float f) {
  return (Pixel){p.red * f, p.green * f, p.blue * f};
}
static inline Pixel pixel_mul_table(Pixel p, const PixelShadeTable *t) {
  return (Pixel){t->v[p.red], t->v[p.green], t->v[p.blue]};
}
#endif
