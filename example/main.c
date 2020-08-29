#include "windowbackend.h"
#include "../render/renderer.h"
#include "../render/texture.h"
#include "../render/sprite.h"
#include "../render/scene.h"
#include "../math/mat3.h"
#include <math.h>
#include <string.h>

#define SIZEW 1366
#define SIZEH 768

int main(){
    Vec2i size = {SIZEW,SIZEH};

    WindowBackEnd backend;
    windowBackEndInit(&backend);

    Renderer renderer;
    rendererInit(&renderer, size,(BackEnd*) &backend );

    Texture frame;
    texture_init(&frame, (Vec2i){16,16}, malloc(16*16*sizeof(Pixel)));

    for (int i = 0; i < 16; i++)
        for (int y = 0; y < 16; y++)
            ((uint8_t *)frame.frameBuffer)[i * 16+ y ] = (i + y) % 2 == 0 ? 0x22 : 0xBB;

    Sprite sprite;
    spriteInit(&sprite,frame,mat3Rotate(45));

    Scene s;
    sceneInit(&s);
    sceneAddRenderable(&s, spriteAsRenderable(&sprite));

    rendererSetScene(&renderer, &s);

    float phi = 0;
    Mat3 t;
    while (1) {
        sprite.t = mat3Translate((Vec2f){-7.5,-8});

        t = mat3Rotate(phi += 0.001);
        sprite.t = mat3MultiplyM(&sprite.t, &t );

        t = mat3Scale((Vec2f){20,20});
        sprite.t = mat3MultiplyM(&sprite.t,&t);

        t = mat3Translate((Vec2f){400,300});
        sprite.t = mat3MultiplyM(&sprite.t,&t);

        rendererSetCamera(&renderer,(Vec4i){0,0,SIZEW,SIZEH});
        rendererRender(&renderer);
    }

    return 0;
}

