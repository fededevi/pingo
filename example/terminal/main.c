#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "render/mesh.h"
#include "render/object.h"
#include "render/pixel.h"
#include "render/renderer.h"
#include "render/scene.h"

#include "terminalbackend.h"

#include "assets/teapot.h"
#include "assets/pingo.h"

int main(){
    Vec2i size = {200, 60};

    TerminalBackend backend;
    terminal_backend_init(&backend, size);

    Renderer renderer;
    rendererInit(&renderer, size,(BackEnd*) &backend );
    rendererSetCamera(&renderer,(Vec4i){0,0,size.x,size.y});

    Scene s;
    sceneInit(&s);
    rendererSetScene(&renderer, &s);

    Object object;
    object.mesh = &pingo_mesh;
    object.material = 0;
    sceneAddRenderable(&s, object_as_renderable(&object));

    float phi = 0;
    Mat4 t;

	// optional hide cursor
	/*printf("\033[?25l");*/
	while (1) {
        // PROJECTION MATRIX - Defines the type of projection used
        renderer.camera_projection = mat4Perspective( 1, 2500.0,(float)size.x / (float)size.y, 0.6);

        //VIEW MATRIX - Defines position and orientation of the "camera"
        Mat4 v = mat4Translate((Vec3f) { 0,50,-250});
        Mat4 rotateDown = mat4RotateX(0.40); //Rotate around origin/orbit
        renderer.camera_view = mat4MultiplyM(&rotateDown, &v );

        //TEA TRANSFORM - Defines position and orientation of the object
        object.transform = mat4RotateZ(3.142128);
        t = mat4Scale((Vec3f){1,1,1});
        object.transform = mat4MultiplyM(&object.transform, &t );
        t = mat4Translate((Vec3f){0,70,0});
        object.transform = mat4MultiplyM(&object.transform, &t );
        t = mat4RotateX(3.14);
        object.transform = mat4MultiplyM(&object.transform, &t );

        //SCENE
        s.transform = mat4RotateY(phi);
        phi += 0.01;

        rendererRender(&renderer);
        usleep(40000);
	}

    return 0;
}

