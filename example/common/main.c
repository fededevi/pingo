#include "example_backend.h"

#include "assets/viking.h"
#include "math/mat4.h"
#include "render/backend.h"

#include "render/transform.h"
#include "render/material.h"
#include "render/object.h"
#include "render/pixel.h"
#include "render/renderer.h"
#include "render/state.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEXTURE_PATH "assets/viking.rgba"
#define TEXTURE_SIZE 1024

// Turns the enum State returned by the render API into something printable.
static const char *state_name(int state) {
  switch (state) {
  case OK:
    return "OK";
  case INIT_ERROR:
    return "INIT_ERROR";
  case RENDER_ERROR:
    return "RENDER_ERROR";
  case SET_ERROR:
    return "SET_ERROR";
  default:
    break;
  }
  return "unrecognized state";
}

// Every setup call below returns enum State and none of them are optional, so
// a failure names the call that failed and stops rather than pressing on with
// a half-built scene.
#define CHECK(call)                                                            \
  do {                                                                         \
    int check_state = (call);                                                  \
    if (check_state != OK) {                                                   \
      fprintf(stderr, "error: %s failed with %s (%d)\n  at %s:%d\n", #call,    \
              state_name(check_state), check_state, __FILE__, __LINE__);       \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)

// Loads a raw RGBA image into *out. *out is only written on success.
static PgError load_texture(const char *filename, Vec2i size, Pixel **out) {
  if (out == NULL) {
    return pg_fail(PG_INVALID_ARGUMENT, "out must not be NULL");
  }
  if (size.x <= 0 || size.y <= 0) {
    return pg_fail(PG_INVALID_ARGUMENT, "texture size must be positive");
  }

  const size_t pixel_count = (size_t)size.x * (size_t)size.y;

  Pixel *image = malloc(pixel_count * sizeof(Pixel));
  if (image == NULL) {
    return pg_fail(PG_OUT_OF_MEMORY, "allocate the texture");
  }

  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    PgError error = pg_fail_errno(PG_IO, "open the texture file");
    if (errno == ENOENT) {
      error.status = PG_NOT_FOUND;
      error.hint = "The path is relative to the working directory. Run the "
                   "example from the build directory, where CMake places a "
                   "copy of assets/.";
    }
    free(image);
    return error;
  }

  // The file stores rows top-to-bottom and the texture wants them the other
  // way up, hence counting y down. Note y >= 0: an earlier bound stopped at 1
  // and left row 0 as uninitialized malloc memory.
  for (int y = size.y - 1; y >= 0; y--) {
    for (int x = 0; x < size.x; x++) {
      unsigned char rgba[4];
      if (fread(rgba, 1, sizeof(rgba), file) != sizeof(rgba)) {
        PgError error = ferror(file)
                            ? pg_fail_errno(PG_IO, "read the texture file")
                            : pg_fail_hint(PG_IO, "read the texture file",
                                           "The file is shorter than the "
                                           "declared image size.");
        fclose(file);
        free(image);
        return error;
      }
      image[y * size.x + x] = pixel_from_rgba(rgba[0], rgba[1], rgba[2], rgba[3]);
    }
  }

  fclose(file);
  *out = image;
  return PG_SUCCESS;
}

int main(void) {
  // main returns int, so it cannot use RETURN_IF_ERROR: it reports and exits.
  Pixel *image = NULL;
  PgError error =
      load_texture(TEXTURE_PATH, (Vec2i){TEXTURE_SIZE, TEXTURE_SIZE}, &image);
  if (pg_failed(error)) {
    pg_error_report(error, stderr);
    return EXIT_FAILURE;
  }

  Texture texture;
  CHECK(texture_init(&texture, (Vec2i){TEXTURE_SIZE, TEXTURE_SIZE}, image));

  Material material;
  CHECK(material_init(&material, &texture));

  Object object;
  CHECK(object_init(&object, &viking_mesh, &material));

  Transform root_node;
  CHECK(transform_init(&root_node, (Renderable *)&object, mat4Identity()));

  // Backend-specific initialization
  Vec2i size = {640, 480};
  Backend *backend = NULL;
  error = create_backend(size, &backend);
  if (pg_failed(error)) {
    pg_error_report(error, stderr);
    free(image);
    return EXIT_FAILURE;
  }

  Renderer renderer;
  CHECK(renderer_init(&renderer, size, backend));
  CHECK(renderer_set_root_renderable(&renderer, (Renderable *)&root_node));

  float phi = 0;

  // Default camera setup (can be overridden by backend if needed)
  renderer.camera.projection =
      mat4Perspective(3, 50.0, (float)size.x / (float)size.y, 0.1);
  renderer.camera.view = mat4Translate((Vec3f){0, 0, 0});

  int exit_code = EXIT_SUCCESS;

  while (1) {
    // Rotation around Y-axis
    Mat4 rotation = mat4RotateY(phi);

    // Move object back so it's visible
    Mat4 translation = mat4Translate((Vec3f){0, -7, -50});

    // Combine transforms: T * R
    Mat4 model = mat4MultiplyM(&rotation, &translation);

    root_node.local = model;

    // Leaving the loop on failure means the cleanup below actually runs,
    // instead of spinning silently on a broken renderer.
    int state = renderer_render(&renderer);
    if (state != OK) {
      fprintf(stderr, "error: renderer_render failed with %s (%d)\n",
              state_name(state), state);
      exit_code = EXIT_FAILURE;
      break;
    }

    phi += 0.01f;
    backend_sleep(16000); // ~60 FPS - backend-specific sleep
  }

  // Clean up
  destroy_backend(backend);
  free(image);
  return exit_code;
}
