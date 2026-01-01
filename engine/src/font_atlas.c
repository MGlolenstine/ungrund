#include "ungrund.h"
#include <webgpu/webgpu.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "../third_party/stb_truetype.h"

// Default text shader embedded as string
static const char* DEFAULT_TEXT_SHADER = 
"struct VertexInput {\n"
"    @location(0) position: vec2f,\n"
"    @location(1) uv: vec2f,\n"
"    @location(2) color: vec4f,\n"
"};\n"
"\n"
"struct VertexOutput {\n"
"    @builtin(position) position: vec4f,\n"
"    @location(0) uv: vec2f,\n"
"    @location(1) color: vec4f,\n"
"};\n"
"\n"
"@vertex\n"
"fn vs_main(in: VertexInput) -> VertexOutput {\n"
"    var out: VertexOutput;\n"
"    out.position = vec4f(in.position, 0.0, 1.0);\n"
"    out.uv = in.uv;\n"
"    out.color = in.color;\n"
"    return out;\n"
"}\n"
"\n"
"@group(0) @binding(0) var font_texture: texture_2d<f32>;\n"
"@group(0) @binding(1) var font_sampler: sampler;\n"
"\n"
"@fragment\n"
"fn fs_main(in: VertexOutput) -> @location(0) vec4f {\n"
"    let alpha = textureSample(font_texture, font_sampler, in.uv).r;\n"
"    return vec4f(in.color.rgb, in.color.a * alpha);\n"
"}\n";

// Glyph information
typedef struct {
    int codepoint;
    float x0, y0, x1, y1;  // Atlas coordinates (pixels)
    float xoff, yoff;       // Offset from cursor position
    float xadvance;         // Horizontal advance
} GlyphInfo;

// Font atlas structure
struct UGFontAtlas {
    WGPUDevice device;
    WGPUQueue queue;
    WGPUTexture texture;
    WGPUTextureView texture_view;
    WGPUSampler sampler;
    WGPUBindGroup bind_group;
    WGPURenderPipeline pipeline;
    
    int atlas_width;
    int atlas_height;
    int font_size;
    float scale;
    
    GlyphInfo* glyphs;
    int glyph_count;
    
    stbtt_fontinfo font_info;
    unsigned char* font_data;
};

// Vertex format for text rendering: position (vec2) + uv (vec2) + color (vec4)
typedef struct {
    float position[2];
    float uv[2];
    float color[4];
} TextVertex;

