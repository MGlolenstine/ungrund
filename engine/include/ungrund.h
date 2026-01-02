#ifndef UNGRUND_H
#define UNGRUND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <webgpu/webgpu.h>

// Forward declarations
typedef struct UGWindow UGWindow;
typedef struct UGRenderer UGRenderer;
typedef struct UGContext UGContext;
typedef struct UGContextBuilder UGContextBuilder;
typedef struct UGRenderFrame UGRenderFrame;
typedef struct UGPipelineBuilder UGPipelineBuilder;
typedef struct UGPipeline UGPipeline;
typedef struct UGUniformBuffer UGUniformBuffer;
typedef struct UGBindGroupBuilder UGBindGroupBuilder;
typedef struct UGVertexBuffer UGVertexBuffer;
typedef struct UGRenderPass UGRenderPass;
typedef struct UGTexture UGTexture;
typedef struct UGSpriteSheet UGSpriteSheet;

// Window management
UGWindow* ug_window_create(const char* title, int width, int height);
void ug_window_destroy(UGWindow* window);
bool ug_window_should_close(UGWindow* window);
void ug_window_poll_events(UGWindow* window);
void* ug_window_get_native_handle(UGWindow* window);
void* ug_window_get_x11_display(void); // Get X11 display (Linux only)
void ug_window_get_size(UGWindow* window, int* width, int* height);

// WebGPU Context management
// Simple API - creates context with sensible defaults
UGContext* ug_context_create(UGWindow* window);

// Builder API - for customization
UGContextBuilder* ug_context_builder_create(UGWindow* window);
void ug_context_builder_set_power_preference(UGContextBuilder* builder, WGPUPowerPreference preference);
void ug_context_builder_set_present_mode(UGContextBuilder* builder, WGPUPresentMode mode);
void ug_context_builder_set_surface_format(UGContextBuilder* builder, WGPUTextureFormat format);
UGContext* ug_context_builder_build(UGContextBuilder* builder);
void ug_context_builder_destroy(UGContextBuilder* builder);

// Context accessors
UGWindow* ug_context_get_window(UGContext* context);
WGPUDevice ug_context_get_device(UGContext* context);
WGPUQueue ug_context_get_queue(UGContext* context);
WGPUSurface ug_context_get_surface(UGContext* context);
WGPUTextureFormat ug_context_get_surface_format(UGContext* context);
void ug_context_get_surface_size(UGContext* context, uint32_t* width, uint32_t* height);

// Context cleanup
void ug_context_destroy(UGContext* context);

// Render frame management - simplifies render loop boilerplate
UGRenderFrame* ug_begin_render_frame(UGContext* context);
WGPUTextureView ug_render_frame_get_view(UGRenderFrame* frame);
WGPUCommandEncoder ug_render_frame_get_encoder(UGRenderFrame* frame);
void ug_end_render_frame(UGRenderFrame* frame);

// Application loop - callback-based render loop that handles everything
// The render_callback is called each frame with the context, delta_time, and userdata
// delta_time is the time elapsed since the last frame in seconds
// Returns when the window is closed
typedef void (*UGRenderCallback)(UGContext* context, UGRenderFrame* frame, float delta_time, void* userdata);
void ug_run(UGContext* context, UGRenderCallback render_callback, void* userdata);

// Shader utilities
WGPUShaderModule ug_shader_module_create_from_file(WGPUDevice device, const char* filepath, const char* label);
WGPUShaderModule ug_shader_module_create_from_source(WGPUDevice device, const char* source, const char* label);

// File I/O utilities
char* ug_read_file(const char* filepath);
unsigned char* ug_read_binary_file(const char* filepath, size_t* out_size);

// Renderer management (deprecated - use UGContext instead)
UGRenderer* ug_renderer_create(UGWindow* window);
void ug_renderer_destroy(UGRenderer* renderer);
void ug_renderer_begin_frame(UGRenderer* renderer);
void ug_renderer_end_frame(UGRenderer* renderer);
void ug_renderer_clear(UGRenderer* renderer, float r, float g, float b, float a);

// Utility functions
double ug_get_time(void);

// Input handling
bool ug_key_pressed(UGWindow* window, int key);

// Key codes (subset of GLFW keys for common use)
#define UG_KEY_W 87
#define UG_KEY_S 83
#define UG_KEY_I 73
#define UG_KEY_K 75
#define UG_KEY_SPACE 32
#define UG_KEY_ESCAPE 256
#define UG_KEY_UP 265
#define UG_KEY_DOWN 264
#define UG_KEY_LEFT 263
#define UG_KEY_RIGHT 262

