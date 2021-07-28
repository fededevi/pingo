#!/bin/bash -e
#gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o consolebackend.o example/consolebackend.c
gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o linuxtermbackend.o example/linuxtermbackend.c
gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o cube.o example/cube.c
#gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o main.o example/main.c
gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o main-linux.o example/main-linux.c
gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o pingo_mesh.o example/pingo_mesh.c
gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o teapot.o example/teapot.c
gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o viking.o example/viking.c
gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o windowbackend.o example/windowbackend.c
gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o mat3.o math/mat3.c
gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o mat4.o math/mat4.c
gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o vec2.o math/vec2.c
gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o vec3.o math/vec3.c
gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o vec4.o math/vec4.c
gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o depth.o render/depth.c
gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o material.o render/material.c
gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o mesh.o render/mesh.c
gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o object.o render/object.c
gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o pixel.o render/pixel.c
gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o rasterizer.o render/rasterizer.c
gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o renderable.o render/renderable.c
gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o renderer.o render/renderer.c
gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o scene.o render/scene.c
gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o sprite.o render/sprite.c
gcc -c -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC -g -I. -o texture.o render/texture.c
g++ -Wl,-O1 -o Pingo linuxtermbackend.o cube.o main-linux.o pingo_mesh.o teapot.o viking.o windowbackend.o mat3.o mat4.o vec2.o vec3.o vec4.o depth.o material.o mesh.o object.o pixel.o rasterizer.o renderable.o renderer.o scene.o sprite.o texture.o
