#include "terminalbackend.h"
#include "render/state.h"
#include "render/target.h"

#include "example/common/example_backend.h"

#include "render/depth.h"
#include "render/pixel.h"
#include "render/renderer.h"
#include "render/texture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

Vec2i totalSize;
static PingoDepth *depth_buffer;
static Pixel *frame_buffer;
static RenderTarget target;

// This backend writes ANSI escapes and one UTF-8 character. A Windows console
// interprets neither by default, so both have to be switched on; on every
// other platform they already work.
static void enable_ansi_output(void) {
#ifdef _WIN32
  HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD mode = 0;
  if (out != INVALID_HANDLE_VALUE && GetConsoleMode(out, &mode)) {
    SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
  }
  SetConsoleOutputCP(CP_UTF8);
#endif
}

void terminal_backend_init_backend(Renderer *ren, Backend *backend,
                                   Vec4i rect) {
  (void)ren;
  (void)backend;
  (void)rect;
}

void terminal_backend_beforeRender(Renderer *ren, Backend *backend) {
  (void)ren;
  (void)backend;

  // clear and move cursor to start point
  printf("\033[2J");
}

void terminal_backend_afterRender(Renderer *ren, Backend *backend) {
  (void)ren;
  (void)backend;

  // Grayscale-to-ASCII mapping scale (from dark to bright)
  const char scale[] = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/"
                       "*tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$█";
  int charSize = strlen(scale);

  // Optional: Move cursor to top-left instead of clearing screen
  printf("\033[H");

  for (int y = totalSize.y - 1; y >= 0; y--) {
    for (int x = 0; x < totalSize.x; x++) {
      // Normalize pixel intensity to [0, 1]
      double normalValue =
          pixel_to_uint8(&frame_buffer[x + y * totalSize.x]) / 255.0;

      // Apply contrast curve
      // normalValue = (normalValue + 0.1) / 1.2;

      // Map to character index
      int index = (int)(normalValue * (charSize - 1) + 0.5); // round

      // Clamp index to valid range
      if (index < 0)
        index = 0;
      if (index >= charSize)
        index = charSize - 1;

      // Output character directly
      putchar(scale[index]);
      putchar(scale[index]);
    }
    putchar('\n');
  }
}

static RenderTarget *terminal_backend_get_target(Renderer *ren, Backend *backend) {
  (void)ren;
  (void)backend;
  return &target;
}

PgError terminal_backend_init(TerminalBackend *this, Vec2i size) {
  if (this == NULL) {
    return pg_fail(PG_INVALID_ARGUMENT, "backend must not be NULL");
  }
  if (size.x <= 0 || size.y <= 0) {
    return pg_fail(PG_INVALID_ARGUMENT, "backend size must be positive");
  }

  enable_ansi_output();

  totalSize = size;
  this->backend.init = &terminal_backend_init_backend;
  this->backend.before_render = &terminal_backend_beforeRender;
  this->backend.after_render = &terminal_backend_afterRender;
  this->backend.get_target = &terminal_backend_get_target;

  const size_t pixels = (size_t)size.x * (size_t)size.y;

  depth_buffer = malloc(pixels * sizeof(PingoDepth));
  if (depth_buffer == NULL) {
    return pg_fail(PG_OUT_OF_MEMORY, "allocate depth buffer");
  }

  frame_buffer = malloc(pixels * sizeof(Pixel));
  if (frame_buffer == NULL) {
    return pg_fail(PG_OUT_OF_MEMORY, "allocate frame buffer");
  }

  if (render_target_init(&target, size, frame_buffer, depth_buffer) != OK) {
    return pg_fail(PG_INVALID_ARGUMENT, "colour and depth buffers");
  }

  return PG_SUCCESS;
}

PgError create_backend(Vec2i size, Backend **out) {
  if (out == NULL) {
    return pg_fail(PG_INVALID_ARGUMENT, "out must not be NULL");
  }

  TerminalBackend *termBackend = malloc(sizeof(TerminalBackend));
  if (termBackend == NULL) {
    return pg_fail(PG_OUT_OF_MEMORY, "allocate terminal backend");
  }

  PgError error = terminal_backend_init(termBackend, size);
  if (pg_failed(error)) {
    destroy_backend((Backend *)termBackend);
    return error;
  }

  *out = (Backend *)termBackend;
  return PG_SUCCESS;
}

void destroy_backend(Backend *backend) {
  free(depth_buffer);
  depth_buffer = NULL;
  free(frame_buffer);
  frame_buffer = NULL;
  free(backend);
}

void backend_sleep(int microseconds) {
#ifdef _WIN32
  Sleep((DWORD)(microseconds / 1000)); // Sleep takes milliseconds
#else
  usleep(microseconds);
#endif
}
