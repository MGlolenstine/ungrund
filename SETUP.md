# Setup Guide for Ungrund Game Engine

This guide will help you set up all the required dependencies to build and run the Ungrund game engine.

## Prerequisites

- C compiler (Clang or GCC)
- Make build system
- Git (for cloning dependencies)

## macOS Setup

### 1. Install Homebrew (if not already installed)

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### 2. Install GLFW

```bash
brew install glfw
```

### 3. Install wgpu-native

wgpu-native is the WebGPU implementation for native platforms.

```bash
# Create a temporary directory
cd /tmp

# Download the latest release for macOS
# Visit: https://github.com/gfx-rs/wgpu-native/releases
# Download the macOS release (e.g., wgpu-macos-x86_64-release.zip or wgpu-macos-aarch64-release.zip)

# For Apple Silicon (M1/M2/M3):
curl -L -o wgpu-native.zip https://github.com/gfx-rs/wgpu-native/releases/download/v27.0.4.0/wgpu-macos-aarch64-release.zip

# For Intel Macs:
# curl -L -o wgpu-native.zip https://github.com/gfx-rs/wgpu-native/releases/download/v0.19.3.1/wgpu-macos-x86_64-release.zip

# Extract the archive
unzip wgpu-native.zip

# Copy the library and header files
sudo mkdir -p /usr/local/include/webgpu
sudo cp libwgpu_native.dylib /usr/local/lib/libwebgpu.dylib
sudo cp webgpu.h /usr/local/include/webgpu/webgpu.h

# Clean up
rm -rf wgpu-native.zip libwgpu_native.dylib webgpu.h
```

### 4. Verify Installation

```bash
# Check GLFW
pkg-config --modversion glfw3

# Check wgpu-native
ls -l /usr/local/lib/libwebgpu.dylib
ls -l /usr/local/include/webgpu/webgpu.h
```

## Linux Setup (Ubuntu/Debian)

The Ungrund engine supports both **X11** and **Wayland** display servers on Linux. The engine automatically detects which display server you're using at runtime.

### 1. Install Build Tools

```bash
sudo apt-get update
sudo apt-get install build-essential git pkg-config
```

### 2. Install GLFW

```bash
sudo apt-get install libglfw3-dev
```

### 3. Install wgpu-native

```bash
# Create a temporary directory
cd /tmp

# Download the latest release for Linux
curl -L -o wgpu-native.zip https://github.com/gfx-rs/wgpu-native/releases/download/v0.19.3.1/wgpu-linux-x86_64-release.zip

# Extract the archive
unzip wgpu-native.zip

# Copy the library and header files
sudo mkdir -p /usr/local/include/webgpu
sudo cp libwgpu_native.so /usr/local/lib/libwebgpu.so
sudo cp webgpu.h /usr/local/include/webgpu/webgpu.h

# Update library cache
sudo ldconfig

# Clean up
rm -rf wgpu-native.zip libwgpu_native.so webgpu.h
```

### 4. Install Display Server Development Libraries

For **X11** support:
```bash
sudo apt-get install libx11-dev
```

For **Wayland** support:
```bash
sudo apt-get install libwayland-dev
```

**Note:** You can install both to support running on either X11 or Wayland. The engine will automatically detect and use the appropriate display server at runtime based on your environment.

## Building the Project

Once all dependencies are installed:

```bash
# Navigate to the project directory
cd /path/to/ungrund

# Build everything
make all

# Or build specific targets
make engine      # Build only the engine library
make triangle    # Build triangle example
make text        # Build text rendering example
```

## Running Examples

```bash
# Run the triangle example
make run-triangle

# Run the text rendering example
make run-text
```

## Troubleshooting

### "GLFW/glfw3.h not found"
- Make sure GLFW is installed via your package manager
- On macOS: `brew install glfw`
- On Linux: `sudo apt-get install libglfw3-dev`

### "webgpu/webgpu.h not found"
- Make sure you've installed wgpu-native and copied the header file to the correct location
- Check that `/usr/local/include/webgpu/webgpu.h` exists

### "libwebgpu not found" at runtime
- On macOS: Make sure `/usr/local/lib/libwebgpu.dylib` exists
- On Linux: Make sure `/usr/local/lib/libwebgpu.so` exists and run `sudo ldconfig`

### Examples don't run
- Make sure you have a GPU that supports Vulkan (Linux) or Metal (macOS)
- Check that your graphics drivers are up to date

### Linux: "Failed to get Wayland display or surface from GLFW"
- This error occurs when running on Wayland but GLFW wasn't compiled with Wayland support
- Install GLFW from source with Wayland support, or use the X11 backend by setting:
  ```bash
  export WAYLAND_DISPLAY=""
  ```
- Alternatively, ensure your GLFW package includes Wayland support (most modern distributions do)

### Linux: "Failed to get X11 display from GLFW"
- This error occurs when running on X11 but GLFW wasn't compiled with X11 support
- Make sure `libx11-dev` is installed and reinstall GLFW
- Check that the `DISPLAY` environment variable is set:
  ```bash
  echo $DISPLAY
  ```

### Linux: How to check which display server I'm using?
```bash
echo $XDG_SESSION_TYPE
# Output will be either "x11" or "wayland"
```

The engine will automatically detect and use the correct display server. You can also check the console output when running examples - it will print "Created X11 surface" or "Created Wayland surface".

## Next Steps

After successful installation, check out:
- `README.md` - Project overview and API documentation
- `examples/triangle/main.c` - Triangle rendering example source
- `examples/text_render/main.c` - Text rendering example source
- `shaders/` - WGSL shader files

