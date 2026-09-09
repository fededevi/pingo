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

#include "golden.h"
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
  checker[0] = pixel_from_rgba(255, 255, 255, 255);
  checker[1] = pixel_from_rgba(60, 60, 60, 255);
  checker[2] = pixel_from_rgba(60, 60, 60, 255);
  checker[3] = pixel_from_rgba(255, 255, 255, 255);
  texture_init(&texture, (Vec2i){2, 2}, checker);
  material_init(&material, &texture);
}

// Nothing drawn: exercises the clear path on its own.
static void build_empty(Renderer *renderer) {
  init_checker_texture();
  mesh.index_count = 0;
  mesh.pos_indices = NULL;
  mesh.tex_indices = NULL;
  mesh.positions = NULL;
  mesh.tex_coords = NULL;
  object_init(&object, &mesh, &material);
  entity_init(&root, (Renderable *)&object, mat4Identity());
  renderer_set_root_renderable(renderer, (Renderable *)&root);

  renderer->camera.projection =
      mat4Perspective(1, 50.0, (float)WIDTH / (float)HEIGHT, 0.6);
  renderer->camera.view = mat4Translate((Vec3f){0, 0, 0});
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

  mesh.index_count = 3;
  mesh.pos_indices = tri_indices;
  mesh.tex_indices = tri_indices;
  mesh.positions = tri_positions;
  mesh.tex_coords = tri_coords;

  object_init(&object, &mesh, &material);
  entity_init(&root, (Renderable *)&object,
              mat4Translate((Vec3f){0, 0, -2.2f}));
  renderer_set_root_renderable(renderer, (Renderable *)&root);

  renderer->camera.projection =
      mat4Perspective(1, 50.0, (float)WIDTH / (float)HEIGHT, 0.6);
  renderer->camera.view = mat4Translate((Vec3f){0, 0, 0});
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

  mesh.index_count = n;
  mesh.pos_indices = cube_pos_indices;
  mesh.tex_indices = cube_tex_indices;
  mesh.positions = cube_positions;
  mesh.tex_coords = cube_coords;

  object_init(&object, &mesh, &material);

  Mat4 rotation = mat4RotateY(0.7f);
  Mat4 translation = mat4Translate((Vec3f){0, 0, -2.8f});
  entity_init(&root, (Renderable *)&object,
              mat4MultiplyM(&rotation, &translation));
  renderer_set_root_renderable(renderer, (Renderable *)&root);

  renderer->camera.projection =
      mat4Perspective(1, 50.0, (float)WIDTH / (float)HEIGHT, 0.6);
  renderer->camera.view = mat4Translate((Vec3f){0, 0, 0});
}

// A mesh carrying no texture coordinates, which several shipped assets do.
// object_render used to dereference tex_indices whenever a material was set,
// so any such mesh crashed the renderer rather than drawing untextured.
static void build_no_uv(Renderer *renderer) {
  build_triangle(renderer);
  mesh.tex_coords = NULL;
  mesh.tex_indices = NULL;
}


static Vec3f tri2_positions[3];
static uint16_t tri2_indices[3];
static Mesh mesh2;
static Object object2;
static Entity child_a;
static Entity child_b;
static Renderable *group_children[2];

