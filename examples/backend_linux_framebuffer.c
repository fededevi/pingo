#include "examples/backends/linux_framebuffer/linux_framebuffer_backend.h"
#include "backend_config.h"

static LinuxFramebufferBackend backend;

Backend * get_backend() {
    Vec2i size = get_rendering_size();
    linux_framebuffer_backend_init(&backend, size, "/dev/fb0");
    return (Backend*)&backend;
}

Vec2i get_rendering_size() {
    return (Vec2i){640, 480};
}
