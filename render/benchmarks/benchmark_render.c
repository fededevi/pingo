/**
 * Render benchmark: renders a fixed number of frames and times them.
 *
 * A fixed count rather than a fixed duration, so every target does the same
 * work and the numbers compare directly - a slow cross-compiled target simply
 * takes longer instead of reporting a different sample.
 *
 * It draws through the memory backend, so it needs no display and runs
 * wherever the tests do. Geometry is generated, which keeps the triangle
 * count an input to the measurement rather than a property of an asset file.
 *
 * The Mpx/s column is frame area per second, not pixels shaded: the quad
 * covers the frame so for it the two coincide, while the sphere leaves most
 * of the frame to the clear, and only its fill rate is directly comparable.
 */

#include "memory_backend.h"

#include "render/entity.h"
#include "render/material.h"
#include "render/mesh.h"
#include "render/object.h"
#include "render/renderer.h"
#include "render/state.h"
#include "render/texture.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DEFAULT_FRAMES 100

// A sphere is a convenient generator: rings x segments x 2 triangles of
// similar size, and it self-occludes, so the depth buffer is exercised too.
#define MAX_RINGS 40
#define MAX_SEGMENTS 40
#define MAX_VERTS ((MAX_RINGS + 1) * (MAX_SEGMENTS + 1))
#define MAX_INDICES (MAX_RINGS * MAX_SEGMENTS * 6)

static Vec3f positions[MAX_VERTS];
static Vec2f coords[MAX_VERTS];
static uint16_t indices[MAX_INDICES];
static Pixel checker[4];

// CLOCK_MONOTONIC would be better but is not everywhere; clock() measures CPU
// time, which is what we want anyway since nothing here waits on anything.
static double elapsed_seconds(clock_t from, clock_t to) {
  return (double)(to - from) / (double)CLOCKS_PER_SEC;
}

// Builds a unit sphere and returns its triangle count.
static int build_sphere(Mesh *mesh, int rings, int segments) {
  int v = 0;
  for (int r = 0; r <= rings; r++) {
    const float phi = 3.14159265f * (float)r / (float)rings;
    const float sp = sinf(phi), cp = cosf(phi);
    for (int s = 0; s <= segments; s++) {
      const float theta = 6.28318531f * (float)s / (float)segments;
      positions[v] = (Vec3f){sp * cosf(theta), cp, sp * sinf(theta)};
      coords[v] = (Vec2f){(float)s / (float)segments, (float)r / (float)rings};
      v++;
    }
  }

  int n = 0;
  for (int r = 0; r < rings; r++) {
    for (int s = 0; s < segments; s++) {
      const uint16_t a = (uint16_t)(r * (segments + 1) + s);
      const uint16_t b = (uint16_t)(a + segments + 1);
      indices[n++] = a;
      indices[n++] = b;
      indices[n++] = (uint16_t)(a + 1);
      indices[n++] = b;
      indices[n++] = (uint16_t)(b + 1);
      indices[n++] = (uint16_t)(a + 1);
    }
  }

  mesh->indexes_count = n;
  mesh->pos_indices = indices;
  mesh->tex_indices = indices;
  mesh->positions = positions;
  mesh->textCoord = coords;
  return n / 3;
}

