#include "linux_framebuffer_backend.h"

#include "render/depth.h"
#include "render/pixel.h"
#include "render/renderer.h"
#include "render/texture.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h> // Added for malloc and free
#include <string.h> // Added for memcpy

Vec4i rect;
Vec2i totalSize;
PingoDepth * zetaBuffer;
Pixel * frameBuffer;
Pixel * renderBuffer; // New buffer for rendering

void init(Renderer *ren, Backend *backend, Vec4i _rect) {
    (void)ren;
    (void)backend;

    rect = _rect;

    // Allocate memory for the render buffer
    renderBuffer = (Pixel *)malloc(_rect.z * _rect.w * sizeof(Pixel));
    if (renderBuffer == NULL) {
        perror("Failed to allocate memory for render buffer");
        exit(EXIT_FAILURE);
    }
}

void beforeRender(Renderer *ren, Backend *backend) {
    (void)ren;
    (void)backend;
}

void afterRender(Renderer *ren, Backend *backend) {
    (void)ren;
    (void)backend;

    // Copy the contents of the render buffer to the framebuffer
    int bufferSize = rect.z * rect.w * sizeof(Pixel);
    memcpy(frameBuffer + rect.x + totalSize.x * rect.y, renderBuffer, bufferSize);
}

Pixel *getFrameBuffer(Renderer *ren, Backend *backend) {
    (void)ren;
    (void)backend;

    return renderBuffer; // Return the render buffer for rendering
}

PingoDepth *getZetaBuffer(Renderer *ren, Backend *backend) {
    (void)ren;
    (void)backend;

    return zetaBuffer;
}

void linux_framebuffer_backend_init(LinuxFramebufferBackend *this, Vec2i size, const char *framebufferDevice) {
    totalSize = size;
    this->backend.init = &init;
    this->backend.beforeRender = &beforeRender;
    this->backend.afterRender = &afterRender;
    this->backend.getFrameBuffer = &getFrameBuffer;
    this->backend.getZetaBuffer = &getZetaBuffer;

    zetaBuffer = malloc(size.x * size.y * sizeof(PingoDepth));
    int fdScreen = open(framebufferDevice, O_RDWR);
    frameBuffer = mmap(0, size.x * size.y * 4, PROT_READ | PROT_WRITE, MAP_SHARED, fdScreen, 0);
}
