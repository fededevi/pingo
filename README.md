# pingo

Pingo is a low level 2D/3D graphics library. It uses no dynamically allocated memory and provides a simple backend interface to implement for your ebedded platform. The graphical backend can just provide a framebuffer on which to write on but can also intercept the draw calls and customize the rendering. 


to build (render directly on linux console) : 

```
git clone git@github.com:fededevi/pingo.git
cd pingo
mkdir build && cd build
cmake .. && make
./pingo_example
```


#### An example with texture and per-triangle shading
![Example](/example/viking.png)
