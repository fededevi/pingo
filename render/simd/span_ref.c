#include "render/simd/span.h"

#include "render/depth.h"
#include "render/texture.h"

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
