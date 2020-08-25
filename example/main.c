#include "windowbackend.h"
#include "../render/renderer.h"
#include "../render/frame.h"

int main(){
    Vec2i pos = {0,0};
    Vec2i size = {800,600};

    WindowBackEnd backend;
    windowBackEndInit(&backend, pos, size);

    Renderer renderer;
    rendererInit(&renderer, size,(BackEnd*) &backend );

    while (1) {
        rendererRender(&renderer);
    }

    return 0;
}

