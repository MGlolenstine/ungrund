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
│   ├── text_render/     # Text rendering example
│   │   └── main.c
│   ├── pong/            # Pong game example
│   │   └── main.c
│   ├── input_callbacks/ # Input handling example
│   │   └── main.c
│   └── geometry_demo/   # Geometry helper functions demo
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
make pong
make input_callbacks
make geometry_demo

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

### Pong Game Example
A simple Pong game demonstrating game logic and input handling.

```bash
make run-pong
```

### Input Callbacks Example
Demonstrates keyboard and mouse input callbacks.

```bash
make run-input
```

### Geometry Demo Example
Demonstrates the new geometry helper functions for generating shapes.

```bash
make run-geometry
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

// Geometry helpers - standard vertex formats
typedef struct {
    float position[2];
    float color[3];
} UGVertex2DColor;

typedef struct {
    float position[2];
    float uv[2];
} UGVertex2DTextured;

// Add shapes to vertex arrays
void ug_add_rect_2d_color(UGVertex2DColor* vertices, size_t* count,
                          float x, float y, float w, float h,
                          float r, float g, float b);

void ug_add_circle_2d_color(UGVertex2DColor* vertices, size_t* count,
                            float x, float y, float width, float height,
                            float r, float g, float b, int segments);

void ug_add_rect_2d_textured(UGVertex2DTextured* vertices, size_t* count,
                             float x, float y, float w, float h,
                             float u0, float v0, float u1, float v1);

void ug_add_circle_2d_textured(UGVertex2DTextured* vertices, size_t* count,
                               float x, float y, float width, float height,
                               float u0, float v0, float u1, float v1,
                               int segments);
```

### Geometry Helper Functions

The library provides helper functions for generating common 2D shapes:

**Key Features:**
- ✅ Standard vertex formats (`UGVertex2DColor`, `UGVertex2DTextured`)
- ✅ Simple API for rectangles and circles/ellipses
- ✅ Works with generic vertex buffers
- ✅ Compatible with custom pipelines
- ✅ Zero overhead - just vertex generation

**Example Usage:**
```c
// Create vertex array
UGVertex2DColor vertices[1024];
size_t count = 0;

// Add a red rectangle
ug_add_rect_2d_color(vertices, &count,
                     0.0f, 0.0f, 0.1f, 0.1f,
                     1.0f, 0.0f, 0.0f);

// Add a green circle (32 segments)
ug_add_circle_2d_color(vertices, &count,
                       0.5f, 0.0f, 0.15f, 0.0f,
                       0.0f, 1.0f, 0.0f, 32);

// Upload to vertex buffer
ug_vertex_buffer_update(vertex_buffer, vertices, count);
```

See `examples/geometry_demo/` for a complete example.

## Examples Overview

### Triangle Example
- Demonstrates basic WebGPU initialization
- Creates a render pipeline with WGSL shaders
- Renders a colored triangle with vertex colors

### Text Rendering Example
- Uses stb_truetype to load system fonts
- Renders text to a bitmap texture
- Displays the texture using WebGPU with alpha blending

### Pong Game Example
- Complete 2-player Pong game
- Demonstrates game loop and physics
- Shows input handling with callbacks
- Uses geometry helpers for rendering

### Input Callbacks Example
- Demonstrates keyboard, mouse move, and mouse button callbacks
- Shows how to set up event-driven input handling
- Simple example for learning the input system

### Geometry Demo Example
- Showcases the new geometry helper functions
- Renders rectangles, circles, and ellipses
- Demonstrates both static and animated shapes
- Shows how to use `UGVertex2DColor` format

## License

This is a demonstration project. Use as you see fit.

