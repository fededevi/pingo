#include "object.h"
#include "backend.h"
#include "depth.h"

#include <stdbool.h>

// Rows at least this wide get their exact span solved for; narrower rows just
// test each pixel. Tunable so the trade-off can be measured rather than
// guessed - see the comment at the loop.
#ifndef PINGO_SPAN_CLIP_MIN_WIDTH
#define PINGO_SPAN_CLIP_MIN_WIDTH 16
#endif

// Bounding-box area from which tabulating a triangle's shade beats converting
// and multiplying three channels for every one of its pixels. Chosen by
// measurement; the table has 256 entries, so it needs a triangle of about
// that size to repay building it.
#ifndef PINGO_SHADE_TABLE_MIN_AREA
#define PINGO_SHADE_TABLE_MIN_AREA 256
#endif
#include "math/fun.h"
#include "math/mat4.h"
#include "mesh.h"
#include "render/material.h"
#include "renderer.h"
#include "state.h"

// Frustum culling helper function
static int frustum_cull_triangle(Vec4f a, Vec4f b, Vec4f c) {
  // Simple frustum culling - check if all vertices are outside any plane

  // Near plane (z > w)
  if (a.z > a.w && b.z > b.w && c.z > c.w)
    return 1;

  // Far plane (z < -w)
  if (a.z < -a.w && b.z < -b.w && c.z < -c.w)
    return 1;

  // Left plane (x < -w)
  if (a.x < -a.w && b.x < -b.w && c.x < -c.w)
    return 1;

  // Right plane (x > w)
  if (a.x > a.w && b.x > b.w && c.x > c.w)
    return 1;

  // Top plane (y > w)
  if (a.y > a.w && b.y > b.w && c.y > c.w)
    return 1;

  // Bottom plane (y < -w)
  if (a.y < -a.w && b.y < -b.w && c.y < -c.w)
    return 1;

  return 0; // Not culled
}

// Integer division rounding toward minus infinity and toward plus infinity.
// C truncates toward zero, which is the wrong direction for half the signs
// that come up when solving an edge equation for its zero crossing.
static inline int32_t div_floor(int32_t a, int32_t b) {
  int32_t q = a / b;
  if (a % b != 0 && ((a < 0) != (b < 0))) {
    q--;
  }
  return q;
}

static inline int32_t div_ceil(int32_t a, int32_t b) {
  int32_t q = a / b;
  if (a % b != 0 && ((a < 0) == (b < 0))) {
    q++;
  }
  return q;
}

// Narrows [*lo, *hi] to the offsets where w + d * step >= 0 holds. Returns
// false when the edge excludes the whole row.
static inline bool span_clip(int32_t w, int32_t step, int32_t *lo,
                            int32_t *hi) {
  if (step == 0) {
    return w >= 0;
  }
  if (step > 0) {
    const int32_t bound = div_ceil(-w, step);
    if (bound > *lo) {
      *lo = bound;
    }
  } else {
    const int32_t bound = div_floor(-w, step);
    if (bound < *hi) {
      *hi = bound;
    }
  }
  return *lo <= *hi;
}