// Mouse button enum
typedef enum {
    UG_MOUSE_BUTTON_LEFT = 0,
    UG_MOUSE_BUTTON_RIGHT = 1,
    UG_MOUSE_BUTTON_MIDDLE = 2
} UGMouseButton;

// Input callbacks
// Key callback: called when a key is pressed or released
// Parameters: key code, pressed (true) or released (false), userdata
typedef void (*UGKeyCallback)(int key, bool pressed, void* userdata);

// Mouse move callback: called when the mouse moves within the window
// Parameters: x position, y position, userdata
typedef void (*UGMouseMoveCallback)(double x, double y, void* userdata);

// Mouse button callback: called when a mouse button is pressed or released
// Parameters: button, pressed (true) or released (false), userdata
typedef void (*UGMouseButtonCallback)(UGMouseButton button, bool pressed, void* userdata);

// Set input callbacks
void ug_window_set_key_callback(UGWindow* window, UGKeyCallback callback, void* userdata);
void ug_window_set_mouse_move_callback(UGWindow* window, UGMouseMoveCallback callback, void* userdata);
void ug_window_set_mouse_button_callback(UGWindow* window, UGMouseButtonCallback callback, void* userdata);

// Pipeline builder - simplified pipeline creation
UGPipelineBuilder* ug_pipeline_builder_create(UGContext* context, const char* shader_path);
void ug_pipeline_builder_set_layout(UGPipelineBuilder* builder, WGPUPipelineLayout layout);
void ug_pipeline_builder_set_vertex_buffer(UGPipelineBuilder* builder, WGPUVertexBufferLayout* layout);
void ug_pipeline_builder_enable_blending(UGPipelineBuilder* builder, bool enable);
void ug_pipeline_builder_set_topology(UGPipelineBuilder* builder, WGPUPrimitiveTopology topology);
// New: Add uniforms and textures directly to pipeline builder (auto-creates layouts)
void ug_pipeline_builder_add_uniform(UGPipelineBuilder* builder, uint32_t binding,
                                      UGUniformBuffer* uniform, WGPUShaderStage visibility);
void ug_pipeline_builder_add_texture(UGPipelineBuilder* builder, uint32_t binding,
                                      WGPUTextureView texture_view, WGPUSampler sampler);
WGPUBindGroup ug_pipeline_builder_build_bind_group(UGPipelineBuilder* builder, WGPUBindGroupLayout layout);
WGPURenderPipeline ug_pipeline_builder_build(UGPipelineBuilder* builder);
void ug_pipeline_builder_destroy(UGPipelineBuilder* builder);

// Pipeline wrapper - owns all pipeline-related resources for automatic cleanup
// This is a higher-level API that manages pipeline, layouts, bind groups, and uniforms
UGPipeline* ug_pipeline_create(UGContext* context);
void ug_pipeline_set_render_pipeline(UGPipeline* pipeline, WGPURenderPipeline render_pipeline);
void ug_pipeline_set_pipeline_layout(UGPipeline* pipeline, WGPUPipelineLayout layout);
void ug_pipeline_add_bind_group(UGPipeline* pipeline, WGPUBindGroup bind_group, WGPUBindGroupLayout layout);
void ug_pipeline_add_uniform(UGPipeline* pipeline, UGUniformBuffer* uniform);
WGPURenderPipeline ug_pipeline_get_handle(UGPipeline* pipeline);
WGPUBindGroup ug_pipeline_get_bind_group(UGPipeline* pipeline, size_t index);
void ug_pipeline_destroy(UGPipeline* pipeline);

// Uniform buffer helpers
UGUniformBuffer* ug_uniform_buffer_create(UGContext* context, size_t size);
void ug_uniform_buffer_update(UGUniformBuffer* uniform, const void* data, size_t size);
WGPUBuffer ug_uniform_buffer_get_handle(UGUniformBuffer* uniform);
void ug_uniform_buffer_destroy(UGUniformBuffer* uniform);

// Bind group builder - simplified bind group creation
UGBindGroupBuilder* ug_bind_group_builder_create(UGContext* context);
void ug_bind_group_builder_add_uniform(UGBindGroupBuilder* builder, uint32_t binding,
                                        UGUniformBuffer* uniform, WGPUShaderStage visibility);
void ug_bind_group_builder_add_texture(UGBindGroupBuilder* builder, uint32_t binding,
                                        WGPUTextureView texture_view, WGPUSampler sampler);
WGPUBindGroupLayout ug_bind_group_builder_create_layout(UGBindGroupBuilder* builder);
WGPUBindGroup ug_bind_group_builder_build(UGBindGroupBuilder* builder, WGPUBindGroupLayout layout);
void ug_bind_group_builder_destroy(UGBindGroupBuilder* builder);