// Two triangles covering the whole frame: a fill-rate figure rather than a
// geometry one, since the per-triangle setup is negligible here.
static const Vec3f quad_pos[4] = {{-6, -6, 0}, {6, -6, 0}, {6, 6, 0}, {-6, 6, 0}};
static const Vec2f quad_uv[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
static const uint16_t quad_idx[6] = {0, 1, 2, 0, 2, 3};

static int build_quad(Mesh *mesh) {
  mesh->indexes_count = 6;
  mesh->pos_indices = quad_idx;
  mesh->tex_indices = quad_idx;
  mesh->positions = quad_pos;
  mesh->textCoord = quad_uv;
  return 2;
}

// rings == 0 selects the quad; otherwise a sphere of that resolution.
static int measure(const char *name, int frames, int width, int height,
                   int rings, int segments, float distance) {
  MemoryBackend backend;
  if (memory_backend_init(&backend, (Vec2i){width, height}) != 0) {
    fprintf(stderr, "  cannot allocate the %dx%d buffers\n", width, height);
    return 1;
  }

  Renderer renderer;
  if (renderer_init(&renderer, (Vec2i){width, height},
                    (Backend *)&backend) != OK) {
    fprintf(stderr, "  renderer_init failed\n");
    memory_backend_free(&backend);
    return 1;
  }

  Texture texture;
  Material material;
  texture_init(&texture, (Vec2i){2, 2}, checker);
  material_init(&material, &texture);

  Mesh mesh;
  const int triangles =
      rings ? build_sphere(&mesh, rings, segments) : build_quad(&mesh);

  Object object;
  Entity root;
  object_init(&object, &mesh, &material);
  entity_init(&root, (Renderable *)&object, mat4Identity());
  renderer_set_root_renderable(&renderer, (Renderable *)&root);

  renderer.camera_projection =
      mat4Perspective(1, 200.0, (float)width / (float)height, 0.6);
  renderer.camera_view = mat4Translate((Vec3f){0, 0, 0});

  // A sphere's silhouette does not change as it turns, so spinning it varies
  // the triangle slopes without changing the pixel count. A flat quad is the
  // opposite: rotating it takes it edge-on and then behind its own cull, so
  // the cost per frame would fall away as the run got longer. Hence the quad
  // is held still - it is a fill-rate case, and its orientation is the point.
  const int spin = (rings != 0);
  Mat4 translation = mat4Translate((Vec3f){0, 0, -distance});

  // Untimed warmup: it faults in the buffers and lets the clock leave its
  // idle state, both of which otherwise land entirely on the first frames and
  // made short runs look several times slower per frame than long ones.
  const int warmup = frames < 10 ? frames : frames / 10 + 1;
  for (int i = 0; i < warmup; i++) {
    Mat4 rotation = mat4RotateY(spin ? 0.01f * (float)i : 0.0f);
    root.transform = mat4MultiplyM(&rotation, &translation);
    if (renderer_render(&renderer) != OK) {
      fprintf(stderr, "  renderer_render failed\n");
      memory_backend_free(&backend);
      return 1;
    }
  }

  const clock_t start = clock();
  for (int i = 0; i < frames; i++) {
    Mat4 rotation = mat4RotateY(spin ? 0.01f * (float)i : 0.0f);
    root.transform = mat4MultiplyM(&rotation, &translation);
    if (renderer_render(&renderer) != OK) {
      fprintf(stderr, "  renderer_render failed\n");
      memory_backend_free(&backend);
      return 1;
    }
  }
  const double seconds = elapsed_seconds(start, clock());

  const double per_frame = seconds / frames;
  printf("  %-14s %4dx%-4d %6d tri  %8.3f s  %8.3f ms/frame  %8.1f fps"
         "  %6.2f Mtri/s  %6.1f Mpx/s\n",
         name, width, height, triangles, seconds, per_frame * 1000.0,
         1.0 / per_frame, triangles / per_frame / 1e6,
         (double)width * height / per_frame / 1e6);

  memory_backend_free(&backend);
  return 0;
}

int main(int argc, char **argv) {
  int frames = DEFAULT_FRAMES;
  if (argc == 2) {
    frames = atoi(argv[1]);
  }
  if (argc > 2 || frames < 1) {
    fprintf(stderr, "usage: %s [frames]\n", argv[0]);
    return 2;
  }

  checker[0] = pixelFromRGBA(255, 255, 255, 255);
  checker[1] = pixelFromRGBA(60, 60, 60, 255);
  checker[2] = pixelFromRGBA(60, 60, 60, 255);
  checker[3] = pixelFromRGBA(255, 255, 255, 255);

  printf("=== Pingo render benchmark: %d frames per case ===\n", frames);
  printf("  %-14s %-9s %10s %8s %14s %10s %13s %11s\n", "scene", "size",
         "triangles", "total", "per frame", "rate", "triangles", "frame area");

  int bad = 0;

  printf("\nfill rate - two triangles covering the frame\n");
  bad |= measure("quad", frames, 64, 48, 0, 0, 5.0f);
  bad |= measure("quad", frames, 160, 120, 0, 0, 5.0f);
  bad |= measure("quad", frames, 320, 240, 0, 0, 5.0f);
  bad |= measure("quad", frames, 640, 480, 0, 0, 5.0f);

  printf("\ngeometry - a self-occluding sphere at a fixed resolution\n");
  bad |= measure("sphere 8x8", frames, 320, 240, 8, 8, 3.0f);
  bad |= measure("sphere 20x20", frames, 320, 240, 20, 20, 3.0f);
  bad |= measure("sphere 40x40", frames, 320, 240, 40, 40, 3.0f);

  printf("\nresolution - the same 800 triangles at four sizes\n");
  bad |= measure("sphere 20x20", frames, 64, 48, 20, 20, 3.0f);
  bad |= measure("sphere 20x20", frames, 160, 120, 20, 20, 3.0f);
  bad |= measure("sphere 20x20", frames, 320, 240, 20, 20, 3.0f);
  bad |= measure("sphere 20x20", frames, 640, 480, 20, 20, 3.0f);

  return bad ? 1 : 0;
}
