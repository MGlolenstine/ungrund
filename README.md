# Ungrund Game Engine

A minimal C/C++ game engine using WebGPU for graphics rendering.

## Project Structure

```
ungrund/
├── engine/              # Engine library
│   ├── include/         # Public headers
│   │   └── ungrund.h
│   └── src/             # Engine source files
│       ├── window.c
│       └── renderer.c
├── examples/            # Example programs
│   ├── triangle/        # Triangle rendering example
│   │   └── main.c
│   └── text_render/     # Text rendering example
│       └── main.c
├── shaders/             # WGSL shader files
│   ├── triangle.wgsl
│   └── text.wgsl
├── third_party/         # Third-party dependencies
│   ├── stb_image.h
│   ├── stb_truetype.h
│   └── tiny_obj_loader.h
├── build/               # Build output (generated)
└── Makefile             # Build system
```

## Dependencies

### Required
- **GLFW3** - Window and input management
- **wgpu-native** - WebGPU native implementation
- **C compiler** - Clang or GCC

### Included (header-only)
- **stb_image.h** - Image loading
- **stb_truetype.h** - Font rendering
- **tiny_obj_loader.h** - 3D model loading

## Installation

### macOS

```bash
# Install dependencies via Homebrew
brew install glfw

# Install wgpu-native
# Download from: https://github.com/gfx-rs/wgpu-native/releases
# Extract and copy:
#   - libwgpu_native.dylib to /usr/local/lib/libwebgpu.dylib
#   - webgpu.h to /usr/local/include/webgpu/webgpu.h
```

### Linux

```bash
# Install dependencies
sudo apt-get install libglfw3-dev

# Install wgpu-native
# Download from: https://github.com/gfx-rs/wgpu-native/releases
# Extract and copy:
#   - libwgpu_native.so to /usr/local/lib/libwebgpu.so
#   - webgpu.h to /usr/local/include/webgpu/webgpu.h
```

## Building

```bash
# Build everything (engine + examples)
make all

# Build only the engine library
make engine

# Build specific examples
make triangle
make text

# Clean build artifacts
make clean

# Show help
make help
```

## Running Examples

### Triangle Example
Renders a colored triangle using WebGPU.

```bash
make run-triangle
```

### Text Rendering Example
Renders "Hello World" text using stb_truetype.

```bash
make run-text
```

## Engine API

The engine provides a minimal API for window management and rendering:

```c
// Window management
UGWindow* ug_window_create(const char* title, int width, int height);
void ug_window_destroy(UGWindow* window);
bool ug_window_should_close(UGWindow* window);
void ug_window_poll_events(UGWindow* window);

// Renderer management (minimal stub)
UGRenderer* ug_renderer_create(UGWindow* window);
void ug_renderer_destroy(UGRenderer* renderer);
```

## Examples Overview

### Triangle Example
- Demonstrates basic WebGPU initialization
- Creates a render pipeline with WGSL shaders
- Renders a colored triangle with vertex colors

### Text Rendering Example
- Uses stb_truetype to load system fonts
- Renders text to a bitmap texture
- Displays the texture using WebGPU with alpha blending

## License

This is a demonstration project. Use as you see fit.

