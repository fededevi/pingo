
# pingo

Pingo is a low level 2D/3D software graphics library. It uses no dynamically allocated memory and provides a simple low level backend interface to implement for your platform. This library can easily be compiled for many different platforms since it has no dependencies except for a working C compiler, it has been tested on Windows, Linux x86, Linux ARM, ESP32, Arm Cortex M3 and even RISC V CPUs.

The cmake project contains targets that build examples that show  3D rendering on different platforms.
```
git clone git@github.com:fededevi/pingo.git
cd pingo
mkdir build && cd build
cmake .. && make
```
 
If you compile on Linux the default cmake project will build 3 example binaries:
- linux_window -> An example that renders to an X11 window
- linux_framebuffer -> An example that renders to a framebuffer device 
- linux_terminal -> An example that renders to the terminal

If you compile on Windows the default cmake project will build 2 example binaries:
- win_window -> An example that renders to a Windows window
- win_terminal -> An example that renders to the terminal

Some backends like the linux framebuffer require exact dimension and pixel format, see pixel.h and this line in the example's main.c:
```
int main(){
    Vec2i size = {300, 150};
```

#### An example with texture and per-triangle shading
![Example](/public/viking.png)