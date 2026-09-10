#include "test_render_unit.h"

#include "render/depth.h"
#include "render/pixel.h"
#include "render/simd/span.h"
#include "render/texture.h"

// A deliberately naive model of one pixel, written straight from the
// expressions object.c used before the span seam existed. The reference
// implementation must agree with this; any assembly must agree with the
// reference. Keeping the model separate from the reference means a mistake has
// to be made twice, in two different styles, to pass unnoticed.
static void model_pixel(const PingoSpan *s, int i, Pixel *dst, PingoDepth *zt) {
  // w0 + i*dw0 rather than an accumulation. Both are exact in int32, so the
  // values agree bit for bit while the expression stays independent of the
  // reference's - which is the point of having a model at all.
  const int32_t w0 = s->w0 + i * s->dw0;
  const int32_t w1 = s->w1 + i * s->dw1;
  const int32_t w2 = s->w2 + i * s->dw2;

  const float depth = -(w0 * s->az + w1 * s->bz + w2 * s->cz) * s->areaInverse;
  const uint32_t v = (uint32_t)(depth * (float)PINGO_DEPTH_MAX);
  if (v < zt[i].d) {
    return;
  }
  zt[i].d = v;

  if (s->tex == 0) {
    dst[i] = s->flat;
    return;
  }
  const float interpInvW = w0 * s->invAw + w1 * s->invBw + w2 * s->invCw;
  if (interpInvW == 0) {
    return;
  }
  const float w = 1.0f / interpInvW;
  const Vec2f uv = {(w0 * s->tca.x + w1 * s->tcb.x + w2 * s->tcc.x) * w,
                    (w0 * s->tca.y + w1 * s->tcb.y + w2 * s->tcc.y) * w};
  const Pixel t = s->pow2 ? texture_read_uv_pow2(s->tex, uv, s->wmask, s->hmask)
                          : texture_read_uv((Texture *)s->tex, uv);
  dst[i] = s->shade ? pixel_mul_table(t, s->shade) : pixel_mul(t, s->factor);
}

#define SPAN_MAX 64

static uint32_t stored_depth_at(int i) { return (uint32_t)i * 40000000u; }

// Runs impl and the model over identical buffers and requires the colour and
// depth results to be bit-identical, not merely close.
static int agrees(const PingoSpan *proto, void (*impl)(const PingoSpan *),
                  const char *what) {
  Pixel c_impl[SPAN_MAX], c_model[SPAN_MAX];
  PingoDepth z_impl[SPAN_MAX], z_model[SPAN_MAX];

  for (int i = 0; i < SPAN_MAX; i++) {
    const Pixel seed = {(uint8_t)(i * 7), (uint8_t)(i * 13), (uint8_t)(i * 29),
                        255};
    c_impl[i] = c_model[i] = seed;
    // A spread of stored depths, so some pixels pass the test and some fail.
    z_impl[i].d = z_model[i].d = stored_depth_at(i);
  }

  PingoSpan a = *proto;
  a.dst = c_impl;
  a.depth = z_impl;
  impl(&a);

  for (int i = 0; i < proto->count; i++) {
    model_pixel(proto, i, c_model, z_model);
  }

  for (int i = 0; i < proto->count; i++) {
    TEST_ASSERT_EQ_INT(z_model[i].d, z_impl[i].d, what);
    TEST_ASSERT_EQ_INT(c_model[i].b, c_impl[i].b, what);
    TEST_ASSERT_EQ_INT(c_model[i].g, c_impl[i].g, what);
    TEST_ASSERT_EQ_INT(c_model[i].r, c_impl[i].r, what);
    TEST_ASSERT_EQ_INT(c_model[i].a, c_impl[i].a, what);
  }
  // Nothing past count may be touched.
  for (int i = proto->count; i < SPAN_MAX; i++) {
    TEST_ASSERT_EQ_INT(stored_depth_at(i), z_impl[i].d, "wrote past count");
  }
  return 1;
}

// Every vector width's remainder tail, for both 4-wide and 8-wide.
static const int counts[] = {0, 1, 4, 7, 8, 9, 15, 16, 17, 31, 32, 33};
#define COUNT_N ((int)(sizeof counts / sizeof counts[0]))

