# Pingo

[![CI Build and Test](https://github.com/fededevi/pingo/workflows/CI%20Build%20and%20Test/badge.svg)](https://github.com/fededevi/pingo/actions)

A small software 3D renderer in C. No GPU, no dynamic allocation and no
dependencies beyond `libm`, so it runs anywhere from a desktop window to a
microcontroller or a terminal. The libraries claim no memory of their own —
all of it is the buffers and meshes you hand them.

![Example](/public/viking.png)

## How it works

Pingo rasterizes triangles into a plain pixel buffer. It never owns that buffer:
a **backend** supplies it, and Pingo only fills it in.

One frame of `renderer_render()`:

1. Ask the backend for its render target — a colour surface plus the depth
   buffer that belongs to it — and call `before_render`.
2. Clear the depth buffer, and the colour surface if `clear` is set.
3. Walk the scene graph from `root_renderable` with an identity transform.
4. Call `after_render`, which is where the backend presents the pixels.

The scene graph is built from one interface — a struct whose first member is a
render function pointer:

```c
typedef struct {
    int (*render)(void *this, Mat4 transform, Renderer *renderer);
} Renderable;
```

Anything embedding a `Renderable` as its first field is drawable, which is how C
gets polymorphism here:

- **`Entity`** — a transform, a visibility flag, and children. Composing these
  is what makes the graph a tree; each node concatenates its transform and
  recurses. Its content may be NULL, which makes it a pure grouping node, and
  its children are `Renderable *`, so anything drawable can be one.
- **`Object`** — a `Mesh` plus a `Material`. This is what actually reaches the
  rasterizer.
- **`Sprite`** — a `Texture` drawn straight through the given transform,
  bypassing the camera.

The camera lives on the `Renderer` as `camera.projection` and `camera.view`.
Backface culling, frustum culling and early-Z are toggles on the same struct
(`renderer_enable_*`), all on by default.

## Libraries

Three libraries. Each is built by its own directory, which also holds that
library's tests:

| Library | Contents |
|---------|----------|
| `pingo_math` | `Vec2/3/4`, `Mat3/4`, fixed-size and allocation-free |
| `pingo_render` | Renderer, rasterizer, scene graph, texture, depth buffer |
| `pingo_assets` | Generated mesh tables (viking, cube, teapot) |

`pingo_render` links `pingo_math`, and `pingo_assets` uses the `Mesh` type from
`pingo_render`, so linking the renderer alone brings the maths with it.

`render_unit_tests` covers the render types in isolation - Pixel, Texture,
depth, RenderTarget, Transform, the drawables and Renderer - checking the
things a picture cannot show: which arguments are rejected, that the masked
and general texture samplers agree, that the shade table matches `pixel_mul`
for every channel value and factor, and that a failing child aborts a
traversal. `math_tests` does the same for the maths.

The renderer's tests draw into memory at 64x48 and compare the result against
committed PPM references, so they run on every platform - including the
cross-compiled ones, where no window backend exists. Regenerate them all after
a deliberate change with `cmake --build <dir> --target update_references`; the
comparison tolerates a few differing pixels, so a suite refreshed by hand and
another left behind will both keep passing while they drift apart.

## Backends

A backend is four function pointers — `init`, `before_render`, `after_render`
and `get_target`. Port Pingo somewhere new by implementing those and handing
the struct to `renderer_init()`.

`get_target` returns a `RenderTarget`: a `Texture` to draw into and a depth
buffer with one entry per pixel of it. They travel together so their sizes
cannot disagree — a `Texture` on its own is any 2D surface, which is what a
source image and a sprite are.

| Example | Output | Needs |
|---------|--------|-------|
| `linux_window` | X11 window | libX11 |
| `linux_framebuffer` | `/dev/fb0` directly | — |
| `terminal` | ASCII art in the terminal (any platform) | — |
| `render_to_image` | JPEG file | libjpeg |
| `win_window` | Win32 GDI window | Windows |

Every one of them links the same `main.c`; only the backend library differs.

## Build

Out-of-source builds only.

```bash
cmake --workflow --preset default   # configure, build, test
```

Binaries land together in the preset's build directory. The examples read their
texture relative to the working directory, so run them from there.

`--workflow` runs three steps in order — configure, build, then the test
suite — and stops at the first failure. There are three presets:

| Preset | Build type | What it produces |
|--------|-----------|------------------|
| `default` | `Release` | every library, example and test the host can build |
| `debug` | `Debug` | the same, unoptimized |
| `shared` | `Release` | the same, as shared libraries - see below |
| `freestanding` | `Release` | the libraries alone, for a target with no OS |

`freestanding` is what a microcontroller build looks like, and takes its cross
compiler from `CC` and `CFLAGS`.

There is also one `arch-*` preset per cross target — `cmake --list-presets=workflow`
lists them. Each carries only its compiler and the sysroot qemu needs, so
`cmake --workflow --preset arch-aarch64` configures, builds and runs the whole
test suite on aarch64 through qemu's binfmt handler. `scripts/bench-all-arch.sh`
walks all of them and prints one table, because `cmake --workflow` takes a
single preset and preset conditions cannot tell whether a compiler is installed.

libX11 and libjpeg are probed at configure time and the examples needing them
skipped with a message, so any environment builds whatever it can without being
told what it has.

### Link it statically

This is the default, and the `shared` preset is the one that opts out. It is
worth about 19% of the render time,
and the reason is worth knowing: the rasterizer's inner loop and the pixel write
it calls per pixel are in different translation units, so the call only
disappears if the compiler can optimize across both. A shared library has to
keep its exported functions replaceable at load time, so it may not inline them
however much link-time optimization can see — the call, and the reloads the
optimizer must assume around it, stay in the innermost loop. A static archive
has no such obligation.

Link-time optimization is what actually removes the call, and it is on by
default in the optimized build types wherever the toolchain reports support for
it (`PINGO_ENABLE_LTO`). On a shared build it is nearly free and nearly
pointless; static is where it pays.

`cmake --install <build dir> --prefix <dir>` installs the libraries, headers and
a package config, so another project can `find_package(pingo)` and link
`pingo::pingo_render`.
