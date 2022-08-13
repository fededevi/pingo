# pingo

Pingo is a low level 2D/3D graphics library. It uses no dynamically allocated memory and provides a simple backend interface to implement for your ebedded platform. The graphical backend can just provide a framebuffer on which to write on but can also intercept the draw calls and customize the rendering. 


Build, crossplatform win/linux, on windows renders on window, on linux renders on terminal,
there are no dependencies on linux, you just need a C compiler: 

Copy and paste this command on a terminal and you should have the running 3d renderer in less than 5 seconds:

```
git clone git@github.com:fededevi/pingo.git
cd pingo
mkdir build && cd build
cmake .. && make
./pingo_example

```

Change these lines to change output resolution in linux terminal example:

```
int main(){
    Vec2i size = {300, 150};
```

#### An example with texture and per-triangle shading
![Example](/example/viking.png)
