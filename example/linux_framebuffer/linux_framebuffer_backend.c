#include "linux_framebuffer_backend.h"
#include "render/state.h"
#include "render/target.h"

#include "example/common/example_backend.h"
// Needed for the complete types: render/fwd.h only forward-declares
// these, and sizeof requires the definitions.
#include "render/depth.h"
#include "render/pixel.h"

#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FRAMEBUFFER_DEVICE "/dev/fb0"

static Vec2i totalSize;
static PingoDepth *depth_buffer;
static Pixel *frame_buffer; // the mmap'd framebuffer
static RenderTarget target;
static Pixel *renderBuffer;
static size_t mappedBytes;
static int framebufferFd = -1;

void init(Renderer *ren, Backend *backend) {
  (void)ren;
  (void)backend;

  // Deliberately ignores _rect. renderer_init passes {0, 0, 0, 0}, and this
  // used to size renderBuffer from it - malloc(0), which renderer_render then
  // memset 1.2 MB into. Everything is sized from the size given to
  // create_backend instead, and allocated there so failures can be reported.
}

void before_render(Renderer *ren, Backend *backend) {
  (void)ren;
  (void)backend;
}

void after_render(Renderer *ren, Backend *backend) {
  (void)ren;
  (void)backend;

  memcpy(frame_buffer, renderBuffer,
         (size_t)totalSize.x * (size_t)totalSize.y * sizeof(Pixel));
}

static RenderTarget *lfb_get_target(Renderer *ren, Backend *backend) {
  (void)ren;
  (void)backend;
  return &target;
}

// Opens the framebuffer and checks it is laid out the way after_render assumes.
static PgError map_framebuffer(Vec2i size) {
  framebufferFd = open(FRAMEBUFFER_DEVICE, O_RDWR);
  if (framebufferFd < 0) {
    PgError error = pg_fail_errno(PG_IO, "open " FRAMEBUFFER_DEVICE);
    if (errno == EACCES) {
      error.status = PG_PERMISSION_DENIED;
      error.hint = "Your user is not in the 'video' group. Run "
                   "'sudo usermod -aG video $USER', then log out and back in.";
    } else if (errno == ENOENT) {
      error.status = PG_NOT_FOUND;
      error.hint = "No framebuffer device. This backend needs a Linux "
                   "framebuffer; try the linux_window or linux_terminal "
                   "example instead.";
    }
    return error;
  }

  struct fb_var_screeninfo vinfo;
  struct fb_fix_screeninfo finfo;
  if (ioctl(framebufferFd, FBIOGET_VSCREENINFO, &vinfo) < 0 ||
      ioctl(framebufferFd, FBIOGET_FSCREENINFO, &finfo) < 0) {
    return pg_fail_errno(PG_IO, "query " FRAMEBUFFER_DEVICE " geometry");
  }

  if (vinfo.bits_per_pixel != 32) {
    return pg_fail_hint(PG_UNSUPPORTED, FRAMEBUFFER_DEVICE " is not 32 bpp",
                        "This backend writes 32-bit pixels. Reconfigure the "
                        "framebuffer, or use another example.");
  }

  if ((unsigned)size.x > vinfo.xres || (unsigned)size.y > vinfo.yres) {
    return pg_fail_hint(PG_INVALID_ARGUMENT,
                        "requested size is larger than " FRAMEBUFFER_DEVICE,
                        "Ask for a size that fits the screen.");
  }

  // after_render blits the render buffer in one memcpy, which is only correct
  // when the framebuffer rows are exactly as wide as ours. Rather than
  // silently producing a skewed image, say so.
  if (finfo.line_length != (unsigned)size.x * sizeof(Pixel)) {
    return pg_fail_hint(PG_UNSUPPORTED,
                        "framebuffer stride does not match the requested width",
                        "This backend needs the render width to equal the "
                        "framebuffer width. Try the screen's native width.");
  }

  mappedBytes = (size_t)size.x * (size_t)size.y * sizeof(Pixel);
  frame_buffer = mmap(NULL, mappedBytes, PROT_READ | PROT_WRITE, MAP_SHARED,
                     framebufferFd, 0);
  if (frame_buffer == MAP_FAILED) {
    frame_buffer = NULL;
    return pg_fail_errno(PG_IO, "mmap " FRAMEBUFFER_DEVICE);
  }

  return PG_SUCCESS;
}

PgError linux_framebuffer_backend_init(LinuxFramebufferBackend *this,
                                       Vec2i size) {
  if (this == NULL) {
    return pg_fail(PG_INVALID_ARGUMENT, "backend must not be NULL");
  }
  if (size.x <= 0 || size.y <= 0) {
    return pg_fail(PG_INVALID_ARGUMENT, "backend size must be positive");
  }

  totalSize = size;
  this->backend.init = &init;
  this->backend.before_render = &before_render;
  this->backend.after_render = &after_render;
  this->backend.get_target = &lfb_get_target;

  const size_t pixels = (size_t)size.x * (size_t)size.y;

  depth_buffer = malloc(pixels * sizeof(PingoDepth));
  if (depth_buffer == NULL) {
    return pg_fail(PG_OUT_OF_MEMORY, "allocate depth buffer");
  }

  renderBuffer = malloc(pixels * sizeof(Pixel));
  if (renderBuffer == NULL) {
    return pg_fail(PG_OUT_OF_MEMORY, "allocate render buffer");
  }

  RETURN_IF_ERROR(map_framebuffer(size));

  if (render_target_init(&target, size, frame_buffer, depth_buffer) != OK) {
    return pg_fail(PG_INVALID_ARGUMENT, "colour and depth buffers");
  }

  return PG_SUCCESS;
}

PgError create_backend(Vec2i size, Backend **out) {
  if (out == NULL) {
    return pg_fail(PG_INVALID_ARGUMENT, "out must not be NULL");
  }

  LinuxFramebufferBackend *lfb = malloc(sizeof(LinuxFramebufferBackend));
  if (lfb == NULL) {
    return pg_fail(PG_OUT_OF_MEMORY, "allocate framebuffer backend");
  }

  PgError error = linux_framebuffer_backend_init(lfb, size);
  if (pg_failed(error)) {
    destroy_backend((Backend *)lfb);
    return error;
  }

  *out = (Backend *)lfb;
  return PG_SUCCESS;
}

void destroy_backend(Backend *backend) {
  if (frame_buffer != NULL) {
    munmap(frame_buffer, mappedBytes);
    frame_buffer = NULL;
  }
  if (framebufferFd >= 0) {
    close(framebufferFd);
    framebufferFd = -1;
  }

  free(depth_buffer);
  depth_buffer = NULL;
  free(renderBuffer);
  renderBuffer = NULL;
  free(backend);
}

void backend_sleep(int microseconds) { usleep(microseconds); }
