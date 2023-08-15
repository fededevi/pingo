#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "math/mat4.h"
#include "render/mesh.h"
#include "render/object.h"
#include "render/renderer.h"
#include "render/entity.h"

#include "terminalbackend.h"

#include "assets/teapot.h"
#include "assets/pingo.h"

int main(){

    Object object;
    object_init(&object, &pingo_mesh, 0);

    Entity root;
    entity_init(&root, &object, mat4Identity());

    Vec2i size = {200, 60};

    TerminalBackend backend;
    terminal_backend_init(&backend, size);

    Renderer renderer;
    renderer_init(&renderer, size,(Backend*) &backend );
    rendererSetCamera(&renderer,(Vec4i){0,0,size.x,size.y});

    renderer_set_root_renderable(&renderer, &root);

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
        root.transform = mat4RotateZ(3.142128);
        t = mat4Scale((Vec3f){1,1,1});
        root.transform = mat4MultiplyM(&root.transform, &t );
        t = mat4Translate((Vec3f){0,70,0});
        root.transform = mat4MultiplyM(&root.transform, &t );
        t = mat4RotateX(3.14);
        root.transform = mat4MultiplyM(&root.transform, &t );

        //SCENE
        root.transform = mat4RotateY(phi);
        phi += 0.01;

        renderer_render(&renderer);
        usleep(40000);
	}

    return 0;
}