// A group with no content of its own, two children reached through the
// multi-child path, and a non-black clear colour. Each of those was
// unreachable in the other scenes: entity_init's single-child shortcut covers
// content, and every scene cleared to black.
static void build_group(Renderer *renderer) {
  init_checker_texture();

  tri_positions[0] = (Vec3f){-1.0f, -0.8f, 0.0f};
  tri_positions[1] = (Vec3f){0.0f, -0.8f, 0.0f};
  tri_positions[2] = (Vec3f){-0.5f, 0.6f, 0.0f};
  tri_coords[0] = (Vec2f){0.0f, 0.0f};
  tri_coords[1] = (Vec2f){1.0f, 0.0f};
  tri_coords[2] = (Vec2f){0.5f, 1.0f};
  tri_indices[0] = 0;
  tri_indices[1] = 1;
  tri_indices[2] = 2;
  mesh.index_count = 3;
  mesh.pos_indices = tri_indices;
  mesh.tex_indices = tri_indices;
  mesh.positions = tri_positions;
  mesh.tex_coords = tri_coords;
  object_init(&object, &mesh, &material);

  tri2_positions[0] = (Vec3f){0.2f, -0.6f, 0.0f};
  tri2_positions[1] = (Vec3f){1.0f, -0.6f, 0.0f};
  tri2_positions[2] = (Vec3f){0.6f, 0.5f, 0.0f};
  tri2_indices[0] = 0;
  tri2_indices[1] = 1;
  tri2_indices[2] = 2;
  mesh2.index_count = 3;
  mesh2.pos_indices = tri2_indices;
  mesh2.tex_indices = tri2_indices;
  mesh2.positions = tri2_positions;
  mesh2.tex_coords = tri_coords;
  object_init(&object2, &mesh2, &material);

  entity_init(&child_a, (Renderable *)&object, mat4Translate((Vec3f){0, 0, -2.5f}));
  entity_init(&child_b, (Renderable *)&object2, mat4Translate((Vec3f){0, 0, -2.5f}));
  group_children[0] = (Renderable *)&child_a;
  group_children[1] = (Renderable *)&child_b;

  entity_init_children(&root, mat4Identity(), group_children, 2);
  renderer_set_root_renderable(renderer, (Renderable *)&root);

  renderer->clear = true;
  renderer->clear_color = pixel_from_rgba(0, 40, 80, 255);

  renderer->camera.projection =
      mat4Perspective(1, 50.0, (float)WIDTH / (float)HEIGHT, 0.6);
  renderer->camera.view = mat4Translate((Vec3f){0, 0, 0});
}

static const Scene scenes[] = {
    {"empty", build_empty},
    {"triangle", build_triangle},
    {"cube", build_cube},
    {"no-uv", build_no_uv},
    {"group", build_group},
};
static const size_t scene_count = sizeof(scenes) / sizeof(scenes[0]);

// The caller owns the backend so the framebuffer outlives the comparison.
static int render_scene(const Scene *scene, MemoryBackend *backend) {
  if (memory_backend_init(backend, (Vec2i){WIDTH, HEIGHT}) != 0) {
    fprintf(stderr, "  could not allocate the %dx%d buffers\n", WIDTH, HEIGHT);
    return 1;
  }

  Renderer renderer;
  if (renderer_init(&renderer, (Vec2i){WIDTH, HEIGHT},
                    (Backend *)backend) != OK) {
    fprintf(stderr, "  renderer_init failed\n");
    return 1;
  }

  scene->build(&renderer);

  if (renderer_render(&renderer) != OK) {
    fprintf(stderr, "  renderer_render failed\n");
    return 1;
  }

  return 0;
}

int main(int argc, char **argv) {
  const int writing = (argc == 2 && strcmp(argv[1], "--write-references") == 0);
  const char *only = (argc == 2 && !writing) ? argv[1] : NULL;

  if (argc > 2) {
    fprintf(stderr, "usage: %s [--write-references | <scene>]\n", argv[0]);
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

    MemoryBackend backend;
    if (render_scene(scene, &backend) != 0) {
      memory_backend_free(&backend);
      failures++;
      continue;
    }

    if (writing) {
      if (golden_write(reference, backend.frame, WIDTH, HEIGHT) != GOLDEN_OK) {
        failures++;
      } else {
        printf("  wrote %s\n", reference);
      }
      memory_backend_free(&backend);
      continue;
    }

    // The empty scene is deliberately blank, so it is the one case where a
    // blank render is the expected result rather than a framing mistake.
    const int require_content = strcmp(scene->name, "empty") != 0;
    const GoldenResult result = golden_compare(reference, backend.frame, WIDTH,
                                               HEIGHT, require_content);
    if (result == GOLDEN_OK) {
      printf("  ok\n");
    } else {
      if (result == GOLDEN_MISMATCH) {
        golden_write_actual(scene->name, backend.frame, WIDTH, HEIGHT);
      }
      failures++;
    }
    memory_backend_free(&backend);
  }

  if (only != NULL && ran == 0) {
    fprintf(stderr, "no scene named '%s'\n", only);
    return 2;
  }

  return failures == 0 ? 0 : 1;
}
