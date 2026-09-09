/**
 * Golden-image tests for the shipped meshes.
 *
 * The renderer's own tests use geometry defined inline, so nothing exercised
 * the meshes in this directory - and three of the four could not be rendered
 * at all, because they carry no texture coordinates and object_render
 * dereferenced those unconditionally. These render each one through the
 * memory backend, which needs no window, no libjpeg and no X11, so they run
 * on every platform CI builds for.
 *
 * Run with --write-references to regenerate after a deliberate change.
 */

#include "assets/cube.h"
#include "assets/pingo.h"
#include "assets/teapot.h"
#include "assets/viking.h"

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
  Mesh *mesh;
  float distance;
} AssetScene;

// Distances chosen so each mesh fills a similar part of the frame. All four
// are centred on the origin and unit-scaled.
static const AssetScene scenes[] = {
    {"cube", &mesh_cube, 3.0f},
    {"teapot", &mesh_teapot, 3.2f},
    {"viking", &viking_mesh, 34.0f},
    {"pingo", &pingo_mesh, 2.6f},
};
static const size_t scene_count = sizeof(scenes) / sizeof(scenes[0]);

static Pixel checker[4];
static Texture texture;
static Material material;

// The caller owns the backend so the framebuffer outlives the comparison.
static int render_scene(const AssetScene *scene, MemoryBackend *backend) {
  if (memory_backend_init(backend, (Vec2i){WIDTH, HEIGHT}) != 0) {
    fprintf(stderr, "  could not allocate the buffers\n");
    return 1;
  }

  Renderer renderer;
  if (renderer_init(&renderer, (Vec2i){WIDTH, HEIGHT},
                    (Backend *)backend) != OK) {
    fprintf(stderr, "  renderer_init failed\n");
    return 1;
  }

  Object object;
  object_init(&object, scene->mesh, &material);

  Entity root;
  Mat4 rotation = mat4RotateY(0.6f);
  Mat4 translation = mat4Translate((Vec3f){0, 0, -scene->distance});
  entity_init(&root, (Renderable *)&object,
              mat4MultiplyM(&rotation, &translation));
  renderer_set_root_renderable(&renderer, (Renderable *)&root);

  renderer.camera.projection =
      mat4Perspective(1, 200.0, (float)WIDTH / (float)HEIGHT, 0.6);
  renderer.camera.view = mat4Translate((Vec3f){0, 0, 0});

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
    fprintf(stderr, "usage: %s [--write-references | <mesh>]\n", argv[0]);
    return 2;
  }

  checker[0] = pixel_from_rgba(255, 255, 255, 255);
  checker[1] = pixel_from_rgba(60, 60, 60, 255);
  checker[2] = pixel_from_rgba(60, 60, 60, 255);
  checker[3] = pixel_from_rgba(255, 255, 255, 255);
  texture_init(&texture, (Vec2i){2, 2}, checker);
  material_init(&material, &texture);

  int failures = 0, ran = 0;
  for (size_t i = 0; i < scene_count; i++) {
    const AssetScene *scene = &scenes[i];
    if (only != NULL && strcmp(only, scene->name) != 0) {
      continue;
    }
    ran++;

    char reference[512];
    snprintf(reference, sizeof(reference), "%s/%s.ppm", PINGO_REFERENCE_DIR,
             scene->name);
    printf("mesh %s\n", scene->name);

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

    // Every shipped mesh should be visible, so a blank render is a failure
    // rather than something to compare against an equally blank reference.
    const GoldenResult result =
        golden_compare(reference, backend.frame, WIDTH, HEIGHT, 1);
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
    fprintf(stderr, "no mesh named '%s'\n", only);
    return 2;
  }
  return failures == 0 ? 0 : 1;
}
