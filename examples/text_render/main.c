#include "ungrund.h"
#include <webgpu/webgpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

// Maximum number of quads (characters/text blocks) we can batch
#define MAX_QUADS 256
#define MAX_VERTICES (MAX_QUADS * 6)  // 6 vertices per quad (2 triangles)

typedef struct {
    float x, y;
    float u, v;
} Vertex;

// Uniform structure matching the shader
typedef struct {
    float text_color[4];      // RGBA color for text
    float outline_color[4];   // RGBA color for outline
    uint32_t render_mode;     // 0=standard, 1=outline, 2=shadow, 3=glow
    float outline_width;      // Width of outline in pixels
    float shadow_offset[2];   // Shadow offset in texture space
    float padding[2];         // Padding to align to 16 bytes
} RenderUniforms;

typedef struct {
    WGPURenderPipeline pipeline;
    WGPUBindGroup bind_group;
    WGPUBuffer vertex_buffer;
    UGUniformBuffer* uniform;
    WGPUQueue queue;
    size_t vertex_count;
    RenderUniforms uniforms;
} RenderData;

// Helper function to create a quad (2 triangles) for batching
void add_quad_to_batch(Vertex* vertices, size_t* vertex_count,
                       float x, float y, float w, float h,
                       float u0, float v0, float u1, float v1) {
    size_t idx = *vertex_count;

    // Triangle 1
    vertices[idx++] = (Vertex){x,     y,     u0, v0};  // Top-left
    vertices[idx++] = (Vertex){x,     y + h, u0, v1};  // Bottom-left
    vertices[idx++] = (Vertex){x + w, y + h, u1, v1};  // Bottom-right

    // Triangle 2
    vertices[idx++] = (Vertex){x,     y,     u0, v0};  // Top-left
    vertices[idx++] = (Vertex){x + w, y + h, u1, v1};  // Bottom-right
    vertices[idx++] = (Vertex){x + w, y,     u1, v0};  // Top-right

    *vertex_count = idx;
}

// Render callback - called each frame
void render(UGContext* context, UGRenderFrame* frame, float delta_time, void* userdata) {
    RenderData* data = (RenderData*)userdata;
    (void)context;
    (void)delta_time;  // Not used in this example

    // Get view and encoder from frame
    WGPUTextureView view = ug_render_frame_get_view(frame);
    WGPUCommandEncoder encoder = ug_render_frame_get_encoder(frame);

    // Animate rendering mode and parameters based on time
    float time = (float)ug_get_time();

    // Cycle through rendering modes every 3 seconds
    data->uniforms.render_mode = ((uint32_t)(time / 3.0f)) % 4;

    // Animate colors
    data->uniforms.text_color[0] = 0.5f + 0.5f * sinf(time * 0.5f);
    data->uniforms.text_color[1] = 0.5f + 0.5f * sinf(time * 0.7f);
    data->uniforms.text_color[2] = 0.5f + 0.5f * sinf(time * 0.9f);
    data->uniforms.text_color[3] = 1.0f;

    // Outline color (complementary)
    data->uniforms.outline_color[0] = 1.0f - data->uniforms.text_color[0];
    data->uniforms.outline_color[1] = 1.0f - data->uniforms.text_color[1];
    data->uniforms.outline_color[2] = 1.0f - data->uniforms.text_color[2];
    data->uniforms.outline_color[3] = 1.0f;

    // Animate outline width
    data->uniforms.outline_width = 1.5f + 0.5f * sinf(time * 2.0f);

    // Animate shadow offset
    data->uniforms.shadow_offset[0] = 0.02f * cosf(time);
    data->uniforms.shadow_offset[1] = 0.02f * sinf(time);

    // Update uniform buffer
    ug_uniform_buffer_update(data->uniform, &data->uniforms, sizeof(RenderUniforms));

    // Build dynamic vertex buffer with multiple quads (batch rendering demonstration)
    Vertex vertices[MAX_VERTICES];
    size_t vertex_count = 0;

    // Main text quad (original position)
    add_quad_to_batch(vertices, &vertex_count, -0.8f, 0.5f, 1.6f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

    // Add smaller duplicate quads to demonstrate batching
    float scale = 0.3f;
    add_quad_to_batch(vertices, &vertex_count,
                      -0.9f, -0.8f, 1.6f * scale, -1.0f * scale,
                      0.0f, 0.0f, 1.0f, 1.0f);

    add_quad_to_batch(vertices, &vertex_count,
                      0.3f, -0.8f, 1.6f * scale, -1.0f * scale,
                      0.0f, 0.0f, 1.0f, 1.0f);

    // Update dynamic vertex buffer
    wgpuQueueWriteBuffer(data->queue, data->vertex_buffer, 0, vertices, vertex_count * sizeof(Vertex));
    data->vertex_count = vertex_count;

    // Setup render pass
    WGPURenderPassColorAttachment color_attachment = {
        .view = view,
        .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
        .loadOp = WGPULoadOp_Clear,
        .storeOp = WGPUStoreOp_Store,
        .clearValue = {0.2, 0.2, 0.3, 1.0},
    };

    WGPURenderPassDescriptor render_pass_desc = {
        .colorAttachmentCount = 1,
        .colorAttachments = &color_attachment,
    };

    // Render - single draw call for all batched quads
    WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &render_pass_desc);
    wgpuRenderPassEncoderSetPipeline(pass, data->pipeline);
    wgpuRenderPassEncoderSetBindGroup(pass, 0, data->bind_group, 0, NULL);
    wgpuRenderPassEncoderSetVertexBuffer(pass, 0, data->vertex_buffer, 0, data->vertex_count * sizeof(Vertex));
    wgpuRenderPassEncoderDraw(pass, data->vertex_count, 1, 0, 0);
    wgpuRenderPassEncoderEnd(pass);
    wgpuRenderPassEncoderRelease(pass);
}

