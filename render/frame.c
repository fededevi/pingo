#include "frame.h"

int frameInit( Frame * f, Vector2I size, const Pixel * buf ) {

    if (size.x * size.y == 0)
        return 1; // 0 sized rect

    if (buf == 0)
        return 2; // null ptr buffer

    f->frameBuffer = (Pixel *)buf;
    f->size = size;

    return 0;
}


Renderable frameAsRenderable(Frame * s) {
    return (Renderable){ .renderableType = RENDERABLE_FRAME, .impl = s};
}

void frameDraw(Frame *f, Vector2I pos, Pixel color)
{
    f->frameBuffer[pos.x + pos.y * f->size.x] = color;
}

Pixel frameRead(Frame *f, Vector2I pos)
{
    return f->frameBuffer[pos.x + pos.y * f->size.x];
}
