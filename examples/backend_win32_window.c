#include "win32_window_backend.h"
#include "backend_config.h"

static Win32WindowBackend backend;

Backend * get_backend() {
    Vec2i size = get_rendering_size();
    back(&backend, size);
    return (Backend*)&backend;
}

Vec2i get_rendering_size() {
    return (Vec2i){800, 600};
}