int main(void) {
    // Create window
    UGWindow* window = ug_window_create("Text Rendering Example", 800, 600);
    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        return 1;
    }

    // Initialize stb_truetype
    stbtt_fontinfo font;
    size_t font_size;

    // Try to load a system font - on macOS, use SF Pro or Helvetica
    unsigned char* font_buffer = NULL;

#if defined(__APPLE__)
    font_buffer = ug_read_binary_file("/System/Library/Fonts/Helvetica.ttc", &font_size);
#elif defined(_WIN32)
    font_buffer = ug_read_binary_file("C:/Windows/Fonts/arial.ttf", &font_size);
#else
    font_buffer = ug_read_binary_file("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", &font_size);
#endif
    
    if (!font_buffer) {
        fprintf(stderr, "Failed to load font file\n");
        return 1;
    }
    
    if (!stbtt_InitFont(&font, font_buffer, stbtt_GetFontOffsetForIndex(font_buffer, 0))) {
        fprintf(stderr, "Failed to initialize font\n");
        return 1;
    }
    
    // Create font bitmap
    const char* text = "Hello World";
    float font_height = 64.0f;
    float scale = stbtt_ScaleForPixelHeight(&font, font_height);
    
    int bitmap_width = 512;
    int bitmap_height = 128;
    unsigned char* bitmap = (unsigned char*)calloc(bitmap_width * bitmap_height, 1);
    
    // Render text to bitmap
    int x = 10;
    int ascent, descent, line_gap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);
    int baseline = (int)(ascent * scale);
    
    for (const char* p = text; *p; p++) {
        int advance, lsb;
        stbtt_GetCodepointHMetrics(&font, *p, &advance, &lsb);
        
        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&font, *p, scale, scale, &x0, &y0, &x1, &y1);
        
        stbtt_MakeCodepointBitmap(&font, bitmap + x + (int)(lsb * scale) + (baseline + y0) * bitmap_width,
                                  x1 - x0, y1 - y0, bitmap_width, scale, scale, *p);
        
        x += (int)(advance * scale);
        
        if (p[1]) {
            x += (int)(scale * stbtt_GetCodepointKernAdvance(&font, p[0], p[1]));
        }
    }

    // Create WebGPU context with default settings
    UGContext* context = ug_context_create(window);
    if (!context) {
        fprintf(stderr, "Failed to create WebGPU context\n");
        free(bitmap);
        free(font_buffer);
        ug_window_destroy(window);
        return 1;
    }

    // Get device and queue for creating resources
    WGPUDevice device = ug_context_get_device(context);
    WGPUQueue queue = ug_context_get_queue(context);

    // Create uniform buffer for rendering parameters
    RenderUniforms uniforms = {
        .text_color = {1.0f, 1.0f, 1.0f, 1.0f},
        .outline_color = {0.0f, 0.0f, 0.0f, 1.0f},
        .render_mode = 0,
        .outline_width = 2.0f,
        .shadow_offset = {0.02f, 0.02f},
    };

    UGUniformBuffer* uniform = ug_uniform_buffer_create(context, sizeof(RenderUniforms));

    // Create texture for font bitmap
    WGPUTextureDescriptor texture_desc = {
        .size = {bitmap_width, bitmap_height, 1},
        .format = WGPUTextureFormat_R8Unorm,
        .usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst,
        .dimension = WGPUTextureDimension_2D,
        .mipLevelCount = 1,
        .sampleCount = 1,
    };
    WGPUTexture font_texture = wgpuDeviceCreateTexture(device, &texture_desc);

    // Upload bitmap to texture
    WGPUTexelCopyTextureInfo dest = {
        .texture = font_texture,
        .mipLevel = 0,
        .origin = {0, 0, 0},
        .aspect = WGPUTextureAspect_All,
    };

    WGPUTexelCopyBufferLayout data_layout = {
        .offset = 0,
        .bytesPerRow = bitmap_width,
        .rowsPerImage = bitmap_height,
    };

    wgpuQueueWriteTexture(queue, &dest, bitmap, bitmap_width * bitmap_height, &data_layout, &texture_desc.size);

    WGPUTextureView texture_view = wgpuTextureCreateView(font_texture, NULL);

    // Create sampler
    WGPUSamplerDescriptor sampler_desc = {
        .addressModeU = WGPUAddressMode_ClampToEdge,
        .addressModeV = WGPUAddressMode_ClampToEdge,
        .magFilter = WGPUFilterMode_Linear,
        .minFilter = WGPUFilterMode_Linear,
        .maxAnisotropy = 1,
    };
    WGPUSampler sampler = wgpuDeviceCreateSampler(device, &sampler_desc);

    // Build bind group with texture and uniform
    UGBindGroupBuilder* bg_builder = ug_bind_group_builder_create(context);
    ug_bind_group_builder_add_texture(bg_builder, 0, texture_view, sampler);
    ug_bind_group_builder_add_uniform(bg_builder, 2, uniform, WGPUShaderStage_Fragment);

    WGPUBindGroupLayout bind_group_layout = ug_bind_group_builder_create_layout(bg_builder);
    WGPUBindGroup bind_group = ug_bind_group_builder_build(bg_builder, bind_group_layout);

    // Create pipeline layout
    WGPUPipelineLayoutDescriptor pipeline_layout_desc = {
        .bindGroupLayoutCount = 1,
        .bindGroupLayouts = &bind_group_layout,
    };
    WGPUPipelineLayout pipeline_layout = wgpuDeviceCreatePipelineLayout(device, &pipeline_layout_desc);

    // Create dynamic vertex buffer
    WGPUBufferDescriptor vertex_buffer_desc = {
        .size = MAX_VERTICES * sizeof(Vertex),
        .usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst,
        .mappedAtCreation = false,
    };
    WGPUBuffer vertex_buffer = wgpuDeviceCreateBuffer(device, &vertex_buffer_desc);

    // Define vertex layout
    WGPUVertexAttribute vertex_attributes[2] = {
        {.format = WGPUVertexFormat_Float32x2, .offset = 0, .shaderLocation = 0},
        {.format = WGPUVertexFormat_Float32x2, .offset = 2 * sizeof(float), .shaderLocation = 1},
    };

    WGPUVertexBufferLayout vertex_buffer_layout = {
        .arrayStride = sizeof(Vertex),
        .stepMode = WGPUVertexStepMode_Vertex,
        .attributeCount = 2,
        .attributes = vertex_attributes,
    };

    // Build pipeline with blending enabled
    UGPipelineBuilder* pipeline_builder = ug_pipeline_builder_create(context, "examples/text_render/text.wgsl");
    if (!pipeline_builder) {
        fprintf(stderr, "Failed to create pipeline builder\n");
        return 1;
    }

    ug_pipeline_builder_set_layout(pipeline_builder, pipeline_layout);
    ug_pipeline_builder_set_vertex_buffer(pipeline_builder, &vertex_buffer_layout);
    ug_pipeline_builder_enable_blending(pipeline_builder, true);

    WGPURenderPipeline pipeline = ug_pipeline_builder_build(pipeline_builder);

    printf("Text rendering with dynamic batching and ubershader.\n");
    printf("Modes cycle every 3s: Standard→Outline→Shadow→Glow. Press ESC to exit.\n");

    // Setup render data
    RenderData render_data = {
        .pipeline = pipeline,
        .bind_group = bind_group,
        .vertex_buffer = vertex_buffer,
        .uniform = uniform,
        .queue = queue,
        .vertex_count = 0,
        .uniforms = uniforms,
    };

    // Run
    ug_run(context, render, &render_data);

    // Cleanup
    ug_uniform_buffer_destroy(uniform);
    ug_bind_group_builder_destroy(bg_builder);
    wgpuBufferRelease(vertex_buffer);
    wgpuRenderPipelineRelease(pipeline);
    ug_pipeline_builder_destroy(pipeline_builder);
    wgpuPipelineLayoutRelease(pipeline_layout);
    wgpuBindGroupRelease(bind_group);
    wgpuBindGroupLayoutRelease(bind_group_layout);
    wgpuSamplerRelease(sampler);
    wgpuTextureViewRelease(texture_view);
    wgpuTextureRelease(font_texture);
    ug_context_destroy(context);
    free(bitmap);
    free(font_buffer);
    ug_window_destroy(window);

    return 0;
}

