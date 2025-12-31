# Geometry Helper Functions

This document describes the geometry helper functions added to the Ungrund library for easy generation of common 2D shapes.

## Overview

The geometry helpers provide a simple API for generating vertex data for rectangles and circles/ellipses. These functions work seamlessly with the library's generic vertex buffer system and custom pipelines.

## Standard Vertex Formats

### UGVertex2DColor

For colored geometry (position + color):

```c
typedef struct {
    float position[2];  // x, y coordinates
    float color[3];     // r, g, b (0.0 to 1.0)
} UGVertex2DColor;
```

**Vertex Layout:**
```c
UGVertexAttribute attrs[2] = {
    {.format = WGPUVertexFormat_Float32x2, .offset = 0, .shader_location = 0},
    {.format = WGPUVertexFormat_Float32x3, .offset = 8, .shader_location = 1},
};
```

**Shader Input:**
```wgsl
struct VertexInput {
    @location(0) position: vec2<f32>,
    @location(1) color: vec3<f32>,
};
```

### UGVertex2DTextured

For textured geometry (position + UV):

```c
typedef struct {
    float position[2];  // x, y coordinates
    float uv[2];        // texture coordinates
} UGVertex2DTextured;
```

**Vertex Layout:**
```c
UGVertexAttribute attrs[2] = {
    {.format = WGPUVertexFormat_Float32x2, .offset = 0, .shader_location = 0},
    {.format = WGPUVertexFormat_Float32x2, .offset = 8, .shader_location = 1},
};
```

## Functions

### ug_add_rect_2d_color

Add a colored rectangle to a vertex array.

```c
void ug_add_rect_2d_color(UGVertex2DColor* vertices, size_t* count,
                          float x, float y, float w, float h,
                          float r, float g, float b);
```

**Parameters:**
- `vertices`: Pointer to vertex array
- `count`: Pointer to current vertex count (will be incremented by 6)
- `x, y`: Center position of the rectangle
- `w, h`: Half-width and half-height
- `r, g, b`: Color components (0.0 to 1.0)

**Example:**
```c
UGVertex2DColor vertices[1024];
size_t count = 0;

// Red rectangle at origin, 0.2 units wide, 0.3 units tall
ug_add_rect_2d_color(vertices, &count, 0.0f, 0.0f, 0.1f, 0.15f, 1.0f, 0.0f, 0.0f);
```

### ug_add_circle_2d_color

Add a colored circle or ellipse to a vertex array.

```c
void ug_add_circle_2d_color(UGVertex2DColor* vertices, size_t* count,
                            float x, float y, float width, float height,
                            float r, float g, float b, int segments);
```

**Parameters:**
- `vertices`: Pointer to vertex array
- `count`: Pointer to current vertex count (will be incremented by `segments * 3`)
- `x, y`: Center position
- `width`: Radius in x direction
- `height`: Radius in y direction (if 0, uses `width` for both → perfect circle)
- `r, g, b`: Color components (0.0 to 1.0)
- `segments`: Number of triangular segments (minimum 3, typical: 24-48)

**Examples:**
```c
UGVertex2DColor vertices[1024];
size_t count = 0;

// Perfect green circle (height = 0 means use width for both)
ug_add_circle_2d_color(vertices, &count, 0.5f, 0.0f, 0.15f, 0.0f, 
                       0.0f, 1.0f, 0.0f, 32);

// Blue ellipse (different width and height)
ug_add_circle_2d_color(vertices, &count, -0.5f, 0.0f, 0.2f, 0.1f,
                       0.0f, 0.0f, 1.0f, 32);
```

### ug_add_rect_2d_textured

Add a textured rectangle to a vertex array.

```c
void ug_add_rect_2d_textured(UGVertex2DTextured* vertices, size_t* count,
                             float x, float y, float w, float h,
                             float u0, float v0, float u1, float v1);
```

**Parameters:**
- `vertices`: Pointer to vertex array
- `count`: Pointer to current vertex count (will be incremented by 6)
- `x, y`: Center position
- `w, h`: Half-width and half-height
- `u0, v0`: Top-left texture coordinate
- `u1, v1`: Bottom-right texture coordinate

**Example:**
```c
UGVertex2DTextured vertices[1024];
size_t count = 0;

// Textured rectangle using full texture (0,0) to (1,1)
ug_add_rect_2d_textured(vertices, &count, 0.0f, 0.0f, 0.5f, 0.5f,
                        0.0f, 0.0f, 1.0f, 1.0f);
```

### ug_add_circle_2d_textured

Add a textured circle or ellipse to a vertex array.

```c
void ug_add_circle_2d_textured(UGVertex2DTextured* vertices, size_t* count,
                               float x, float y, float width, float height,
                               float u0, float v0, float u1, float v1,
                               int segments);
```

**Parameters:**
- Same as `ug_add_circle_2d_color`, plus texture coordinates
- `u0, v0, u1, v1`: Texture coordinate bounds

## Complete Usage Example

```c
#include "ungrund.h"

#define MAX_VERTICES 4096

int main(void) {
    // Setup window and context
    UGWindow* window = ug_window_create("Geometry Demo", 800, 600);
    UGContext* context = ug_context_create(window);
    
    // Create vertex buffer with UGVertex2DColor format
    UGVertexBuffer* vb = ug_vertex_buffer_create(context, sizeof(UGVertex2DColor), MAX_VERTICES);
    
    // Set vertex layout
    UGVertexAttribute attrs[2] = {
        {.format = WGPUVertexFormat_Float32x2, .offset = 0, .shader_location = 0},
        {.format = WGPUVertexFormat_Float32x3, .offset = 8, .shader_location = 1},
    };
    ug_vertex_buffer_set_layout(vb, attrs, 2);
    
    // Build pipeline (shader must match vertex format)
    UGPipelineBuilder* pb = ug_pipeline_builder_create(context, "shader.wgsl");
    ug_pipeline_builder_set_vertex_buffer(pb, ug_vertex_buffer_get_layout(vb));
    WGPURenderPipeline pipeline = ug_pipeline_builder_build(pb);
    
    // In render loop:
    UGVertex2DColor vertices[MAX_VERTICES];
    size_t count = 0;
    
    // Add shapes
    ug_add_rect_2d_color(vertices, &count, -0.5f, 0.0f, 0.1f, 0.1f, 1.0f, 0.0f, 0.0f);
    ug_add_circle_2d_color(vertices, &count, 0.5f, 0.0f, 0.15f, 0.0f, 0.0f, 1.0f, 0.0f, 32);
    
    // Upload and render
    ug_vertex_buffer_update(vb, vertices, count);
    
    UGRenderPass* pass = ug_render_pass_begin(frame, 0.0f, 0.0f, 0.0f, 1.0f);
    ug_render_pass_set_pipeline(pass, pipeline);
    ug_render_pass_set_vertex_buffer(pass, vb);
    ug_render_pass_draw(pass, count);
    ug_render_pass_end(pass);
    
    return 0;
}
```

## Design Philosophy

These helper functions follow the library's design principles:

1. **Decoupled from GPU resources** - Functions just generate vertex data in memory
2. **Works with generic vertex buffers** - `ug_vertex_buffer_update()` accepts any format
3. **Compatible with custom pipelines** - Just match the vertex layout in your shader
4. **Zero overhead** - Simple, inline-friendly functions
5. **Optional** - You can still define custom vertex formats and generation functions

## See Also

- `examples/geometry_demo/` - Complete working example
- `examples/pong/` - Uses similar pattern with custom vertex type
- `engine/src/geometry.c` - Implementation source code

