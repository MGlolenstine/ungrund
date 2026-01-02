#include "ungrund.h"
#include <webgpu/webgpu.h>
#include <stdio.h>

// Simplified render data using UGPipeline wrapper
typedef struct {
    UGPipeline* pipeline;
    UGUniformBuffer* uniform;
} RenderData;

// Render callback - called each frame
void render(UGContext* context, UGRenderFrame* frame, float delta_time, void* userdata) {
    RenderData* data = (RenderData*)userdata;
    (void)context;
    (void)delta_time;

    // Update rotation based on time
    float rotation = (float)ug_get_time();
    ug_uniform_buffer_update(data->uniform, &rotation, sizeof(float));

    // Use UGRenderPass for simplified rendering
    UGRenderPass* pass = ug_render_pass_begin(frame, 0.1f, 0.1f, 0.1f, 1.0f);
    ug_render_pass_set_pipeline(pass, ug_pipeline_get_handle(data->pipeline));
    ug_render_pass_set_bind_group(pass, 0, ug_pipeline_get_bind_group(data->pipeline, 0));
    ug_render_pass_draw(pass, 3);
    ug_render_pass_end(pass);
}

int main(void) {
    // Create window and context
    UGWindow* window = ug_window_create("Rotating Triangle - Simplified API", 800, 600);
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

    // Create uniform buffer for rotation
    UGUniformBuffer* uniform = ug_uniform_buffer_create(context, sizeof(float));

    // Build pipeline with integrated bind group support
    UGPipelineBuilder* pipeline_builder = ug_pipeline_builder_create(context, "examples/triangle/triangle.wgsl");
    if (!pipeline_builder) {
        fprintf(stderr, "Failed to create pipeline builder\n");
        return 1;
    }

    // Add uniform directly to pipeline builder
    ug_pipeline_builder_add_uniform(pipeline_builder, 0, uniform, WGPUShaderStage_Vertex);
    
    // Build the pipeline
    WGPURenderPipeline render_pipeline = ug_pipeline_builder_build(pipeline_builder);
    
    // Create bind group
    WGPUDevice device = ug_context_get_device(context);
    WGPUBindGroupLayoutDescriptor bg_layout_desc = {
        .entryCount = 1,
        .entries = &(WGPUBindGroupLayoutEntry){
            .binding = 0,
            .visibility = WGPUShaderStage_Vertex,
            .buffer = {
                .type = WGPUBufferBindingType_Uniform,
                .minBindingSize = 0,
            },
        },
    };
    WGPUBindGroupLayout bind_group_layout = wgpuDeviceCreateBindGroupLayout(device, &bg_layout_desc);
    WGPUBindGroup bind_group = ug_pipeline_builder_build_bind_group(pipeline_builder, bind_group_layout);

    // Create UGPipeline wrapper to manage all resources
    UGPipeline* pipeline = ug_pipeline_create(context);
    ug_pipeline_set_render_pipeline(pipeline, render_pipeline);
    ug_pipeline_add_bind_group(pipeline, bind_group, bind_group_layout);
    ug_pipeline_add_uniform(pipeline, uniform);

    printf("Rotating triangle example (Simplified API). Press ESC to exit.\n");
    printf("This example demonstrates the new UGPipeline wrapper for automatic resource management.\n");

    // Setup render data
    RenderData render_data = {
        .pipeline = pipeline,
        .uniform = uniform,
    };

    // Run
    ug_run(context, render, &render_data);

    // Cleanup - MUCH simpler! Just destroy the pipeline wrapper and it cleans up everything
    ug_pipeline_destroy(pipeline);  // Destroys pipeline, layouts, bind groups, and uniforms
    ug_pipeline_builder_destroy(pipeline_builder);
    ug_context_destroy(context);
    ug_window_destroy(window);

    return 0;
}

