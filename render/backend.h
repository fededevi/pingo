#pragma once

/**
  * Privdes a common interface to multiple graphical backends
  */

typedef struct Renderer Renderer;
typedef struct Pixel Pixel;

typedef struct BackEnd {
    //Called on initialization and re-initialization
    void (*init)(Renderer *, struct BackEnd * );

    //Called before starting rendering
    void (*beforeRender)(Renderer *, struct BackEnd * );

    //Called after having finished a render
    void (*afterRender)(Renderer *, struct BackEnd * );

    //Should return the address of the buffer (height*width*sizeof(Pixel))
    Pixel * (*getFrameBuffer)(Renderer *, struct BackEnd * );
} BackEnd;
