project( pingo )
set ( SRC
    ./example/teapot.c
    ./example/teapot.h
    ./example/memorybackend.c
    ./example/pingo_mesh.c
    ./example/viking.h 
    ./example/viking.c
    ./example/cube.h 
    ./example/cube.c
    ./example/memorybackend.h
    ./example/pingo_mesh.h
    ./render/texture.c
    ./render/rasterizer.h
    ./render/sprite.h
    ./render/object.h
    ./render/renderable.c
    ./render/backend.h
    ./render/mesh.h
    ./render/renderer.c
    ./render/depth.c
    ./render/material.h
    ./render/scene.c
    ./render/scene.h
    ./render/depth.h
    ./render/mesh.c
    ./render/object.c
    ./render/pixel.c
    ./render/renderer.h
    ./render/sprite.c
    ./render/texture.h
    ./render/material.c
    ./render/pixel.h
    ./render/rasterizer.c
    ./render/renderable.h
    ./math/vec3.h
    ./math/vec2.c
    ./math/vec3.c
    ./math/vec4.h
    ./math/types.h
    ./math/mat3.c
    ./math/vec4.c
    ./math/vec2.h
    ./math/mat4.h
    ./math/mat3.h
    ./math/mat4.c
)

if (WIN32)
    set ( SRC ${SRC}
        ./example/windowbackend.h
        ./example/windowbackend.c
    )
    add_executable(pingo_example ./example/main-win.c ${SRC})
endif (WIN32)

if (UNIX)
    set ( SRC ${SRC}
        ./example/terminalbackend.h
        ./example/terminalbackend.c
    )
    add_executable(pingo_example ./example/main-linux.c ${SRC})
endif (UNIX)

target_link_libraries(pingo_example m) # C math lib
