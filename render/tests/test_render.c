/**
 * Golden-image tests for the renderer.
 *
 * Each scene is rendered into memory at 64x48 and compared against a
 * committed PPM. Run with --write-references to regenerate them after a
 * deliberate change; inspect the PPMs with any image viewer.
 *
 * On a mismatch the actual image is written next to the executable as
 * <scene>-actual.ppm so it can be compared by eye.
 *
 * The comparison is not exact. Coverage is decided by floating point, so a
 * last-bit difference between architectures can flip a pixel on a triangle
 * edge from covered to uncovered - a full-value change, not a small one.
 * A handful of such pixels is therefore tolerated, while a real regression
 * moves far more than that.
 */

#include "memory_backend.h"

#include "render/entity.h"
#include "render/material.h"
#include "render/mesh.h"
#include "render/object.h"
#include "render/renderer.h"
#include "render/state.h"
#include "render/texture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH 64
#define HEIGHT 48
#define PIXEL_COUNT (WIDTH * HEIGHT)

// Per-channel difference treated as noise rather than a change.
#define CHANNEL_TOLERANCE 2
// Pixels allowed to differ beyond that, for the edge-coverage flips above.
#define MAX_DIFFERING_PIXELS (PIXEL_COUNT / 200) // 0.5%, i.e. 15 of 3072

typedef struct {
  const char *name;
  void (*build)(Renderer *renderer);
} Scene;

// ---------------------------------------------------------------------------
// Scene fixtures. All geometry is written out literally so that the expected
// image depends on nothing but the renderer.
// ---------------------------------------------------------------------------

static Pixel checker[4];
static Texture texture;
static Material material;
static Mesh mesh;
static Object object;
static Entity root;

static void init_checker_texture(void) {
  checker[0] = pixelFromRGBA(255, 255, 255, 255);
  checker[1] = pixelFromRGBA(60, 60, 60, 255);
  checker[2] = pixelFromRGBA(60, 60, 60, 255);
  checker[3] = pixelFromRGBA(255, 255, 255, 255);
  texture_init(&texture, (Vec2i){2, 2}, checker);
  material_init(&material, &texture);
}

// Nothing drawn: exercises the clear path on its own.
static void build_empty(Renderer *renderer) {
  init_checker_texture();
  mesh.indexes_count = 0;
  mesh.pos_indices = NULL;
  mesh.tex_indices = NULL;
  mesh.positions = NULL;
  mesh.textCoord = NULL;
  object_init(&object, &mesh, &material);
  entity_init(&root, (Renderable *)&object, mat4Identity());
  renderer_set_root_renderable(renderer, (Renderable *)&root);

  renderer->camera_projection =
      mat4Perspective(1, 50.0, (float)WIDTH / (float)HEIGHT, 0.6);
  renderer->camera_view = mat4Translate((Vec3f){0, 0, 0});
}

static Vec3f tri_positions[3];
static Vec2f tri_coords[3];
static uint16_t tri_indices[3];

// One triangle straight in front of the camera.
static void build_triangle(Renderer *renderer) {
  init_checker_texture();

  tri_positions[0] = (Vec3f){-1.0f, -0.8f, 0.0f};
  tri_positions[1] = (Vec3f){1.0f, -0.8f, 0.0f};
  tri_positions[2] = (Vec3f){0.0f, 1.0f, 0.0f};
  tri_coords[0] = (Vec2f){0.0f, 0.0f};
  tri_coords[1] = (Vec2f){1.0f, 0.0f};
  tri_coords[2] = (Vec2f){0.5f, 1.0f};
  tri_indices[0] = 0;
  tri_indices[1] = 1;
  tri_indices[2] = 2;

  mesh.indexes_count = 3;
  mesh.pos_indices = tri_indices;
  mesh.tex_indices = tri_indices;
  mesh.positions = tri_positions;
  mesh.textCoord = tri_coords;

  object_init(&object, &mesh, &material);
  entity_init(&root, (Renderable *)&object,
              mat4Translate((Vec3f){0, 0, -2.2f}));
  renderer_set_root_renderable(renderer, (Renderable *)&root);

  renderer->camera_projection =
      mat4Perspective(1, 50.0, (float)WIDTH / (float)HEIGHT, 0.6);
  renderer->camera_view = mat4Translate((Vec3f){0, 0, 0});
}

static Vec3f cube_positions[8];
static Vec2f cube_coords[4];
static uint16_t cube_pos_indices[36];
static uint16_t cube_tex_indices[36];