int test_span(void) {
  PingoSpan s;
  memset(&s, 0, sizeof s);
  s.count = 8;
  // Edge functions summing to 6000 against z of -0.5 and areaInverse 1/6000
  // put depth near 0.5 - which is also the 2^31 quantisation boundary the
  // vector implementations have to straddle correctly.
  s.w0 = 1000; s.w1 = 2000; s.w2 = 3000;
  s.dw0 = 5;   s.dw1 = -3;  s.dw2 = 2;
  s.az = -0.5f; s.bz = -0.5f; s.cz = -0.5f;
  s.areaInverse = 1.0f / 6000.0f;
  s.flat = (Pixel){10, 20, 30, 255};
  s.tex = 0;

  TEST_ASSERT(agrees(&s, pingo_span_ref, "flat fill, ref"), "flat ref");

  for (int i = 0; i < COUNT_N; i++) {
    s.count = counts[i];
    TEST_ASSERT(agrees(&s, pingo_span_ref, "flat fill tail, ref"), "flat tail");
  }

  // A 4x4 power-of-two texture, so both samplers are reachable.
  Pixel texels[16];
  for (int i = 0; i < 16; i++) {
    texels[i] = (Pixel){(uint8_t)(i * 16), (uint8_t)(255 - i * 16),
                        (uint8_t)(i * 3), 255};
  }
  Texture tx;
  texture_init(&tx, (Vec2i){4, 4}, texels);

  s.tex = &tx;
  s.pow2 = 1;
  s.wmask = 3;
  s.hmask = 3;
  s.invAw = 1.0f / 6000.0f;
  s.invBw = 1.2f / 6000.0f;
  s.invCw = 0.8f / 6000.0f;
  s.tca = (Vec2f){0.30f / 6000.0f, 0.80f / 6000.0f};
  s.tcb = (Vec2f){0.55f / 6000.0f, 0.10f / 6000.0f};
  s.tcc = (Vec2f){0.05f / 6000.0f, 0.45f / 6000.0f};
  s.factor = 0.6f;

  s.shade = 0; // the pixel_mul path
  for (int i = 0; i < COUNT_N; i++) {
    s.count = counts[i];
    TEST_ASSERT(agrees(&s, pingo_span_ref, "textured mul, ref"), "tex mul");
  }

  PixelShadeTable tbl; // the table path
  pixel_shade_table_init(&tbl, 0.6f);
  s.shade = &tbl;
  for (int i = 0; i < COUNT_N; i++) {
    s.count = counts[i];
    TEST_ASSERT(agrees(&s, pingo_span_ref, "textured table, ref"), "tex tbl");
  }

  s.pow2 = 0; // the general sampler
  s.shade = 0;
  s.count = 16;
  TEST_ASSERT(agrees(&s, pingo_span_ref, "textured non-pow2, ref"), "tex gen");

  // Negative u/v: the interpolated coordinate goes negative just outside a
  // triangle, and the reference relies on signed wrap before the cast.
  s.pow2 = 1;
  s.tca = (Vec2f){-0.90f / 6000.0f, -0.40f / 6000.0f};
  s.count = 16;
  TEST_ASSERT(agrees(&s, pingo_span_ref, "textured negative uv, ref"), "tex neg");

  // Depth boundaries, driven through az/bz/cz so the whole span sits at one
  // depth. The 2^31 convert boundary is at 0.5; the reference is undefined at
  // exactly 1.0, so approach it without reaching it.
  static const float depths[] = {0.0f, 0.4999999f, 0.5f, 0.5000001f,
                                 0.9999999f};
  s.tex = 0;
  s.count = 16;
  s.dw0 = s.dw1 = s.dw2 = 0; /* every pixel at the same depth */
  s.w0 = s.w1 = s.w2 = 2000;
  for (int i = 0; i < (int)(sizeof depths / sizeof depths[0]); i++) {
    const float d = depths[i];
    s.az = s.bz = s.cz = -d;
    s.areaInverse = 1.0f / 6000.0f;
    TEST_ASSERT(agrees(&s, pingo_span_ref, "depth boundary, ref"), "depth ref");
  }

  return 1;
}
