# pingo

Pingo is a low level 2D/3D graphics library. It uses no dynamic memory and provides a simple backend interface to implement. The graphical backend should just provide a memory buffer on which to draw on. The example uses a Windows window as the target framebuffer.

The library itself provides an interface to add your own primitives, by default it supports redering of 2D sprites and textured meshes.

Example/main.c contains a couple of examples with 2D and 3D content. 

Shading and texturing:
![Example](/example/shading.png)


Rendering on console:
![Example](/example/console.png)
