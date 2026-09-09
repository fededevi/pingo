#include "pixel.h"

#ifdef PINGO_PIXEL_UINT8

extern Pixel pixel_random() { return (Pixel){(uint8_t)rand()}; }

uint8_t pixel_to_uint8(Pixel *p) { return p->g; }

extern Pixel pixel_from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  return (Pixel){((r + g + b) / 3)};
}
#endif

#ifdef PINGO_PIXEL_RGB888
extern Pixel pixel_random() {
  return (Pixel){(uint8_t)rand(), (uint8_t)rand(), (uint8_t)rand()};
}

extern uint8_t pixel_to_uint8(Pixel *p) { return (p->r + p->g + p->b) / 3; }

extern Pixel pixel_from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  (void)a;
  return (Pixel){r, g, b};
}
#endif

#ifdef PINGO_PIXEL_RGBA8888
extern Pixel pixel_random() {
  return (Pixel){(uint8_t)rand(), (uint8_t)rand(), (uint8_t)rand(), 255};
}

extern uint8_t pixel_to_uint8(Pixel *p) { return (p->r + p->g + p->b) / 3; }

extern Pixel pixel_from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  return (Pixel){r, g, b, a};
}

#endif

#ifdef PINGO_PIXEL_BGRA8888
extern Pixel pixel_random() {
  return (Pixel){(uint8_t)rand(), (uint8_t)rand(), (uint8_t)rand(), 255};
}

extern uint8_t pixel_to_uint8(Pixel *p) { return (p->r + p->g + p->b) / 3; }

extern Pixel pixel_from_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  return (Pixel){b, g, r, a};
}

#endif
