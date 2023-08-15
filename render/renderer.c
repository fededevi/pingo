#include <string.h>
#include <stdio.h>
#include "math/mat4.h"
#include "render/state.h"
#include "renderer.h"
#include "pixel.h"
#include "depth.h"
#include "backend.h"

int renderer_init(Renderer * r, Vec2i size, Backend * backend) {
    r->root_renderable = 0;
    r->clear = 1;
    r->clear_color = PIXELBLACK;
    r->backend = backend;
    r->backend->init(r, r->backend, (Vec4i) { 0, 0, 0, 0 });

    int e = 0;
    e = texture_init( &r->framebuffer, size, backend->getFrameBuffer(r, backend));
    if (e) return e;

    return 0;
}



int renderer_render(Renderer *r)
{
    Backend *be = r->backend;

    int pixels = r->framebuffer.size.x * r->framebuffer.size.y;
    memset(be->getZetaBuffer(r,be), 0, pixels * sizeof (PingoDepth));

    be->beforeRender(r, be);

    //get current framebuffe from Backend
    r->framebuffer.frameBuffer = be->getFrameBuffer(r, be);

    //Clear draw buffer before rendering
    if (r->clear) {
        memset(be->getFrameBuffer(r,be), 0, pixels * sizeof (Pixel));
    }

    r->root_renderable->render(r->root_renderable, mat4Identity(), r);

    be->afterRender(r, be);

    return 0;
}

int renderer_set_root_renderable(Renderer *renderer, Renderable *root)
{
    IF_NULL_RETURN(renderer, SET_ERROR);
    IF_NULL_RETURN(root, SET_ERROR);

    renderer->root_renderable = root;
    return 0;
}
