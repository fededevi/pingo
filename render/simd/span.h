#pragma once

// One run of pixels, handed to one function.
//
// The run is known to be fully covered. object.c calls this only where
// span_clip has already narrowed the row, so no coverage mask is needed and
// the only per-pixel decision is the depth test.
//
// The three edge functions and their per-x steps are passed as integers, and
// every interpolant is recomputed per pixel from them, exactly as the scalar
// loop did. That is deliberate and it cost a redesign to learn: carrying a
// float start-and-delta per interpolant instead, and accumulating, is not
// bit-identical to recomputing. Integer accumulation is exact; float
// accumulation drifts by a relative 2^-24 per step, and depth is quantised by
// 2^32, so that drift reaches tens of depth units - enough to flip a depth
// test at a silhouette and move a golden image. The differential test caught
// it on the first run.

#include "render/fwd.h"
#include "render/pixel.h"
#include "math/vec2.h"

#include <stdint.h>

typedef struct PingoSpan {
  Pixel *dst;        // &color[x0 + y*width]
  PingoDepth *depth; // &zeta[x0 + y*width]
  int32_t count;     // pixels in the run; may legitimately be 0

  // Edge functions at the first pixel, and their exact per-x steps.
  int32_t w0, w1, w2;
  int32_t dw0, dw1, dw2;

  // Per-triangle, fixed for the whole span.
  float az, bz, cz;          // NDC z at the three vertices
  float areaInverse;
  float invAw, invBw, invCw; // 1/w at the three vertices
  Vec2f tca, tcb, tcc;       // texture coords, already divided by w

  const Texture *tex;   // NULL selects the flat-fill path
  int32_t wmask, hmask; // meaningful only when pow2 is set
  int32_t pow2;         // selects the masked sampler

  Pixel flat;                   // used when tex == NULL
  const PixelShadeTable *shade; // NULL selects pixel_mul(text, factor)
  float factor;                 // used when shade == NULL
} PingoSpan;

// The reference. Always compiled, on every target, and externally callable
// even where pingo_span resolves to assembly - the differential test needs
// both reachable by name inside one binary.
void pingo_span_ref(const PingoSpan *s);

#if defined(PINGO_SIMD_AVX2)
void pingo_span_avx2(const PingoSpan *s);
#endif
#if defined(PINGO_SIMD_SSE2)
void pingo_span_sse2(const PingoSpan *s);
#endif

// Compile-time dispatch. No probe, no function pointer, no indirect call: the
// build has already decided, so this collapses to a direct call.
static inline void pingo_span(const PingoSpan *s) {
#if defined(PINGO_SIMD_AVX2)
  pingo_span_avx2(s);
#elif defined(PINGO_SIMD_SSE2)
  pingo_span_sse2(s);
#else
  pingo_span_ref(s);
#endif
}