UGFontAtlas* ug_font_atlas_create(UGContext* context, const char* font_path, 
                                   int font_size, int atlas_width, int atlas_height) {
    if (!context || !font_path || font_size <= 0 || atlas_width <= 0 || atlas_height <= 0) {
        return NULL;
    }
    
    UGFontAtlas* atlas = (UGFontAtlas*)calloc(1, sizeof(UGFontAtlas));
    if (!atlas) {
        return NULL;
    }
    
    atlas->device = ug_context_get_device(context);
    atlas->queue = ug_context_get_queue(context);
    atlas->atlas_width = atlas_width;
    atlas->atlas_height = atlas_height;
    atlas->font_size = font_size;
    
    // Load font file
    size_t font_file_size;
    atlas->font_data = ug_read_binary_file(font_path, &font_file_size);
    if (!atlas->font_data) {
        fprintf(stderr, "Failed to load font file: %s\n", font_path);
        free(atlas);
        return NULL;
    }
    
    // Initialize font
    // For .ttc files (TrueType Collection), we need to get the offset for the first font
    int font_offset = stbtt_GetFontOffsetForIndex(atlas->font_data, 0);
    if (!stbtt_InitFont(&atlas->font_info, atlas->font_data, font_offset)) {
        fprintf(stderr, "Failed to initialize font\n");
        free(atlas->font_data);
        free(atlas);
        return NULL;
    }
    
    // Calculate scale for desired font size
    atlas->scale = stbtt_ScaleForPixelHeight(&atlas->font_info, (float)font_size);
    
    // Generate atlas bitmap for ASCII printable characters (32-126)
    int first_char = 32;
    int num_chars = 95;  // ASCII printable characters
    atlas->glyph_count = num_chars;
    atlas->glyphs = (GlyphInfo*)calloc(num_chars, sizeof(GlyphInfo));
    
    // Create bitmap for atlas
    unsigned char* bitmap = (unsigned char*)calloc(atlas_width * atlas_height, 1);
    
    // Pack glyphs into atlas
    int x = 2, y = 2;  // Start with padding
    int row_height = 0;
    
    for (int i = 0; i < num_chars; i++) {
        int codepoint = first_char + i;
        
        // Get glyph metrics
        int advance, lsb;
        stbtt_GetCodepointHMetrics(&atlas->font_info, codepoint, &advance, &lsb);
        
        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&atlas->font_info, codepoint, atlas->scale, atlas->scale, &x0, &y0, &x1, &y1);
        
        int glyph_width = x1 - x0;
        int glyph_height = y1 - y0;

        // Check if we need to move to next row
        if (x + glyph_width + 2 > atlas_width) {
            x = 2;
            y += row_height + 2;
            row_height = 0;
        }

        // Check if we're out of vertical space
        if (y + glyph_height + 2 > atlas_height) {
            fprintf(stderr, "Warning: Font atlas too small, some glyphs may be missing\n");
            break;
        }

        // Render glyph to bitmap
        stbtt_MakeCodepointBitmap(&atlas->font_info,
                                  bitmap + x + y * atlas_width,
                                  glyph_width, glyph_height,
                                  atlas_width,
                                  atlas->scale, atlas->scale,
                                  codepoint);

        // Store glyph info
        atlas->glyphs[i].codepoint = codepoint;
        atlas->glyphs[i].x0 = (float)x;
        atlas->glyphs[i].y0 = (float)y;
        atlas->glyphs[i].x1 = (float)(x + glyph_width);
        atlas->glyphs[i].y1 = (float)(y + glyph_height);
        atlas->glyphs[i].xoff = (float)x0;
        atlas->glyphs[i].yoff = (float)y0;
        atlas->glyphs[i].xadvance = (float)advance * atlas->scale;

        // Update position
        x += glyph_width + 2;
        if (glyph_height > row_height) {
            row_height = glyph_height;
        }
    }

    // Create texture
    WGPUTextureDescriptor texture_desc = {
        .size = {atlas_width, atlas_height, 1},
        .format = WGPUTextureFormat_R8Unorm,
        .usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst,
        .dimension = WGPUTextureDimension_2D,
        .mipLevelCount = 1,
        .sampleCount = 1,
    };
    atlas->texture = wgpuDeviceCreateTexture(atlas->device, &texture_desc);

    // Upload bitmap to texture
    WGPUTexelCopyTextureInfo dest = {
        .texture = atlas->texture,
        .mipLevel = 0,
        .origin = {0, 0, 0},
        .aspect = WGPUTextureAspect_All,
    };

    WGPUTexelCopyBufferLayout data_layout = {
        .offset = 0,
        .bytesPerRow = atlas_width,
        .rowsPerImage = atlas_height,
    };

    WGPUExtent3D write_size = {atlas_width, atlas_height, 1};
    wgpuQueueWriteTexture(atlas->queue, &dest, bitmap, atlas_width * atlas_height, &data_layout, &write_size);

    free(bitmap);

    // Create texture view
    WGPUTextureViewDescriptor view_desc = {
        .format = WGPUTextureFormat_R8Unorm,
        .dimension = WGPUTextureViewDimension_2D,
        .baseMipLevel = 0,
        .mipLevelCount = 1,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .aspect = WGPUTextureAspect_All,
    };
    atlas->texture_view = wgpuTextureCreateView(atlas->texture, &view_desc);

    // Create sampler
    WGPUSamplerDescriptor sampler_desc = {
        .addressModeU = WGPUAddressMode_ClampToEdge,
        .addressModeV = WGPUAddressMode_ClampToEdge,
        .addressModeW = WGPUAddressMode_ClampToEdge,
        .magFilter = WGPUFilterMode_Linear,
        .minFilter = WGPUFilterMode_Linear,
        .mipmapFilter = WGPUMipmapFilterMode_Nearest,
        .lodMinClamp = 0.0f,
        .lodMaxClamp = 1.0f,
        .compare = WGPUCompareFunction_Undefined,
        .maxAnisotropy = 1,
    };
    atlas->sampler = wgpuDeviceCreateSampler(atlas->device, &sampler_desc);

    // Create bind group layout
    WGPUBindGroupLayoutEntry layout_entries[2] = {
        {
            .binding = 0,
            .visibility = WGPUShaderStage_Fragment,
            .texture = {
                .sampleType = WGPUTextureSampleType_Float,
                .viewDimension = WGPUTextureViewDimension_2D,
                .multisampled = false,
            },
        },
        {
            .binding = 1,
            .visibility = WGPUShaderStage_Fragment,
            .sampler = {
                .type = WGPUSamplerBindingType_Filtering,
            },
        },
    };

    WGPUBindGroupLayoutDescriptor bg_layout_desc = {
        .entryCount = 2,
        .entries = layout_entries,
    };
    WGPUBindGroupLayout bind_group_layout = wgpuDeviceCreateBindGroupLayout(atlas->device, &bg_layout_desc);

    // Create bind group
    WGPUBindGroupEntry bind_entries[2] = {
        {
            .binding = 0,
            .textureView = atlas->texture_view,
        },
        {
            .binding = 1,
            .sampler = atlas->sampler,
        },
    };

    WGPUBindGroupDescriptor bind_group_desc = {
        .layout = bind_group_layout,
        .entryCount = 2,
        .entries = bind_entries,
    };
    atlas->bind_group = wgpuDeviceCreateBindGroup(atlas->device, &bind_group_desc);

    // Create pipeline layout
    WGPUPipelineLayoutDescriptor pipeline_layout_desc = {
        .bindGroupLayoutCount = 1,
        .bindGroupLayouts = &bind_group_layout,
    };
    WGPUPipelineLayout pipeline_layout = wgpuDeviceCreatePipelineLayout(atlas->device, &pipeline_layout_desc);

    // Create shader module from embedded source
    WGPUShaderModule shader = ug_shader_module_create_from_source(atlas->device, DEFAULT_TEXT_SHADER, "Text Shader");
    if (!shader) {
        fprintf(stderr, "Failed to create text shader\n");
        ug_font_atlas_destroy(atlas);
        wgpuBindGroupLayoutRelease(bind_group_layout);
        wgpuPipelineLayoutRelease(pipeline_layout);
        return NULL;
    }

    // Define vertex layout for TextVertex
    WGPUVertexAttribute vertex_attributes[3] = {
        {.format = WGPUVertexFormat_Float32x2, .offset = 0, .shaderLocation = 0},                    // position
        {.format = WGPUVertexFormat_Float32x2, .offset = 2 * sizeof(float), .shaderLocation = 1},   // uv
        {.format = WGPUVertexFormat_Float32x4, .offset = 4 * sizeof(float), .shaderLocation = 2},   // color
    };

    WGPUVertexBufferLayout vertex_buffer_layout = {
        .arrayStride = sizeof(TextVertex),
        .stepMode = WGPUVertexStepMode_Vertex,
        .attributeCount = 3,
        .attributes = vertex_attributes,
    };

    // Create pipeline with blending
    WGPUBlendState blend_state = {
        .color = {
            .operation = WGPUBlendOperation_Add,
            .srcFactor = WGPUBlendFactor_SrcAlpha,
            .dstFactor = WGPUBlendFactor_OneMinusSrcAlpha,
        },
        .alpha = {
            .operation = WGPUBlendOperation_Add,
            .srcFactor = WGPUBlendFactor_One,
            .dstFactor = WGPUBlendFactor_Zero,
        },
    };

    WGPUColorTargetState color_target = {
        .format = ug_context_get_surface_format(context),
        .blend = &blend_state,
        .writeMask = WGPUColorWriteMask_All,
    };

    WGPUFragmentState fragment_state = {
        .module = shader,
        .entryPoint = {"fs_main", WGPU_STRLEN},
        .targetCount = 1,
        .targets = &color_target,
    };

    WGPURenderPipelineDescriptor pipeline_desc = {
        .layout = pipeline_layout,
        .vertex = {
            .module = shader,
            .entryPoint = {"vs_main", WGPU_STRLEN},
            .bufferCount = 1,
            .buffers = &vertex_buffer_layout,
        },
        .primitive = {
            .topology = WGPUPrimitiveTopology_TriangleList,
        },
        .multisample = {
            .count = 1,
            .mask = ~0u,
            .alphaToCoverageEnabled = false,
        },
        .fragment = &fragment_state,
    };

    atlas->pipeline = wgpuDeviceCreateRenderPipeline(atlas->device, &pipeline_desc);

    // Cleanup temporary resources
    wgpuShaderModuleRelease(shader);
    wgpuBindGroupLayoutRelease(bind_group_layout);
    wgpuPipelineLayoutRelease(pipeline_layout);

    return atlas;
}

