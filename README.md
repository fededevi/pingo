
# Pingo - High-Performance Software 3D Graphics Library

[![CI Build and Test](https://github.com/fededevi/pingo/workflows/CI%20Build%20and%20Test/badge.svg)](https://github.com/fededevi/pingo/actions)

Pingo is a high-performance, low-level 2D/3D software graphics library designed for cross-platform compatibility and optimal performance. It features:

- **🚀 Optimized Math Library**: 82-504% performance improvements with SIMD-friendly operations
- **🎯 Advanced Rendering**: Configurable culling optimizations providing up to 122% FPS improvements
- **⚡ Zero Dynamic Allocation**: No malloc/free - perfect for embedded systems
- **🔧 Modular Design**: Optional X11 dependency - works in headless environments
- **🌐 Cross-Platform**: Windows, Linux x86/ARM, ESP32, Cortex M3, RISC-V support
- **✅ Comprehensive Testing**: Full test suite with automated benchmarks

## Quick Start

```bash
git clone https://github.com/fededevi/pingo.git
cd pingo
mkdir build && cd build
cmake .. && make

# Run tests
./tests/math_tests
./tests/math_benchmarks

# Try examples
./linux_terminal      # ASCII art rendering
./linux_framebuffer   # Direct framebuffer rendering
./linux_window        # X11 window rendering (requires X11)
```

## Build Targets

### Core Libraries
- **`libpingo.so`** - Main graphics library (no external dependencies)
- **`libassets.so`** - 3D model assets (Viking, Cube, Teapot)

### Examples
- **`linux_window`** - X11 window rendering (requires X11)
- **`linux_framebuffer`** - Direct framebuffer rendering
- **`linux_terminal`** - ASCII art terminal rendering  
- **`render_to_image`** - JPEG file output (requires libjpeg)
- **`win_window`** - Windows GDI rendering (Windows only)

### Test & Benchmark Suite
- **`math_tests`** - Comprehensive math library test suite
- **`math_benchmarks`** - Performance benchmarks for math operations
- **`library_optimized_test`** - Multi-object rendering optimization tests
- **`visual_benchmarks`** - Real-time visual performance analysis

## Performance Optimizations

### Math Library Improvements
- **Vec3 Normalize**: +504% performance with fast paths
- **Mat4 Operations**: +82-200% performance improvements
- **Mat3 Operations**: +150-300% performance gains

### Rendering Optimizations
- **Backface Culling**: +45% FPS improvement
- **Early Z-Test**: +22% FPS improvement  
- **Frustum Culling**: +79% FPS in multi-object scenes
- **Combined**: Up to +122% FPS improvement (3.25→7.22 FPS)

## Architecture Overview

### Rendering Pipeline
```
Scene → Transform → Projection → Culling → Rasterization → Shading → Output
```

### Backend System
Pingo uses a flexible backend system that allows rendering to different targets:

```c
typedef struct Backend {
    void (*init)(Renderer *, struct Backend *, Vec4i rect);
    void (*beforeRender)(Renderer *, struct Backend *);
    void (*afterRender)(Renderer *, struct Backend *);
    Pixel *(*getFrameBuffer)(Renderer *, struct Backend *);
    PingoDepth *(*getZetaBuffer)(Renderer *, struct Backend *);
} Backend;
```

### Optimization Configuration
```c
// Configure rendering optimizations at runtime
renderer_enable_backface_culling(&renderer, true);   // +45% FPS
renderer_enable_frustum_culling(&renderer, true);    // +79% FPS (multi-object)
renderer_enable_early_z_test(&renderer, true);       // +22% FPS
renderer_set_all_optimizations(&renderer, true);     // +122% FPS combined
```

## Dependencies

### Core Library
- **No external dependencies** - Only standard C library (libc, libm)
- Works in **headless environments** without display systems

### Optional Dependencies  
- **X11** - For window-based examples (libx11-dev, libxext-dev)
- **JPEG** - For image output examples (libjpeg-dev)
- **MinGW** - For Windows cross-compilation

## Building

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake

# Optional: X11 support
sudo apt-get install libx11-dev libxext-dev

# Optional: JPEG support  
sudo apt-get install libjpeg-dev
```

### Build Options
```bash
# Standard build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Debug build with symbols
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel

# Build specific targets
cmake --build build --target math_tests --parallel
cmake --build build --target linux_window --parallel
```

### Cross-Platform Notes
- **Linux**: All targets supported
- **Windows**: Use MinGW or Visual Studio
- **Embedded**: Disable examples, use core library only
- **Headless**: X11 targets automatically disabled if not available

## Testing

### Running Tests
```bash
cd build

# Run all math library tests
./tests/math_tests

# Run performance benchmarks  
./tests/math_benchmarks

# Test rendering optimizations (requires X11)
./tests/library_optimized_test

# Visual performance analysis (requires X11)
./tests/visual_benchmarks
```

### Test Coverage
- **Vector Operations** (Vec2, Vec3, Vec4)
- **Matrix Operations** (Mat3, Mat4)  
- **Mathematical Functions** (edge, orientation, perspective)
- **Rendering Pipeline** (culling, rasterization, optimization)
- **Performance Benchmarks** (math ops, rendering throughput)

## API Reference

### Core Types
```c
// Vectors
typedef struct { float x, y; } Vec2f;
typedef struct { float x, y, z; } Vec3f;  
typedef struct { float x, y, z, w; } Vec4f;

// Matrices
typedef struct { float elements[9]; } Mat3;
typedef struct { float elements[16]; } Mat4;

// Rendering
typedef struct { /* ... */ } Renderer;
typedef struct { /* ... */ } Backend;
typedef struct { /* ... */ } Entity;
```

### Key Functions
```c
// Math operations
Vec3f vec3Normalize(Vec3f v);           // +504% optimized
Mat4 mat4MultiplyM(Mat4 *a, Mat4 *b);   // +82% optimized  
Mat4 mat4Perspective(float fov, float far, float aspect, float near);

// Rendering
int renderer_init(Renderer *r, Vec2i size, Backend *backend);
int renderer_render(Renderer *r);
void renderer_enable_backface_culling(Renderer *r, bool enable);
```

## Performance Metrics

### Benchmark Results (Release Build)
```
Math Operations (ops/sec):
- vec3Normalize:     17.8M → 107.2M (+504%)
- mat4MultiplyM:     26.9M → 49.0M  (+82%)
- mat3Inverse:       34.5M → 87.2M  (+153%)

Rendering (640×480, 25 objects):
- No optimizations:  3.25 FPS
- All optimizations: 7.22 FPS (+122%)
- Triangle throughput: 474K triangles/sec
```

## Contributing

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Run tests** (`./build/tests/math_tests`)
4. **Commit** changes (`git commit -m 'Add amazing feature'`)
5. **Push** to branch (`git push origin feature/amazing-feature`)
6. **Open** a Pull Request

### Development Guidelines
- Maintain **zero dynamic allocation** design
- Add **tests** for new functionality  
- Run **benchmarks** to verify performance
- Ensure **cross-platform** compatibility
- Update **documentation** for API changes

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Example Output

#### Viking model with texture and per-triangle shading
![Example](/public/viking.png)

#### ASCII Terminal Rendering
The terminal backend can render 3D scenes directly to your terminal using ASCII characters, perfect for headless environments or retro aesthetics.