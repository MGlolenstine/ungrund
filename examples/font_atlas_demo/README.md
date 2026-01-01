# Font Atlas Demo

This example demonstrates the **Font Atlas** feature of the Ungrund graphics library.

## What is Font Atlas?

The Font Atlas system provides a simplified way to render text without requiring users to:
- Write custom shaders for text rendering
- Manually load fonts with stb_truetype
- Create textures, samplers, and bind groups
- Manage font bitmap generation

Instead, you get a simple API that handles all the complexity internally.

## Features Demonstrated

- **Simple text rendering** with `ug_font_atlas_add_text()`
- **Colored text** with RGBA color control
- **Animated text** with pulsing alpha
- **Multiple text strings** efficiently batched in a single draw call
- **No shader writing required** - uses embedded default shader

## How It Works

1. **Create font atlas** from a TrueType font file:
   ```c
   UGFontAtlas* font = ug_font_atlas_create(context, "font.ttf", 32, 512, 512);
   ```

2. **Get vertex buffer configuration**:
   ```c
   size_t vertex_size = ug_font_atlas_get_vertex_size();
   UGVertexAttribute attributes[3];
   ug_font_atlas_get_vertex_attributes(attributes);
   ```

3. **Add text to vertex array**:
   ```c
   ug_font_atlas_add_text(font, vertices, &count, "Hello!", x, y, r, g, b, a);
   ```

4. **Render with pre-configured pipeline**:
   ```c
   ug_render_pass_set_pipeline(pass, ug_font_atlas_get_pipeline(font));
   ug_render_pass_set_bind_group(pass, 0, ug_font_atlas_get_bind_group(font));
   ug_render_pass_draw(pass, vertex_count);
   ```

## Building and Running

```bash
# Build the demo
make font_atlas_demo

# Run the demo
./build/font_atlas_demo
```

## Notes

- The demo uses `/System/Library/Fonts/Helvetica.ttc` (macOS). Adjust the font path for your system.
- Coordinates are in NDC (Normalized Device Coordinates): -1.0 to 1.0
- The helper function `pixel_to_ndc()` converts pixel coordinates to NDC
- Each character generates 6 vertices (2 triangles)
- The font atlas supports ASCII printable characters (32-126)

## Comparison to Manual Approach

**Without Font Atlas** (text_render example): ~355 lines of setup code
**With Font Atlas** (this example): ~186 lines total, including rendering logic

The Font Atlas removes:
- ❌ Manual stb_truetype calls (~50 lines)
- ❌ Texture creation and upload (~30 lines)
- ❌ Sampler creation (~15 lines)
- ❌ Bind group setup (~20 lines)
- ❌ Custom shader file (~40 lines)
- ❌ Pipeline creation boilerplate (~30 lines)

And provides:
- ✅ Simple `ug_font_atlas_create()` call
- ✅ Pre-configured pipeline and bind group
- ✅ Embedded default shader
- ✅ Easy text generation with `ug_font_atlas_add_text()`

