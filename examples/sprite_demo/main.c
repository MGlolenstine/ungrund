#include "ungrund.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_VERTICES 1024

typedef struct {
    WGPURenderPipeline pipeline;
    WGPUBindGroup bind_group;
    UGVertexBuffer* vertex_buffer;
    UGTexture* texture;
    UGSpriteSheet* sprite_sheet;

    // Animation state
    float animation_time;
    int current_frame;
    int frame_count;
    float frame_duration;

    // Multiple animated sprites
    float sprite1_x;
    float sprite2_x;
    float sprite3_rotation;
} RenderData;

// Render callback
void render(UGContext* context, UGRenderFrame* frame, float delta_time, void* userdata) {
    RenderData* data = (RenderData*)userdata;
    (void)context;
    
    // Update animation
    data->animation_time += delta_time;
    if (data->animation_time >= data->frame_duration) {
        data->current_frame = (data->current_frame + 1) % data->frame_count;
        data->animation_time = 0.0f;
    }
    
    // Update sprite positions for variety
    float time = (float)ug_get_time();
    data->sprite1_x = 0.5f * sinf(time * 2.0f);
    data->sprite2_x = -0.5f * cosf(time * 1.5f);
    data->sprite3_rotation = time;
    
    // Build vertex data
    UGVertex2DTextured vertices[MAX_VERTICES];
    size_t vertex_count = 0;
    
    // Sprite 1: Animated sprite moving horizontally
    ug_sprite_sheet_add_sprite(data->sprite_sheet, vertices, &vertex_count,
                              data->current_frame, data->sprite1_x, 0.5f, 0.15f, 0.15f);
    
    // Sprite 2: Same animation, different position
    ug_sprite_sheet_add_sprite(data->sprite_sheet, vertices, &vertex_count,
                              data->current_frame, data->sprite2_x, -0.5f, 0.15f, 0.15f);
    
    // Sprite 3: Different frame (static)
    int static_frame = (data->frame_count / 2) % data->frame_count;
    ug_sprite_sheet_add_sprite(data->sprite_sheet, vertices, &vertex_count,
                              static_frame, 0.0f, 0.0f, 0.2f, 0.2f);
    
    // Sprite 4: Show all frames in a row (for demonstration)
    for (int i = 0; i < data->frame_count && i < 8; i++) {
        float x = -0.8f + (i * 0.25f);
        ug_sprite_sheet_add_sprite(data->sprite_sheet, vertices, &vertex_count,
                                  i, x, -0.8f, 0.08f, 0.08f);
    }
    
    // Update vertex buffer
    ug_vertex_buffer_update(data->vertex_buffer, vertices, vertex_count);
    
    // Render
    UGRenderPass* pass = ug_render_pass_begin(frame, 0.1f, 0.1f, 0.15f, 1.0f);
    ug_render_pass_set_pipeline(pass, data->pipeline);
    ug_render_pass_set_bind_group(pass, 0, data->bind_group);
    ug_render_pass_set_vertex_buffer(pass, data->vertex_buffer);
    ug_render_pass_draw(pass, vertex_count);
    ug_render_pass_end(pass);
}

