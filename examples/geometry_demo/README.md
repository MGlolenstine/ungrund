# Geometry Demo Example

This example demonstrates the new geometry helper functions added to the Ungrund engine library. These functions make it easy to generate common 2D shapes without having to manually write vertex generation code.

## New Library Features

### Standard Vertex Formats

The library now provides two standard vertex formats:

```c
// Position (vec2) + Color (vec3)
typedef struct {
    float position[2];
    float color[3];
} UGVertex2DColor;

// Position (vec2) + UV coordinates (vec2)
typedef struct {
    float position[2];
    float uv[2];
} UGVertex2DTextured;
```

### Geometry Helper Functions

#### For Colored Geometry (`UGVertex2DColor`)

**Add Rectangle:**
```c
void ug_add_rect_2d_color(UGVertex2DColor* vertices, size_t* count,
                          float x, float y, float w, float h,
                          float r, float g, float b);
```
- `x, y`: Center position
- `w, h`: Half-width and half-height
- `r, g, b`: Color components (0.0 to 1.0)
- Generates 6 vertices (2 triangles)

**Add Circle/Ellipse:**
```c
void ug_add_circle_2d_color(UGVertex2DColor* vertices, size_t* count,
                            float x, float y, float width, float height,
                            float r, float g, float b, int segments);
```
- `x, y`: Center position
- `width`: Radius in x direction
- `height`: Radius in y direction (if 0, uses `width` for both to create a perfect circle)
- `r, g, b`: Color components (0.0 to 1.0)
- `segments`: Number of triangular segments (higher = smoother, minimum 3)
- Generates `segments * 3` vertices

#### For Textured Geometry (`UGVertex2DTextured`)

**Add Textured Rectangle:**
```c
void ug_add_rect_2d_textured(UGVertex2DTextured* vertices, size_t* count,
                             float x, float y, float w, float h,
                             float u0, float v0, float u1, float v1);
```
- `x, y`: Center position
- `w, h`: Half-width and half-height
- `u0, v0, u1, v1`: Texture coordinate bounds
- Generates 6 vertices (2 triangles)

**Add Textured Circle/Ellipse:**
```c
void ug_add_circle_2d_textured(UGVertex2DTextured* vertices, size_t* count,
                               float x, float y, float width, float height,
                               float u0, float v0, float u1, float v1,
                               int segments);
```
- Same parameters as colored version, plus texture coordinates
- Generates `segments * 3` vertices

## Building and Running

```bash
# Build the example
make geometry_demo

# Run the example
make run-geometry
```

## What This Example Shows

The demo renders several animated shapes using the new helper functions:

1. **Red Rectangle** (top-left) - Basic rectangle
2. **Green Circle** (top-right) - Perfect circle (height = 0)
3. **Blue Ellipse** (bottom-left) - Ellipse with different width/height
4. **Yellow Orbiting Circle** - Animated position
5. **Cyan Rectangles** - Multiple rectangles in a circular pattern
6. **Magenta Pulsing Circle** - Animated size at center

## Usage Pattern

```c
// 1. Create a vertex array
UGVertex2DColor vertices[MAX_VERTICES];
size_t vertex_count = 0;

// 2. Add shapes using helper functions
ug_add_rect_2d_color(vertices, &vertex_count, 
                     0.0f, 0.0f, 0.1f, 0.1f,
                     1.0f, 0.0f, 0.0f);  // Red rectangle

ug_add_circle_2d_color(vertices, &vertex_count,
                       0.5f, 0.0f, 0.15f, 0.0f,
                       0.0f, 1.0f, 0.0f, 32);  // Green circle

// 3. Upload to vertex buffer
ug_vertex_buffer_update(vertex_buffer, vertices, vertex_count);

// 4. Render
ug_render_pass_draw(pass, vertex_count);
```

## Key Benefits

✅ **No manual vertex generation** - Helper functions handle the math
✅ **Type-safe** - Uses standard vertex formats
✅ **Works with generic vertex buffers** - No special setup required
✅ **Works with custom pipelines** - Just configure the vertex layout
✅ **Flexible** - Can still define custom vertex formats if needed
✅ **Zero overhead** - Functions are simple and inline-friendly

## Compatibility

These helper functions work seamlessly with:
- Generic `UGVertexBuffer` (accepts any vertex format)
- Custom pipelines (just match the vertex layout)
- Custom shaders (define matching vertex inputs)

You can mix and match:
- Use library helpers for common shapes
- Define custom vertex formats for special needs
- Use multiple vertex buffers with different formats in the same app

