# pingo

Pingo is a low level graphics library designed to be small and safe. It uses no dynamic memory and provides a simple backend interface to implement. 
The graphical backend should just provide a memory buffer on which to draw on.

The library itself provides an interface to add your own primitives (Renderables) and your own containers (#Scene), by default it supports redering of simple sprites.

It is _mostly_ format agnostic, the #Pixel structure defines the format of the framebuffer to render. 

