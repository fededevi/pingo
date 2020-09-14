# pingo

Pingo is a low level graphics library. It uses no dynamic memory and provides a simple backend interface to implement. The graphical backend should just provide a memory buffer on which to draw on. The example uses a Windows window as the target framebuffer.

The library itself provides an interface to add your own primitives, by default it supports redering of 2D sprites and textured meshes.

Example/main.c contains a couple of examples with 2D and 3D content. 