int object_render(void *this, Mat4 m, Renderer *r) {
  Object *o = this;

  IF_NULL_RETURN(o, RENDER_ERROR);
  IF_NULL_RETURN(r, RENDER_ERROR);

  const Vec2i scrSize = r->target.color.size;

  // VIEW MATRIX
  Mat4 v = mat4Inverse(&r->camera_view);
  Mat4 p = r->camera_projection;

  // Read from the target rather than fetched through the backend: it used to
  // be an opaque call made two or three times per pixel.
  PingoDepth *const zeta = r->target.depth;

  // All loop-invariant, and none of it can be hoisted by the compiler: these
  // are opaque cross-translation-unit calls, so it has to assume every one of
  // them could return something different or touch memory.
  const Mat4 vm = mat4MultiplyM(&v, &m);
  const Vec3f light = vec3Normalize((Vec3f){-8, 5, 5});
  const float halfX = scrSize.x * 0.5f;
  const float halfY = scrSize.y * 0.5f;

  // Fixed for the whole object, so the sampler need not re-derive it for
  // every textured pixel - and the general path's code stays out of the
  // innermost loop.
  const Texture *tex = (o->material != 0) ? o->material->texture : 0;
  const int tex_pow2 = (tex != 0) && texture_is_pow2(tex);
  const int tex_w_mask = (tex != 0) ? tex->size.x - 1 : 0;
  const int tex_h_mask = (tex != 0) ? tex->size.y - 1 : 0;

  for (int i = 0; i < o->mesh->index_count; i += 3) {
    const Vec3f *ver1 = &o->mesh->positions[o->mesh->pos_indices[i + 0]];
    const Vec3f *ver2 = &o->mesh->positions[o->mesh->pos_indices[i + 1]];
    const Vec3f *ver3 = &o->mesh->positions[o->mesh->pos_indices[i + 2]];

    Vec4f a = {ver1->x, ver1->y, ver1->z, 1};
    Vec4f b = {ver2->x, ver2->y, ver2->z, 1};
    Vec4f c = {ver3->x, ver3->y, ver3->z, 1};

    a = mat4MultiplyVec4(&a, &vm);
    b = mat4MultiplyVec4(&b, &vm);
    c = mat4MultiplyVec4(&c, &vm);

    // View-space positions, kept for the face normal below. Built member-wise
    // on purpose: casting &a from Vec4f* to Vec3f* and dereferencing it
    // violates strict aliasing, which -O2 and above are entitled to act on.
    const Vec3f a3 = {a.x, a.y, a.z};
    const Vec3f b3 = {b.x, b.y, b.z};
    const Vec3f c3 = {c.x, c.y, c.z};

    a = mat4MultiplyVec4(&a, &p);
    b = mat4MultiplyVec4(&b, &p);
    c = mat4MultiplyVec4(&c, &p);

    // Frustum culling (configurable)
    if (r->enable_frustum_culling && frustum_cull_triangle(a, b, c))
      continue;

    // Triangle is completely behind camera
    if (a.z > 0 && b.z > 0 && c.z > 0)
      continue;

    // Clip-space w, kept because the perspective divide just below replaces
    // it with 1 and the texture interpolation needs it.
    const float aw = a.w;
    const float bw = b.w;
    const float cw = c.w;

    // convert to device coordinates by perspective division
    a.x /= a.w;
    a.y /= a.w;
    a.z /= a.w;
    a.w = 1;
    b.x /= b.w;
    b.y /= b.w;
    b.z /= b.w;
    b.w = 1;
    c.x /= c.w;
    c.y /= c.w;
    c.z /= c.w;
    c.w = 1;

    // Backface culling (configurable)
    if (r->enable_backface_culling) {
      float clocking = isClockwise(a.x, a.y, b.x, b.y, c.x, c.y);
      if (clocking >= 0)
        continue;
    }

    // Everything below is for a triangle that will actually be drawn. The
    // normal costs a cross product and a square root, and on a closed mesh
    // about half of all triangles are discarded above - so computing it
    // before the culls, as this used to, threw that half away.
    const Vec3f na = vec3fsubV(a3, b3);
    const Vec3f nb = vec3fsubV(a3, c3);
    const Vec3f normal = vec3Normalize(vec3Cross(na, nb));
    float diffuseLight = (1.0f + vec3Dot(normal, light)) * 0.5f;
    diffuseLight = MIN(1.0f, MAX(diffuseLight, 0.0f));

    Vec2f tca = {0, 0};
    Vec2f tcb = {0, 0};
    Vec2f tcc = {0, 0};

    // A mesh need not carry texture coordinates - teapot and pingo do not.
    // Checking only the material dereferenced NULL for those, so every such
    // mesh crashed the renderer instead of drawing untextured.
    if (o->material != 0 && o->mesh->tex_coords != 0 &&
        o->mesh->tex_indices != 0) {
      tca = o->mesh->tex_coords[o->mesh->tex_indices[i + 0]];
      tcb = o->mesh->tex_coords[o->mesh->tex_indices[i + 1]];
      tcc = o->mesh->tex_coords[o->mesh->tex_indices[i + 2]];
    }

    // Compute Screen coordinates (optimized)
    Vec2i a_s = {(int)(a.x * halfX + halfX), (int)(a.y * halfY + halfY)};
    Vec2i b_s = {(int)(b.x * halfX + halfX), (int)(b.y * halfY + halfY)};
    Vec2i c_s = {(int)(c.x * halfX + halfX), (int)(c.y * halfY + halfY)};

    int32_t minX = MIN(MIN(a_s.x, b_s.x), c_s.x);
    int32_t minY = MIN(MIN(a_s.y, b_s.y), c_s.y);
    int32_t maxX = MAX(MAX(a_s.x, b_s.x), c_s.x);
    int32_t maxY = MAX(MAX(a_s.y, b_s.y), c_s.y);

    minX = MIN(MAX(minX, 0), r->target.color.size.x);
    minY = MIN(MAX(minY, 0), r->target.color.size.y);
    maxX = MIN(MAX(maxX, 0), r->target.color.size.x);
    maxY = MIN(MAX(maxY, 0), r->target.color.size.y);

    // Barycentric coordinates at minX/minY corner
    Vec2i minTriangle = {minX, minY};

    int32_t area = orient2d(a_s, b_s, c_s);
    if (area == 0)
      continue;
    const float areaInverse = 1.0f / (float)area;

    int32_t A01 = (a_s.y - b_s.y); // Barycentric coordinates steps
    int32_t B01 = (b_s.x - a_s.x); // Barycentric coordinates steps
    int32_t A12 = (b_s.y - c_s.y); // Barycentric coordinates steps
    int32_t B12 = (c_s.x - b_s.x); // Barycentric coordinates steps
    int32_t A20 = (c_s.y - a_s.y); // Barycentric coordinates steps
    int32_t B20 = (a_s.x - c_s.x); // Barycentric coordinates steps

    int32_t w0_row = orient2d(b_s, c_s, minTriangle);
    int32_t w1_row = orient2d(c_s, a_s, minTriangle);
    int32_t w2_row = orient2d(a_s, b_s, minTriangle);

    // Perspective-correct texturing interpolates u/w, v/w and 1/w linearly in
    // screen space and divides at the end. The previous code divided by NDC z
    // instead, which is not proportional to w, so texture coordinates barely
    // varied across a surface receding from the camera.
    // Fixed for the whole triangle: diffuseLight does not vary per pixel, so
    // computing this inside the loop repeated the same work for every pixel of
    // an untextured mesh - and two of the four shipped meshes are untextured.
    const Pixel flat_color = pixel_mul(pixel_from_uint8(255), diffuseLight);

    PixelShadeTable shade;
    const int use_shade_table =
        (o->material != 0) &&
        ((int32_t)(maxX - minX) * (maxY - minY) >= PINGO_SHADE_TABLE_MIN_AREA);
    if (use_shade_table) {
      pixel_shade_table_init(&shade, diffuseLight);
    }

    float invAw = 0, invBw = 0, invCw = 0;
    if (o->material != 0 && aw != 0 && bw != 0 && cw != 0) {
      invAw = 1.0f / aw;
      invBw = 1.0f / bw;
      invCw = 1.0f / cw;
      tca.x *= invAw;
      tca.y *= invAw;
      tcb.x *= invBw;
      tcb.y *= invBw;
      tcc.x *= invCw;
      tcc.y *= invCw;
    }

    for (int16_t y = minY; y < maxY;
         y++, w0_row += B12, w1_row += B20, w2_row += B01) {
      // Solve for the run of x on this row where all three edge functions are
      // non-negative, instead of walking the whole bounding box and rejecting
      // most of it. Each edge is linear in x, so its sign change is one
      // division; a triangle covers about half its bounding box, so this is
      // half the iterations.
      int32_t lo = 0;
      int32_t hi = maxX - minX - 1;
      // Only worth it once the row is wide enough to repay three integer
      // divisions. Below that the per-pixel sign test is cheaper, which is the
      // common case for a densely tessellated mesh.
      if (hi >= PINGO_SPAN_CLIP_MIN_WIDTH) {
        if (!span_clip(w0_row, A12, &lo, &hi) ||
            !span_clip(w1_row, A20, &lo, &hi) ||
            !span_clip(w2_row, A01, &lo, &hi) || lo > hi) {
          continue;
        }
      }

      int32_t w0 = w0_row + lo * A12;
      int32_t w1 = w1_row + lo * A20;
      int32_t w2 = w2_row + lo * A01;

      for (int32_t x = minX + lo; x <= minX + hi;
           x++, w0 += A12, w1 += A20, w2 += A01) {
        // The span bounds are exact, so this is only a guard against an edge
        // case in the arithmetic above rather than the primary rejection.
        if ((w0 | w1 | w2) < 0)
          continue;

        float depth = -(w0 * a.z + w1 * b.z + w2 * c.z) * areaInverse;
        // Lower bound 0, not -1. depth_test_and_write casts this to an
        // unsigned type, which is undefined for a negative value, and the
        // old bound admitted [-1, 0). Nothing in the tests or the shipped
        // meshes ever produced a negative depth - instrumenting every write
        // across all eight scenes counted zero - so this rejects nothing that
        // used to be drawn, and makes the cast unreachable for negatives
        // rather than merely unreached.
        if (depth < 0.0f || depth > 1.0f)
          continue;

        const int pixel_index = x + y * scrSize.x;
        if (!depth_test_and_write(zeta, pixel_index, depth))
          continue;

        if (o->material != 0) {
          // Texture lookup

          const float interpInvW = w0 * invAw + w1 * invBw + w2 * invCw;
          if (interpInvW == 0) {
            continue;
          }
          // One reciprocal and two multiplies, rather than dividing twice by
          // the same value. A float division is an order of magnitude dearer
          // than a multiply, and this is per textured pixel.
          const float w = 1.0f / interpInvW;
          const float textCoordx = (w0 * tca.x + w1 * tcb.x + w2 * tcc.x) * w;
          const float textCoordy = (w0 * tca.y + w1 * tcb.y + w2 * tcc.y) * w;

          const Vec2f uv = {textCoordx, textCoordy};
          Pixel text =
              tex_pow2 ? texture_read_uv_pow2(tex, uv, tex_w_mask, tex_h_mask)
                       : texture_read_uv(o->material->texture, uv);
          texture_draw_index(&r->target.color, pixel_index,
                             use_shade_table
                                 ? pixel_mul_table(text, &shade)
                                 : pixel_mul(text, diffuseLight));
        } else {
          texture_draw_index(&r->target.color, pixel_index, flat_color);
        }
      }
    }
  }

  return OK;
};

int object_init(Object *this, Mesh *mesh, Material *material) {
  IF_NULL_RETURN(this, INIT_ERROR);
  IF_NULL_RETURN(mesh, INIT_ERROR);
  // IF_NULL_RETURN(material, INIT_ERROR);

  this->material = material;
  this->mesh = mesh;
  this->renderable.render = &object_render;

  return OK;
}
