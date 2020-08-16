#import "scene.h"

int sceneAddRenderable(Scene * scene, const Renderable renderable)
{
    if (scene->numberOfRenderables >= MAX_SCENE_RENDERABLES)
        return 1; //Too many renderables in this scene

    scene->renderables[scene->numberOfRenderables++] = renderable;
    return 0;
}

int sceneInit(Scene * s)
{
    s->numberOfRenderables = 0;

    return 0;
}
