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

    Entity root_entity;
    entity_init(&root_entity, (Renderable*)&object, mat4Identity());

    Vec2i size = {200, 60};

    TerminalBackend backend;
    terminal_backend_init(&backend, size);

    Renderer renderer;
    renderer_init(&renderer, size,(Backend*) &backend );

    renderer_set_root_renderable(&renderer, (Renderable*)&root_entity);

    float phi = 0;
    Mat4 t;

    // PROJECTION MATRIX - Defines the type of projection used
    renderer.camera_projection = mat4Perspective( 1, 2500.0,(float)size.x / (float)size.y, 1);

    //VIEW MATRIX - Defines position and orientation of the "camera"
    Mat4 v = mat4Translate((Vec3f) { 0,0,-250});
    Mat4 rotateDown = mat4RotateX(0.40); //Rotate around origin/orbit
    renderer.camera_view = mat4MultiplyM(&rotateDown, &v );

    while (1) {
        Mat4 rotate1 = mat4RotateY(phi);
        Mat4 rotate2 = mat4RotateX(3.1421);
        root_entity.transform = mat4MultiplyM( &rotate1, &rotate2 );
        phi += 0.01;

        renderer_render(&renderer);
        usleep(40000);
	}

    return 0;
}

