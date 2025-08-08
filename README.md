
# Pingo

Pingo is a low-level 2D/3D software graphics library designed for cross-platform rendering. It uses no dynamically allocated memory and provides a simple low-level backend interface to implement for your platform. This library can easily be compiled for many different platforms since it has no dependencies except for a working C compiler. It has been tested on Windows, Linux x86, Linux ARM, ESP32, Arm Cortex M3 and even RISC V CPUs.

## 🏗️ Architecture Overview

Pingo follows a modular architecture with clear separation of concerns:

### Core Components

- **`math/`** - Mathematical foundations including vectors (Vec2, Vec3, Vec4), matrices (Mat3, Mat4), and utility functions
- **`render/`** - Core rendering engine with rasterizer, depth buffer, texture management, and entity system
- **`assets/`** - 3D model data (cube, teapot, viking models) and asset management
- **`backends/`** - Platform-specific rendering backends implementing the common Backend interface

### Backend System

The library uses a pluggable backend system that allows rendering to different targets:

- **Linux Window Backend** (`backends/linux_window/`) - X11 window rendering
- **Linux Framebuffer Backend** (`backends/linux_framebuffer/`) - Direct framebuffer device rendering
- **Terminal Backend** (`backends/terminal/`) - ASCII/ANSI terminal rendering
- **Render-to-Image Backend** (`backends/render_to_image/`) - JPEG file output
- **Win32 Window Backend** (`backends/win32_window/`) - Windows native window rendering

Each backend implements the common `Backend` interface defined in `render/backend.h`, providing:
- Frame buffer access
- Depth buffer management
- Initialization and cleanup hooks
- Pre/post rendering callbacks

### Rendering Pipeline

The rendering system follows a traditional 3D graphics pipeline:

1. **Entity System** - Objects with transforms and materials
2. **Rasterization** - Software rasterizer with depth testing
3. **Texture Mapping** - Per-triangle texture sampling
4. **Backend Output** - Platform-specific frame buffer updates

## 🚀 Quick Start

```bash
git clone git@github.com:fededevi/pingo.git
cd pingo
mkdir build && cd build
cmake .. && make
```

## 📦 Build Targets

### Linux Builds
The default cmake project will build 3 example binaries on Linux:
- **`linux_window`** - Renders to an X11 window
- **`linux_framebuffer`** - Renders to a framebuffer device 
- **`linux_terminal`** - Renders to the terminal

### Windows Builds
On Windows, the project builds 2 example binaries:
- **`win_window`** - Renders to a Windows window
- **`win_terminal`** - Renders to the terminal

## ⚙️ Configuration

Some backends like the Linux framebuffer require exact dimensions and pixel format. See `render/pixel.h` for pixel format details and configure the rendering size in your example's main.c:

```c
int main(){
    Vec2i size = {300, 150};
    // ... rest of your code
}
```

## 🎨 Example with Texture and Per-Triangle Shading

![Viking House Example](/public/viking.png)

The example above demonstrates:
- 3D model loading and rendering
- Texture mapping with RGBA textures
- Real-time rotation and transformation
- Material and lighting system
- Cross-platform backend abstraction

## 📁 Project Structure

```
pingo/
├── math/           # Mathematical foundations (vectors, matrices)
├── render/         # Core rendering engine
├── assets/         # 3D models and asset data
├── backends/       # Platform-specific rendering backends
├── examples/       # Usage examples and backend configurations
└── public/         # Documentation assets
```

## 🔧 Development

The library is designed for:
- **Zero dynamic allocation** - All memory is pre-allocated
- **Cross-platform compatibility** - Minimal dependencies
- **Educational purposes** - Clear, readable software rendering implementation
- **Embedded systems** - Lightweight and portable

## 📄 License

This project is licensed under the terms specified in the LICENSE file.