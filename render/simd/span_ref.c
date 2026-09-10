#include "render/simd/span.h"

#include "render/depth.h"
#include "render/texture.h"

#include <stddef.h>

// The layout render/simd/span_*.S hard-codes as byte offsets. A field
// reordered or resized without updating the assembly would corrupt pixels
// silently; this makes it a compile error instead. C99 has no _Static_assert,
// so the negative array size is the idiom.
typedef char pingo_span_layout_check
    [(offsetof(PingoSpan, dst) == 0 && offsetof(PingoSpan, depth) == 8 &&
      offsetof(PingoSpan, count) == 16 && offsetof(PingoSpan, w0) == 20 &&
      offsetof(PingoSpan, dw0) == 32 && offsetof(PingoSpan, az) == 44 &&
      offsetof(PingoSpan, areaInverse) == 56 &&
      offsetof(PingoSpan, tex) == 96 && offsetof(PingoSpan, flat) == 116 &&
      offsetof(PingoSpan, shade) == 120 &&
      offsetof(PingoSpan, factor) == 128 && sizeof(PingoSpan) == 136)
         ? 1
         : -1];

// The definition of correct: the expressions object.c evaluated per pixel,
// moved behind one call and not otherwise altered. Every other implementation
// must be bit-identical to this, which the differential test in
// render/tests/unit/test_span.c establishes.
//
// Two loops rather than one with a branch: the texture test is fixed for the
// whole span, so hoisting it keeps the flat path free of code it never
// executes - the same reason object.c hoists it out of the row loop.
void pingo_span_ref(const PingoSpan *s) {
  Pixel *const dst = s->dst;
  PingoDepth *const zt = s->depth;
  const int32_t n = s->count;

  int32_t w0 = s->w0, w1 = s->w1, w2 = s->w2;

  if (s->tex == 0) {
    for (int32_t i = 0; i < n;
         i++, w0 += s->dw0, w1 += s->dw1, w2 += s->dw2) {
      const float depth =
          -(w0 * s->az + w1 * s->bz + w2 * s->cz) * s->areaInverse;
      // Lower bound 0, not -1: depth_test_and_write casts to an unsigned
      // type, which is undefined for a negative value. Upper bound 1 for the
      // same reason - (float)UINT32_MAX rounds to 2^32, so 1.0 itself does
      // not fit either. object.c applied this before the seam existed and it
      // has to stay, or the cast becomes reachable for values it cannot hold.
      if (depth < 0.0f || depth > 1.0f) {
        continue;
      }
      if (!depth_test_and_write(zt, (int)i, depth)) {
        continue;
      }
      dst[i] = s->flat;
    }
    return;
  }

  for (int32_t i = 0; i < n; i++, w0 += s->dw0, w1 += s->dw1, w2 += s->dw2) {
    const float depth =
        -(w0 * s->az + w1 * s->bz + w2 * s->cz) * s->areaInverse;
    if (depth < 0.0f || depth > 1.0f) {
      continue;
    }
    if (!depth_test_and_write(zt, (int)i, depth)) {
      continue;
    }

    const float interpInvW =
        w0 * s->invAw + w1 * s->invBw + w2 * s->invCw;
    if (interpInvW == 0) {
      continue;
    }
    const float w = 1.0f / interpInvW;
    const Vec2f uv = {(w0 * s->tca.x + w1 * s->tcb.x + w2 * s->tcc.x) * w,
                      (w0 * s->tca.y + w1 * s->tcb.y + w2 * s->tcc.y) * w};
    const Pixel t = s->pow2
                        ? texture_read_uv_pow2(s->tex, uv, s->wmask, s->hmask)
                        : texture_read_uv((Texture *)s->tex, uv);
    dst[i] = s->shade ? pixel_mul_table(t, s->shade) : pixel_mul(t, s->factor);
  }
}
