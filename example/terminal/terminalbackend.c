#include "terminalbackend.h"

#include "example/common/example_backend.h"

#include "render/depth.h"
#include "render/pixel.h"
#include "render/renderer.h"
#include "render/texture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Vec2i totalSize;
PingoDepth *zetaBuffer;
Pixel *frameBuffer;

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
          pixelToUInt8(&frameBuffer[x + y * totalSize.x]) / 255.0;

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

Pixel *terminal_backend_getFrameBuffer(Renderer *ren, Backend *backend) {
  (void)ren;
  (void)backend;
  return frameBuffer;
}

PingoDepth *terminal_backend_getZetaBuffer(Renderer *ren, Backend *backend) {
  (void)ren;
  (void)backend;
  return zetaBuffer;
}

PgError terminal_backend_init(TerminalBackend *this, Vec2i size) {
  if (this == NULL) {
    return pg_fail(PG_INVALID_ARGUMENT, "backend must not be NULL");
  }
  if (size.x <= 0 || size.y <= 0) {
    return pg_fail(PG_INVALID_ARGUMENT, "backend size must be positive");
  }

  totalSize = size;
  this->backend.init = &terminal_backend_init_backend;
  this->backend.beforeRender = &terminal_backend_beforeRender;
  this->backend.afterRender = &terminal_backend_afterRender;
  this->backend.getFrameBuffer = &terminal_backend_getFrameBuffer;
  this->backend.getZetaBuffer = &terminal_backend_getZetaBuffer;

  const size_t pixels = (size_t)size.x * (size_t)size.y;

  zetaBuffer = malloc(pixels * sizeof(PingoDepth));
  if (zetaBuffer == NULL) {
    return pg_fail(PG_OUT_OF_MEMORY, "allocate depth buffer");
  }

  frameBuffer = malloc(pixels * sizeof(Pixel));
  if (frameBuffer == NULL) {
    return pg_fail(PG_OUT_OF_MEMORY, "allocate frame buffer");
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
  free(zetaBuffer);
  zetaBuffer = NULL;
  free(frameBuffer);
  frameBuffer = NULL;
  free(backend);
}

void backend_sleep(int microseconds) { usleep(microseconds); }
