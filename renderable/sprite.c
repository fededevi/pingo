#include "sprite.h"
#include <stdlib.h>

int spriteInit(Sprite *s, Frame f, Vector2I position)
{
    if (f.frameBuffer == 0)
        return 1;

    s->frame = f;
    s->position = position;

    return 0;
}

Renderable spriteAsRenderable(Sprite * s) {
    return (Renderable){ .renderableType = RENDERABLE_SPRITE, .impl = s};
}

int spriteRandomize(Sprite * s)
{

    for (int x = 0; x < s->frame.size.x; x++ ) {
        for (int y = 0; y < s->frame.size.y; y++ ) {
            frameDraw(&s->frame,(Vector2I){x,y},(Pixel){rand(),rand(),rand()});
        }
    }

    return 0;
}
