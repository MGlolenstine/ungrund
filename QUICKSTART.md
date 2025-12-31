# Quick Start Guide

Get up and running with the Ungrund game engine in minutes!

## Prerequisites Check

Before building, ensure you have:
- ✅ GLFW3 installed
- ✅ wgpu-native installed
- ✅ C compiler (Clang or GCC)
- ✅ Make

If you don't have these, see [SETUP.md](SETUP.md) for detailed installation instructions.

## Build and Run

### Build Everything

```bash
make all
```

This will:
1. Build the engine library (`build/lib/libungrund.a`)
2. Build the triangle example (`build/triangle`)
3. Build the text rendering example (`build/text_render`)

### Run the Triangle Example

```bash
make run-triangle
```

You should see a window with a colored triangle (red, green, and blue vertices).

### Run the Text Rendering Example

```bash
make run-text
```

You should see a window with "Hello World" rendered in white text.

## Project Overview

```
ungrund/
├── engine/              # Engine library source
│   ├── include/         # Public API headers
│   └── src/             # Implementation
├── examples/            # Working examples
│   ├── triangle/        # WebGPU triangle rendering
│   └── text_render/     # Font rendering with stb_truetype
├── shaders/             # WGSL shaders
├── third_party/         # Header-only libraries
└── Makefile             # Build system
```

## Available Make Targets

```bash
make all          # Build everything (default)
make engine       # Build only the engine library
make triangle     # Build triangle example
make text         # Build text rendering example
make run-triangle # Build and run triangle example
make run-text     # Build and run text rendering example
make clean        # Remove all build artifacts
make help         # Show all available targets
```

## What's Included

### Engine Library
- Window management (using GLFW)
- Basic renderer interface (minimal stub)
- Cross-platform support (macOS, Linux, Windows)

### Third-Party Libraries
- **stb_image.h** - Image loading
- **stb_truetype.h** - Font rendering
- **tiny_obj_loader.h** - 3D model loading

### Examples

#### Triangle Example (`examples/triangle/main.c`)
Demonstrates:
- WebGPU initialization
- Surface creation
- Shader loading (WGSL)
- Render pipeline setup
- Drawing a colored triangle

#### Text Rendering Example (`examples/text_render/main.c`)
Demonstrates:
- Loading system fonts with stb_truetype
- Rendering text to a bitmap
- Creating WebGPU textures
- Texture sampling and alpha blending
- Displaying text on screen

## Next Steps

1. **Explore the examples**: Check out the source code in `examples/`
2. **Modify shaders**: Edit the WGSL shaders in `shaders/`
3. **Create your own example**: Use the existing examples as templates
4. **Extend the engine**: Add features to the engine library

## Troubleshooting

### Build fails with "GLFW/glfw3.h not found"
→ Install GLFW: `brew install glfw` (macOS) or `sudo apt-get install libglfw3-dev` (Linux)

### Build fails with "webgpu/webgpu.h not found"
→ Install wgpu-native (see [SETUP.md](SETUP.md))

### Examples build but don't run
→ Check that you have a GPU with Vulkan (Linux) or Metal (macOS) support

### Text example shows blank screen
→ Make sure the font file path in `examples/text_render/main.c` is correct for your system

## Resources

- [WebGPU Specification](https://www.w3.org/TR/webgpu/)
- [WGSL Specification](https://www.w3.org/TR/WGSL/)
- [wgpu-native GitHub](https://github.com/gfx-rs/wgpu-native)
- [GLFW Documentation](https://www.glfw.org/documentation.html)
- [stb Libraries](https://github.com/nothings/stb)

## License

This is a demonstration project. Use as you see fit.