// Vertex buffer - simplified dynamic vertex buffer management
// Vertex attribute structure for defining vertex layout
typedef struct {
    WGPUVertexFormat format;
    uint64_t offset;
    uint32_t shader_location;
} UGVertexAttribute;

UGVertexBuffer* ug_vertex_buffer_create(UGContext* context, size_t vertex_size, size_t max_vertices);
void ug_vertex_buffer_set_layout(UGVertexBuffer* vb, const UGVertexAttribute* attributes, size_t attribute_count);
void ug_vertex_buffer_update(UGVertexBuffer* vb, const void* data, size_t vertex_count);
WGPUBuffer ug_vertex_buffer_get_handle(UGVertexBuffer* vb);
WGPUVertexBufferLayout* ug_vertex_buffer_get_layout(UGVertexBuffer* vb);
void ug_vertex_buffer_destroy(UGVertexBuffer* vb);

// Convenience functions for standard vertex formats (auto-sets layout)
UGVertexBuffer* ug_vertex_buffer_create_2d_color(UGContext* context, size_t max_vertices);
UGVertexBuffer* ug_vertex_buffer_create_2d_textured(UGContext* context, size_t max_vertices);

// Render pass - simplified render pass management
UGRenderPass* ug_render_pass_begin(UGRenderFrame* frame, float r, float g, float b, float a);
void ug_render_pass_set_pipeline(UGRenderPass* pass, WGPURenderPipeline pipeline);
void ug_render_pass_set_vertex_buffer(UGRenderPass* pass, UGVertexBuffer* vertex_buffer);
void ug_render_pass_set_bind_group(UGRenderPass* pass, uint32_t group_index, WGPUBindGroup bind_group);
void ug_render_pass_draw(UGRenderPass* pass, uint32_t vertex_count);
void ug_render_pass_draw_indexed(UGRenderPass* pass, uint32_t index_count);
void ug_render_pass_end(UGRenderPass* pass);

// Geometry helpers - standard vertex formats and primitive generation
// Standard 2D vertex format: position (vec2) + color (vec3)
typedef struct {
    float position[2];
    float color[3];
} UGVertex2DColor;

// Standard 2D textured vertex format: position (vec2) + UV (vec2)
typedef struct {
    float position[2];
    float uv[2];
} UGVertex2DTextured;

// Add a rectangle to a vertex array (2D position + color format)
// x, y: center position, w, h: half-width and half-height
// r, g, b: color components (0.0 to 1.0)
void ug_add_rect_2d_color(UGVertex2DColor* vertices, size_t* count,
                          float x, float y, float w, float h,
                          float r, float g, float b);

// Add a circle or ellipse to a vertex array (2D position + color format)
// x, y: center position
// width: radius in x direction (or radius for perfect circle if height is 0)
// height: radius in y direction (if 0, uses width for both to create perfect circle)
// r, g, b: color components (0.0 to 1.0)
// segments: number of triangular segments (higher = smoother, minimum 3)
void ug_add_circle_2d_color(UGVertex2DColor* vertices, size_t* count,
                            float x, float y, float width, float height,
                            float r, float g, float b, int segments);

// Add a rectangle to a vertex array (2D position + UV format)
// x, y: center position, w, h: half-width and half-height
// u0, v0, u1, v1: texture coordinates for the rectangle
void ug_add_rect_2d_textured(UGVertex2DTextured* vertices, size_t* count,
                             float x, float y, float w, float h,
                             float u0, float v0, float u1, float v1);

// Add a circle or ellipse to a vertex array (2D position + UV format)
// x, y: center position
// width: radius in x direction (or radius for perfect circle if height is 0)
// height: radius in y direction (if 0, uses width for both to create perfect circle)
// u0, v0, u1, v1: texture coordinate bounds
// segments: number of triangular segments (higher = smoother, minimum 3)
void ug_add_circle_2d_textured(UGVertex2DTextured* vertices, size_t* count,
                               float x, float y, float width, float height,
                               float u0, float v0, float u1, float v1,
                               int segments);

// Texture loading - simplified image loading and texture creation
// Load a texture from an image file (supports PNG, JPG, BMP, TGA, etc.)
// filepath: Path to the image file
// Returns NULL on failure
UGTexture* ug_texture_create_from_file(UGContext* context, const char* filepath);

// Destroy texture and free all resources
void ug_texture_destroy(UGTexture* texture);

// Get the texture view for binding to shaders
WGPUTextureView ug_texture_get_view(UGTexture* texture);

