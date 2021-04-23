
#include "ttgobackend.h"

#include "../render/renderer.h"
#include "../render/texture.h"
#include "../render/pixel.h"
#include "../render/depth.h"

void _ttgoBackendInit( Renderer * ren, BackEnd * backEnd, Vec4i _rect) {

}

void _ttgoBackendBeforeRender( Renderer * ren, BackEnd * backEnd) {
}

void _ttgoBackendAfterRender( Renderer * ren,  BackEnd * backEnd) {

}

Pixel * _ttgoBackendGetFrameBuffer( Renderer * ren,  BackEnd * backEnd) {
    return ((TTGOBackend *) backEnd) -> frameBuffer;
}

Depth * _ttgoBackendGetZetaBuffer( Renderer * ren,  BackEnd * backEnd) {
    return ((TTGOBackend *) backEnd) -> zetaBuffer;
}

void ttgoBackendInit( TTGOBackend * this, Pixel * buf, Vec2i size) {

    this->backend.init = &_ttgoBackendInit;
    this->backend.beforeRender = &_ttgoBackendBeforeRender;
    this->backend.afterRender = &_ttgoBackendAfterRender;
    this->backend.getFrameBuffer = &_ttgoBackendGetFrameBuffer;
    this->backend.getZetaBuffer = &_ttgoBackendGetZetaBuffer;

    this -> zetaBuffer = malloc(size.x*size.y*sizeof (Depth));
    this -> frameBuffer = buf;
}