// A rotated cube: transforms, the depth buffer and backface culling together.
static void build_cube(Renderer *renderer) {
  init_checker_texture();

  const float h = 0.9f;
  const Vec3f corners[8] = {
      {-h, -h, -h}, {h, -h, -h}, {h, h, -h}, {-h, h, -h},
      {-h, -h, h},  {h, -h, h},  {h, h, h},  {-h, h, h},
  };
  for (int i = 0; i < 8; i++) {
    cube_positions[i] = corners[i];
  }

  cube_coords[0] = (Vec2f){0.0f, 0.0f};
  cube_coords[1] = (Vec2f){1.0f, 0.0f};
  cube_coords[2] = (Vec2f){1.0f, 1.0f};
  cube_coords[3] = (Vec2f){0.0f, 1.0f};

  // Two triangles per face, wound so the outward faces survive culling.
  const uint16_t faces[6][4] = {
      {0, 1, 2, 3}, {5, 4, 7, 6}, {4, 0, 3, 7},
      {1, 5, 6, 2}, {4, 5, 1, 0}, {3, 2, 6, 7},
  };
  int n = 0;
  for (int f = 0; f < 6; f++) {
    const int order[6] = {0, 1, 2, 0, 2, 3};
    for (int k = 0; k < 6; k++) {
      cube_pos_indices[n] = faces[f][order[k]];
      cube_tex_indices[n] = (uint16_t)order[k];
      n++;
    }
  }

  mesh.indexes_count = n;
  mesh.pos_indices = cube_pos_indices;
  mesh.tex_indices = cube_tex_indices;
  mesh.positions = cube_positions;
  mesh.textCoord = cube_coords;

  object_init(&object, &mesh, &material);

  Mat4 rotation = mat4RotateY(0.7f);
  Mat4 translation = mat4Translate((Vec3f){0, 0, -2.8f});
  entity_init(&root, (Renderable *)&object,
              mat4MultiplyM(&rotation, &translation));
  renderer_set_root_renderable(renderer, (Renderable *)&root);

  renderer->camera_projection =
      mat4Perspective(1, 50.0, (float)WIDTH / (float)HEIGHT, 0.6);
  renderer->camera_view = mat4Translate((Vec3f){0, 0, 0});
}

static const Scene scenes[] = {
    {"empty", build_empty},
    {"triangle", build_triangle},
    {"cube", build_cube},
};
static const size_t scene_count = sizeof(scenes) / sizeof(scenes[0]);

// ---------------------------------------------------------------------------
// PPM handling. Binary P6, so the references open in any image viewer.
// ---------------------------------------------------------------------------

// Row 0 of the framebuffer is the bottom of the image, so this flips while
// converting - otherwise every reference would open upside down.
static void to_rgb(const Pixel *frame, unsigned char *rgb) {
  for (int i = 0; i < PIXEL_COUNT; i++) {
    const int x = i % WIDTH;
    const int y = i / WIDTH;
    Pixel p = frame[(HEIGHT - 1 - y) * WIDTH + x];
#ifdef PINGO_PIXEL_UINT8
    rgb[i * 3 + 0] = p.g;
    rgb[i * 3 + 1] = p.g;
    rgb[i * 3 + 2] = p.g;
#else
    rgb[i * 3 + 0] = p.r;
    rgb[i * 3 + 1] = p.g;
    rgb[i * 3 + 2] = p.b;
#endif
  }
}

static int write_ppm(const char *path, const unsigned char *rgb) {
  FILE *f = fopen(path, "wb");
  if (f == NULL) {
    fprintf(stderr, "  cannot write %s\n", path);
    return 1;
  }
  fprintf(f, "P6\n%d %d\n255\n", WIDTH, HEIGHT);
  size_t written = fwrite(rgb, 1, PIXEL_COUNT * 3, f);
  if (fclose(f) != 0 || written != PIXEL_COUNT * 3) {
    fprintf(stderr, "  short write to %s\n", path);
    return 1;
  }
  return 0;
}

// Reads a P6 of exactly our dimensions. Deliberately strict: a reference that
// does not match the test's expectations is a broken reference, not a pass.
static int read_ppm(const char *path, unsigned char *rgb) {
  FILE *f = fopen(path, "rb");
  if (f == NULL) {
    fprintf(stderr, "  cannot open reference %s\n", path);
    fprintf(stderr, "  run this executable with --write-references first\n");
    return 1;
  }

  int w = 0, h = 0, maxval = 0;
  if (fscanf(f, "P6 %d %d %d", &w, &h, &maxval) != 3) {
    fprintf(stderr, "  %s is not a binary PPM\n", path);
    fclose(f);
    return 1;
  }
  if (w != WIDTH || h != HEIGHT || maxval != 255) {
    fprintf(stderr, "  %s is %dx%d maxval %d, expected %dx%d maxval 255\n",
            path, w, h, maxval, WIDTH, HEIGHT);
    fclose(f);
    return 1;
  }
  fgetc(f); // the single whitespace byte before the data

  size_t read = fread(rgb, 1, PIXEL_COUNT * 3, f);
  fclose(f);
  if (read != PIXEL_COUNT * 3) {
    fprintf(stderr, "  %s holds %zu bytes of pixel data, expected %d\n", path,
            read, PIXEL_COUNT * 3);
    return 1;
  }
  return 0;
}