void ug_font_atlas_destroy(UGFontAtlas* atlas) {
    if (!atlas) {
        return;
    }

    if (atlas->pipeline) wgpuRenderPipelineRelease(atlas->pipeline);
    if (atlas->bind_group) wgpuBindGroupRelease(atlas->bind_group);
    if (atlas->sampler) wgpuSamplerRelease(atlas->sampler);
    if (atlas->texture_view) wgpuTextureViewRelease(atlas->texture_view);
    if (atlas->texture) wgpuTextureRelease(atlas->texture);
    if (atlas->glyphs) free(atlas->glyphs);
    if (atlas->font_data) free(atlas->font_data);

    free(atlas);
}

WGPURenderPipeline ug_font_atlas_get_pipeline(UGFontAtlas* atlas) {
    return atlas ? atlas->pipeline : NULL;
}

WGPUBindGroup ug_font_atlas_get_bind_group(UGFontAtlas* atlas) {
    return atlas ? atlas->bind_group : NULL;
}

static GlyphInfo* find_glyph(UGFontAtlas* atlas, int codepoint) {
    // Simple linear search (could be optimized with hash table)
    for (int i = 0; i < atlas->glyph_count; i++) {
        if (atlas->glyphs[i].codepoint == codepoint) {
            return &atlas->glyphs[i];
        }
    }
    return NULL;
}