// Get the sampler for texture sampling
WGPUSampler ug_texture_get_sampler(UGTexture* texture);

// Get texture dimensions
void ug_texture_get_size(UGTexture* texture, int* width, int* height);

// Sprite Sheet - sprite animation system for 2D games
// Create a sprite sheet from a texture
// texture: The texture containing the sprite sheet
// sprite_width: Width of each sprite in pixels
// sprite_height: Height of each sprite in pixels
// The sprite sheet is organized in a grid from left-to-right, top-to-bottom
// Sprite index 0 is top-left, incrementing across rows
UGSpriteSheet* ug_sprite_sheet_create(UGTexture* texture, int sprite_width, int sprite_height);

// Destroy sprite sheet (does not destroy the texture)
void ug_sprite_sheet_destroy(UGSpriteSheet* sheet);

// Add a sprite to a vertex array
// sheet: The sprite sheet
// vertices: Pointer to vertex array (UGVertex2DTextured format)
// count: Pointer to current vertex count (will be incremented by 6)
// sprite_index: Index of the sprite to render (0-based, left-to-right, top-to-bottom)
// x, y: Center position in NDC or world space
// w, h: Half-width and half-height
void ug_sprite_sheet_add_sprite(UGSpriteSheet* sheet, UGVertex2DTextured* vertices, size_t* count,
                                int sprite_index, float x, float y, float w, float h);

// Get the texture associated with this sprite sheet
UGTexture* ug_sprite_sheet_get_texture(UGSpriteSheet* sheet);

// Get the size of individual sprites in the sheet
void ug_sprite_sheet_get_sprite_size(UGSpriteSheet* sheet, int* width, int* height);

// Get the total number of sprites in the sheet
int ug_sprite_sheet_get_sprite_count(UGSpriteSheet* sheet);

// Font Atlas - simplified text rendering system
// Handles font loading, atlas generation, and provides pre-configured pipeline/bind group
typedef struct UGFontAtlas UGFontAtlas;

// Create a font atlas from a TrueType font file
// font_path: Path to .ttf or .otf font file
// font_size: Font size in pixels
// atlas_width, atlas_height: Size of the texture atlas (e.g., 512x512)
// Returns NULL on failure
UGFontAtlas* ug_font_atlas_create(UGContext* context, const char* font_path,
                                   int font_size, int atlas_width, int atlas_height);

// Destroy font atlas and free all resources
void ug_font_atlas_destroy(UGFontAtlas* atlas);

// Get the pre-configured render pipeline for text rendering
WGPURenderPipeline ug_font_atlas_get_pipeline(UGFontAtlas* atlas);

// Get the pre-configured bind group (contains texture and sampler)
WGPUBindGroup ug_font_atlas_get_bind_group(UGFontAtlas* atlas);

// Add text to a vertex buffer using pixel coordinates (recommended)
// vertices: Pointer to vertex array (must be large enough)
// count: Pointer to current vertex count (will be incremented)
// text: UTF-8 text string to render
// x, y: Position in pixel coordinates (origin at top-left, y-down)
// context: Context to get window dimensions for coordinate conversion
// r, g, b, a: Text color (0.0 to 1.0)
// Note: Each character adds 6 vertices (2 triangles)
void ug_font_atlas_add_text_px(UGFontAtlas* atlas, void* vertices, size_t* count,
                               const char* text, float x, float y, UGContext* context,
                               float r, float g, float b, float a);

// Add text to a vertex buffer using NDC coordinates (advanced)
// vertices: Pointer to vertex array (must be large enough)
// count: Pointer to current vertex count (will be incremented)
// text: UTF-8 text string to render
// x, y: Position in NDC space (-1 to 1)
// pixel_height: Height of one pixel in NDC space (typically 2.0 / screen_height)
// r, g, b, a: Text color (0.0 to 1.0)
// Note: Each character adds 6 vertices (2 triangles)
void ug_font_atlas_add_text(UGFontAtlas* atlas, void* vertices, size_t* count,
                            const char* text, float x, float y, float pixel_height,
                            float r, float g, float b, float a);

// Get the vertex size for text rendering (for creating vertex buffers)
size_t ug_font_atlas_get_vertex_size(void);

// Get vertex attributes for text rendering (for setting up vertex buffer layout)
// attributes: Array of 3 UGVertexAttribute structures to fill
void ug_font_atlas_get_vertex_attributes(UGVertexAttribute* attributes);

// Platform-specific helpers
#if defined(__APPLE__)
void* ug_create_metal_layer(void* ns_window_ptr);
#endif

#ifdef __cplusplus
}
#endif

#endif // UNGRUND_H