int main(void) {
    // Create window and context
    UGWindow* window = ug_window_create("Sprite Sheet Animation Demo", 800, 600);
    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        return 1;
    }
    
    UGContext* context = ug_context_create(window);
    if (!context) {
        fprintf(stderr, "Failed to create context\n");
        ug_window_destroy(window);
        return 1;
    }
    
    // Load texture (you'll need to provide a sprite sheet image)
    // For now, we'll try to load a test image
    UGTexture* texture = ug_texture_create_from_file(context, "examples/sprite_demo/spritesheet.png");
    if (!texture) {
        fprintf(stderr, "Failed to load sprite sheet texture\n");
        fprintf(stderr, "Please provide a sprite sheet at: examples/sprite_demo/spritesheet.png\n");
        ug_context_destroy(context);
        ug_window_destroy(window);
        return 1;
    }
    
    // Create sprite sheet (assuming 32x32 pixel sprites)
    UGSpriteSheet* sprite_sheet = ug_sprite_sheet_create(texture, 32, 32);
    if (!sprite_sheet) {
        fprintf(stderr, "Failed to create sprite sheet\n");
        ug_texture_destroy(texture);
        ug_context_destroy(context);
        ug_window_destroy(window);
        return 1;
    }
    
    printf("Sprite sheet loaded successfully!\n");
    printf("Total sprites: %d\n", ug_sprite_sheet_get_sprite_count(sprite_sheet));
    
    // Create vertex buffer
    UGVertexBuffer* vertex_buffer = ug_vertex_buffer_create_2d_textured(context, MAX_VERTICES);

    // Build bind group with texture
    UGBindGroupBuilder* bg_builder = ug_bind_group_builder_create(context);
    ug_bind_group_builder_add_texture(bg_builder, 0, ug_texture_get_view(texture), ug_texture_get_sampler(texture));

    WGPUBindGroupLayout bind_group_layout = ug_bind_group_builder_create_layout(bg_builder);
    WGPUBindGroup bind_group = ug_bind_group_builder_build(bg_builder, bind_group_layout);

    // Create pipeline layout
    WGPUDevice device = ug_context_get_device(context);
    WGPUPipelineLayoutDescriptor pipeline_layout_desc = {
        .bindGroupLayoutCount = 1,
        .bindGroupLayouts = &bind_group_layout,
    };
    WGPUPipelineLayout pipeline_layout = wgpuDeviceCreatePipelineLayout(device, &pipeline_layout_desc);

    // Build pipeline with texture support
    UGPipelineBuilder* pipeline_builder = ug_pipeline_builder_create(context, "examples/sprite_demo/sprite.wgsl");
    if (!pipeline_builder) {
        fprintf(stderr, "Failed to create pipeline builder\n");
        return 1;
    }

    // Set vertex buffer layout and pipeline layout
    ug_pipeline_builder_set_vertex_buffer(pipeline_builder, ug_vertex_buffer_get_layout(vertex_buffer));
    ug_pipeline_builder_set_layout(pipeline_builder, pipeline_layout);

    // Enable alpha blending for transparency
    ug_pipeline_builder_enable_blending(pipeline_builder, true);

    // Build pipeline
    WGPURenderPipeline pipeline = ug_pipeline_builder_build(pipeline_builder);
    if (!pipeline) {
        fprintf(stderr, "Failed to build pipeline\n");
        return 1;
    }

    // Initialize render data
    RenderData render_data = {
        .pipeline = pipeline,
        .bind_group = bind_group,
        .vertex_buffer = vertex_buffer,
        .texture = texture,
        .sprite_sheet = sprite_sheet,
        .animation_time = 0.0f,
        .current_frame = 0,
        .frame_count = ug_sprite_sheet_get_sprite_count(sprite_sheet),
        .frame_duration = 0.1f, // 10 FPS animation
        .sprite1_x = 0.0f,
        .sprite2_x = 0.0f,
        .sprite3_rotation = 0.0f,
    };

    // Limit frame count for animation (use first 8 frames if there are many)
    if (render_data.frame_count > 8) {
        render_data.frame_count = 8;
    }

    printf("Animating %d frames at %.1f FPS\n", render_data.frame_count, 1.0f / render_data.frame_duration);

    // Run the application
    ug_run(context, render, &render_data);

    // Cleanup
    ug_sprite_sheet_destroy(sprite_sheet);
    ug_texture_destroy(texture);
    wgpuRenderPipelineRelease(pipeline);
    wgpuPipelineLayoutRelease(pipeline_layout);
    wgpuBindGroupRelease(bind_group);
    wgpuBindGroupLayoutRelease(bind_group_layout);
    ug_bind_group_builder_destroy(bg_builder);
    ug_pipeline_builder_destroy(pipeline_builder);
    ug_vertex_buffer_destroy(vertex_buffer);
    ug_context_destroy(context);
    ug_window_destroy(window);

    return 0;
}

