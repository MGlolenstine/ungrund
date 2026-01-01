# Font Atlas Feature

## Overview

The **Font Atlas** system provides a simplified way to render text in Ungrund without requiring users to write custom shaders, manually manage font loading, or handle texture/bind group setup.

This feature was added to address the complexity of text rendering, which previously required ~355 lines of boilerplate code (as seen in the `text_render` example).

## Design Philosophy

The Font Atlas follows **Option 2** from the design analysis: **Font Atlas Generator Helper**

This approach:
- ✅ Removes tedious boilerplate (shader writing, texture setup, stb_truetype complexity)
- ✅ Keeps user control (works with existing `UGVertexBuffer` and `UGRenderPass` systems)
- ✅ Minimal global state (each font atlas is self-contained)
- ✅ Composable (fits naturally with the library's explicit, low-level design)

## What Was Added

### 1. New Files

- **`engine/src/font_atlas.c`** - Complete implementation (~480 lines)
- **`examples/font_atlas_demo/main.c`** - Demo example (~190 lines)
- **`examples/font_atlas_demo/README.md`** - Documentation

### 2. API Added to `engine/include/ungrund.h`

```c
typedef struct UGFontAtlas UGFontAtlas;

// Create font atlas from TrueType font file
UGFontAtlas* ug_font_atlas_create(UGContext* context, const char* font_path,
                                   int font_size, int atlas_width, int atlas_height);

// Destroy font atlas
void ug_font_atlas_destroy(UGFontAtlas* atlas);

// Get pre-configured pipeline and bind group
WGPURenderPipeline ug_font_atlas_get_pipeline(UGFontAtlas* atlas);
WGPUBindGroup ug_font_atlas_get_bind_group(UGFontAtlas* atlas);

// Add text to vertex buffer
void ug_font_atlas_add_text(UGFontAtlas* atlas, void* vertices, size_t* count,
                            const char* text, float x, float y, float pixel_height,
                            float r, float g, float b, float a);

// Helper functions for vertex buffer setup
size_t ug_font_atlas_get_vertex_size(void);
void ug_font_atlas_get_vertex_attributes(UGVertexAttribute* attributes);
```

### 3. Embedded Default Shader

The font atlas includes an embedded WGSL shader for text rendering, so users never need to write or manage a shader file for basic text rendering.

## Key Features

### Automatic Font Atlas Generation

- Loads TrueType fonts (.ttf, .otf, .ttc) using stb_truetype
- Generates a texture atlas with ASCII printable characters (32-126)
- Handles .ttc (TrueType Collection) files correctly
- Configurable atlas size and font size

### Pre-Configured Resources

- **Texture**: R8Unorm format for efficient single-channel storage
- **Sampler**: Linear filtering with clamp-to-edge addressing
- **Bind Group**: Pre-configured with texture and sampler
- **Pipeline**: Pre-built with alpha blending enabled

### Simple Text Generation

The `ug_font_atlas_add_text()` function:
- Generates vertex data for text rendering
- Supports RGBA color per text string
- Works with the existing vertex buffer system
- Each character generates 6 vertices (2 triangles)

### Vertex Format

Text rendering uses a custom vertex format:
```c
struct TextVertex {
    float position[2];  // Screen position
    float uv[2];        // Texture coordinates
    float color[4];     // RGBA color
};
```

## Usage Example

```c
// 1. Create font atlas
UGFontAtlas* font = ug_font_atlas_create(context, "font.ttf", 32, 512, 512);

// 2. Create vertex buffer
size_t vertex_size = ug_font_atlas_get_vertex_size();
UGVertexBuffer* vb = ug_vertex_buffer_create(context, vertex_size, MAX_VERTICES);

// 3. Set vertex layout
UGVertexAttribute attributes[3];
ug_font_atlas_get_vertex_attributes(attributes);
ug_vertex_buffer_set_layout(vb, attributes, 3);

// 4. In render loop - add text
void* vertices = malloc(vertex_size * MAX_VERTICES);
size_t count = 0;

// Calculate pixel height in NDC space
int screen_width, screen_height;
ug_window_get_size(window, &screen_width, &screen_height);
float pixel_height = 2.0f / screen_height;

ug_font_atlas_add_text(font, vertices, &count, "Hello World!",
                      x, y, pixel_height, 1.0f, 1.0f, 1.0f, 1.0f);

// 5. Upload and render
ug_vertex_buffer_update(vb, vertices, count);

UGRenderPass* pass = ug_render_pass_begin(frame, 0, 0, 0, 1);
ug_render_pass_set_pipeline(pass, ug_font_atlas_get_pipeline(font));
ug_render_pass_set_bind_group(pass, 0, ug_font_atlas_get_bind_group(font));
ug_render_pass_set_vertex_buffer(pass, vb);
ug_render_pass_draw(pass, count);
ug_render_pass_end(pass);

// 6. Cleanup
ug_font_atlas_destroy(font);
```

## Comparison: Before vs After

### Before (text_render example)
- **~355 lines** of setup code
- Manual stb_truetype calls (~50 lines)
- Texture creation and upload (~30 lines)
- Sampler creation (~15 lines)
- Bind group setup (~20 lines)
- Custom shader file (~40 lines)
- Pipeline creation boilerplate (~30 lines)

### After (font_atlas_demo)
- **~190 lines** total (including rendering logic)
- Single `ug_font_atlas_create()` call
- Pre-configured pipeline and bind group
- No shader file needed
- Simple `ug_font_atlas_add_text()` for text generation

**Reduction: ~165 lines of boilerplate removed!**

## Technical Details

### Font Atlas Packing

Glyphs are packed into the atlas using a simple row-based algorithm:
- 2-pixel padding between glyphs
- Automatic row wrapping
- Warning if atlas is too small

### Glyph Information

Each glyph stores:
- Atlas coordinates (x0, y0, x1, y1)
- Offset from cursor (xoff, yoff)
- Horizontal advance (xadvance)

### Shader Implementation

The embedded shader:
- Samples the R8 texture (alpha channel)
- Multiplies by vertex color
- Outputs RGBA with alpha blending

## Building and Running

```bash
# Build the engine with font atlas support
make engine

# Build the demo
make font_atlas_demo

# Run the demo
./build/font_atlas_demo
# or
make run-font-atlas
```

## Future Enhancements

Possible improvements:
- Support for custom character ranges (not just ASCII)
- SDF (Signed Distance Field) rendering for better scaling
- Multiple font sizes in one atlas
- Kerning support
- UTF-8 text support (currently ASCII only)
- Font atlas caching/reuse

## Conclusion

The Font Atlas feature successfully simplifies text rendering in Ungrund while maintaining the library's design philosophy of explicit, composable APIs. Users can now add text to their applications with minimal boilerplate while still having full control over the rendering pipeline.