// ---------------------------------------------------------------------------

static int render_scene(const Scene *scene, unsigned char *rgb) {
  MemoryBackend backend;
  if (memory_backend_init(&backend, (Vec2i){WIDTH, HEIGHT}) != 0) {
    fprintf(stderr, "  could not allocate the %dx%d buffers\n", WIDTH, HEIGHT);
    return 1;
  }

  Renderer renderer;
  if (renderer_init(&renderer, (Vec2i){WIDTH, HEIGHT},
                    (Backend *)&backend) != OK) {
    fprintf(stderr, "  renderer_init failed\n");
    memory_backend_free(&backend);
    return 1;
  }

  scene->build(&renderer);

  if (renderer_render(&renderer) != OK) {
    fprintf(stderr, "  renderer_render failed\n");
    memory_backend_free(&backend);
    return 1;
  }

  to_rgb(backend.frame, rgb);
  memory_backend_free(&backend);
  return 0;
}

// Returns the number of pixels differing by more than CHANNEL_TOLERANCE, and
// reports the worst offender to make a failure diagnosable.
static int compare(const unsigned char *actual, const unsigned char *expected) {
  int differing = 0;
  int worst = 0;
  int worst_index = -1;

  for (int i = 0; i < PIXEL_COUNT; i++) {
    int delta = 0;
    for (int c = 0; c < 3; c++) {
      int d = actual[i * 3 + c] - expected[i * 3 + c];
      if (d < 0) {
        d = -d;
      }
      if (d > delta) {
        delta = d;
      }
    }
    if (delta > CHANNEL_TOLERANCE) {
      differing++;
      if (delta > worst) {
        worst = delta;
        worst_index = i;
      }
    }
  }

  if (differing > 0) {
    fprintf(stderr,
            "  %d of %d pixels differ (allowed %d); worst delta %d at (%d, %d)\n",
            differing, PIXEL_COUNT, MAX_DIFFERING_PIXELS, worst,
            worst_index % WIDTH, worst_index / WIDTH);
  }
  return differing;
}

int main(int argc, char **argv) {
  const int writing = (argc == 2 && strcmp(argv[1], "--write-references") == 0);
  const char *only = (argc == 2 && !writing) ? argv[1] : NULL;

  if (argc > 2) {
    fprintf(stderr, "usage: %s [--write-references | <scene>]\n", argv[0]);
    return 2;
  }

  unsigned char *actual = malloc(PIXEL_COUNT * 3);
  unsigned char *expected = malloc(PIXEL_COUNT * 3);
  if (actual == NULL || expected == NULL) {
    fprintf(stderr, "out of memory\n");
    free(actual);
    free(expected);
    return 2;
  }

  int failures = 0;
  int ran = 0;

  for (size_t i = 0; i < scene_count; i++) {
    const Scene *scene = &scenes[i];
    if (only != NULL && strcmp(only, scene->name) != 0) {
      continue;
    }
    ran++;

    char reference[512];
    snprintf(reference, sizeof(reference), "%s/%s.ppm", PINGO_REFERENCE_DIR,
             scene->name);

    printf("scene %s\n", scene->name);

    if (render_scene(scene, actual) != 0) {
      failures++;
      continue;
    }

    if (writing) {
      if (write_ppm(reference, actual) != 0) {
        failures++;
      } else {
        printf("  wrote %s\n", reference);
      }
      continue;
    }

    if (read_ppm(reference, expected) != 0) {
      failures++;
      continue;
    }

    const int differing = compare(actual, expected);
    if (differing > MAX_DIFFERING_PIXELS) {
      char actual_path[512];
      snprintf(actual_path, sizeof(actual_path), "%s-actual.ppm", scene->name);
      write_ppm(actual_path, actual);
      fprintf(stderr, "  FAIL: wrote %s for comparison\n", actual_path);
      failures++;
    } else {
      printf("  ok%s\n", differing ? " (within tolerance)" : "");
    }
  }

  free(actual);
  free(expected);

  if (only != NULL && ran == 0) {
    fprintf(stderr, "no scene named '%s'\n", only);
    return 2;
  }

  return failures == 0 ? 0 : 1;
}
