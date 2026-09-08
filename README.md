# Pingo

[![CI Build and Test](https://github.com/fededevi/pingo/workflows/CI%20Build%20and%20Test/badge.svg)](https://github.com/fededevi/pingo/actions)

A small software 3D renderer in C. No GPU, no dynamic allocation in the library
itself, and no dependencies beyond `libm` — so it runs anywhere from a desktop
window to a microcontroller or a terminal.

![Example](/public/viking.png)

## How it works

Pingo rasterizes triangles into a plain pixel buffer. It never owns that buffer:
a **backend** supplies it, and Pingo only fills it in.

One frame of `renderer_render()`:

1. Clear the depth buffer (and the framebuffer, if `clear` is set).
2. Ask the backend for the framebuffer and depth buffer, and call `beforeRender`.
3. Walk the scene graph from `root_renderable` with an identity transform.
4. Call `afterRender`, which is where the backend presents the pixels.

The scene graph is built from one interface — a struct whose first member is a
render function pointer:

```c
typedef struct {
    int (*render)(void *this, Mat4 transform, Renderer *renderer);
} Renderable;
```

Anything embedding a `Renderable` as its first field is drawable, which is how C
gets polymorphism here:

- **`Entity`** — a transform, a visibility flag, and child entities. Composing
  these is what makes the graph a tree; each node concatenates its transform and
  recurses.
- **`Object`** — a `Mesh` plus a `Material`. This is what actually reaches the
  rasterizer.
- **`Sprite`** — a `Texture` drawn straight through the given transform,
  bypassing the camera.

The camera lives on the `Renderer` as `camera_projection` and `camera_view`.
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

## Backends

A backend is five function pointers — `init`, `beforeRender`, `afterRender`,
`getFrameBuffer`, `getZetaBuffer`. Port Pingo somewhere new by implementing
those and handing the struct to `renderer_init()`.

| Example | Output | Needs |
|---------|--------|-------|
| `linux_window` | X11 window | libX11 |
| `linux_framebuffer` | `/dev/fb0` directly | — |
| `linux_terminal` | ASCII art in the terminal | — |
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

Presets: `default` and `debug`. The presets build
`Release`; a plain `cmake -B build -S .` uses whatever build type you ask for,
including none. Configuring without optimization prints a note that the
benchmarks will be measuring unoptimized code.

`pingo_math`, `pingo_render` and the tests need nothing beyond `libm`. libX11
and libjpeg are probed at configure time, and the examples needing them are
skipped with a message if they're absent — so any environment builds whatever
it can without being told what it has.

`cmake --install <build dir> --prefix <dir>` installs the libraries, headers and
a package config, so another project can `find_package(pingo)` and link
`pingo::pingo_render`.

## License

CC0 1.0 Universal — public domain.
