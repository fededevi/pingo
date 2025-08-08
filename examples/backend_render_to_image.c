#include "linux_window_backend.h"
#include "backend_config.h"

static LinuxWindowBackend backend;

Backend * get_backend() {
    Vec2i size = get_rendering_size();
    linuxWindowBackendInit(&backend, size);
    return (Backend*)&backend;
}

Vec2i get_rendering_size() {
    return (Vec2i){640, 480};
}