void ug_font_atlas_add_text_px(UGFontAtlas* atlas, void* vertices, size_t* count,
                               const char* text, float x, float y, UGContext* context,
                               float r, float g, float b, float a) {
    if (!atlas || !vertices || !count || !text || !context) {
        return;
    }

    // Get window dimensions for pixel-to-NDC conversion
    int width, height;
    ug_window_get_size(ug_context_get_window(context), &width, &height);

    // Convert pixel coordinates to NDC
    // Origin at top-left, y-down -> NDC with origin at center, y-up
    float ndc_x = (x / width) * 2.0f - 1.0f;
    float ndc_y = 1.0f - (y / height) * 2.0f;

    // Calculate pixel height in NDC space
    float pixel_height = 2.0f / height;

    // Call the NDC-based function
    ug_font_atlas_add_text(atlas, vertices, count, text, ndc_x, ndc_y, pixel_height, r, g, b, a);
}

void ug_font_atlas_add_text(UGFontAtlas* atlas, void* vertices, size_t* count,
                            const char* text, float x, float y, float pixel_height,
                            float r, float g, float b, float a) {
    if (!atlas || !vertices || !count || !text) {
        return;
    }

    TextVertex* verts = (TextVertex*)vertices;
    float cursor_x = x;
    float cursor_y = y;

    float inv_atlas_width = 1.0f / atlas->atlas_width;
    float inv_atlas_height = 1.0f / atlas->atlas_height;

    // Calculate pixel scale (assumes square pixels)
    float pixel_scale = pixel_height;

    for (const char* p = text; *p; p++) {
        int codepoint = (int)(*p);

        GlyphInfo* glyph = find_glyph(atlas, codepoint);
        if (!glyph) {
            continue;  // Skip unknown characters
        }

        // Calculate screen-space quad (convert pixel metrics to NDC)
        // Note: Y is flipped because NDC Y increases upward, but font metrics increase downward
        float x0 = cursor_x + glyph->xoff * pixel_scale;
        float y0 = cursor_y - glyph->yoff * pixel_scale;
        float x1 = x0 + (glyph->x1 - glyph->x0) * pixel_scale;
        float y1 = y0 - (glyph->y1 - glyph->y0) * pixel_scale;

        // Calculate UV coordinates
        float u0 = glyph->x0 * inv_atlas_width;
        float v0 = glyph->y0 * inv_atlas_height;
        float u1 = glyph->x1 * inv_atlas_width;
        float v1 = glyph->y1 * inv_atlas_height;

        // Add two triangles for the quad
        // Triangle 1
        verts[*count] = (TextVertex){{x0, y0}, {u0, v0}, {r, g, b, a}};
        (*count)++;
        verts[*count] = (TextVertex){{x1, y0}, {u1, v0}, {r, g, b, a}};
        (*count)++;
        verts[*count] = (TextVertex){{x0, y1}, {u0, v1}, {r, g, b, a}};
        (*count)++;

        // Triangle 2
        verts[*count] = (TextVertex){{x0, y1}, {u0, v1}, {r, g, b, a}};
        (*count)++;
        verts[*count] = (TextVertex){{x1, y0}, {u1, v0}, {r, g, b, a}};
        (*count)++;
        verts[*count] = (TextVertex){{x1, y1}, {u1, v1}, {r, g, b, a}};
        (*count)++;

        // Advance cursor (convert pixel advance to NDC)
        cursor_x += glyph->xadvance * pixel_scale;
    }
}

size_t ug_font_atlas_get_vertex_size(void) {
    return sizeof(TextVertex);
}

void ug_font_atlas_get_vertex_attributes(UGVertexAttribute* attributes) {
    if (!attributes) {
        return;
    }

    attributes[0].format = WGPUVertexFormat_Float32x2;
    attributes[0].offset = 0;
    attributes[0].shader_location = 0;

    attributes[1].format = WGPUVertexFormat_Float32x2;
    attributes[1].offset = 2 * sizeof(float);
    attributes[1].shader_location = 1;

    attributes[2].format = WGPUVertexFormat_Float32x4;
    attributes[2].offset = 4 * sizeof(float);
    attributes[2].shader_location = 2;
}

