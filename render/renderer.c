#include <string.h>
#include <stdio.h>
#include "math/mat4.h"
#include "render/state.h"
#include "renderer.h"
#include "pixel.h"
#include "depth.h"
#include "backend.h"

int rendererInit(Renderer * r, Vec2i size, BackEnd * backEnd) {
    r->root_renderable = 0;
    r->clear = 1;
    r->clearColor = PIXELBLACK;
    r->backEnd = backEnd;

    r->backEnd->init(r, r->backEnd, (Vec4i) { 0, 0, 0, 0 });

    int e = 0;
    e = texture_init( & (r->frameBuffer), size, backEnd->getFrameBuffer(r, backEnd));
    if (e) return e;

    return 0;
}



int rendererRender(Renderer * r) {

    int pixels = r->frameBuffer.size.x * r->frameBuffer.size.y;
    memset(r->backEnd->getZetaBuffer(r,r->backEnd), 0, pixels * sizeof (PingoDepth));

    r->backEnd->beforeRender(r, r->backEnd);

    //get current framebuffe from backend
    r->frameBuffer.frameBuffer = r->backEnd->getFrameBuffer(r, r->backEnd);

    //Clear draw buffer before rendering
    if (r->clear) {
        memset(r->backEnd->getFrameBuffer(r,r->backEnd), 0, pixels * sizeof (Pixel));
    }

    r->root_renderable->render(r->root_renderable, mat4Identity(), r);

    r->backEnd->afterRender(r, r->backEnd);

    return 0;
}

int renderer_set_root_renderable(Renderer *renderer, Renderable *root)
{
    IF_NULL_RETURN(renderer, SET_ERROR);
    IF_NULL_RETURN(root, SET_ERROR);

    renderer->root_renderable = root;
    return 0;
}

int rendererSetCamera(Renderer * r, Vec4i rect) {
    r->camera = rect;
    r->backEnd->init(r, r->backEnd, rect);
    r->frameBuffer.size = (Vec2i) {
            rect.z, rect.w
};
    return 0;
}
