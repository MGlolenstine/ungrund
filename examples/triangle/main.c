#include "ungrund.h"
#include <webgpu/webgpu.h>
#include <stdio.h>

// Simple render data - just what we need
typedef struct {
    WGPURenderPipeline pipeline;
    WGPUBindGroup bind_group;
    UGUniformBuffer* uniform;
} RenderData;

// Render callback - called each frame
void render(UGContext* context, UGRenderFrame* frame, float delta_time, void* userdata) {
    RenderData* data = (RenderData*)userdata;
    (void)context;
    (void)delta_time;  // Not used in this example

    // Update rotation based on time
    float rotation = (float)ug_get_time();
    ug_uniform_buffer_update(data->uniform, &rotation, sizeof(float));

    // Get view and encoder from frame
    WGPUTextureView view = ug_render_frame_get_view(frame);
    WGPUCommandEncoder encoder = ug_render_frame_get_encoder(frame);

    // Setup render pass
    WGPURenderPassColorAttachment color_attachment = {
        .view = view,
        .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
        .loadOp = WGPULoadOp_Clear,
        .storeOp = WGPUStoreOp_Store,
        .clearValue = {0.1, 0.1, 0.1, 1.0},
    };

    WGPURenderPassDescriptor render_pass_desc = {
        .colorAttachmentCount = 1,
        .colorAttachments = &color_attachment,
    };

    // Render
    WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &render_pass_desc);
    wgpuRenderPassEncoderSetPipeline(pass, data->pipeline);
    wgpuRenderPassEncoderSetBindGroup(pass, 0, data->bind_group, 0, NULL);
    wgpuRenderPassEncoderDraw(pass, 3, 1, 0, 0);
    wgpuRenderPassEncoderEnd(pass);
    wgpuRenderPassEncoderRelease(pass);
}

int main(void) {
    // Create window and context
    UGWindow* window = ug_window_create("Rotating Triangle", 800, 600);
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

    // Build bind group with uniform
    UGBindGroupBuilder* bg_builder = ug_bind_group_builder_create(context);
    ug_bind_group_builder_add_uniform(bg_builder, 0, uniform, WGPUShaderStage_Vertex);

    WGPUBindGroupLayout bind_group_layout = ug_bind_group_builder_create_layout(bg_builder);
    WGPUBindGroup bind_group = ug_bind_group_builder_build(bg_builder, bind_group_layout);

    // Create pipeline layout
    WGPUDevice device = ug_context_get_device(context);
    WGPUPipelineLayoutDescriptor layout_desc = {
        .bindGroupLayoutCount = 1,
        .bindGroupLayouts = &bind_group_layout,
    };
    WGPUPipelineLayout pipeline_layout = wgpuDeviceCreatePipelineLayout(device, &layout_desc);

    // Build pipeline
    UGPipelineBuilder* pipeline_builder = ug_pipeline_builder_create(context, "examples/triangle/triangle.wgsl");
    if (!pipeline_builder) {
        fprintf(stderr, "Failed to create pipeline builder\n");
        return 1;
    }

    ug_pipeline_builder_set_layout(pipeline_builder, pipeline_layout);
    WGPURenderPipeline pipeline = ug_pipeline_builder_build(pipeline_builder);

    printf("Rotating triangle example. Press ESC to exit.\n");

    // Setup render data
    RenderData render_data = {
        .pipeline = pipeline,
        .bind_group = bind_group,
        .uniform = uniform,
    };

    // Run
    ug_run(context, render, &render_data);

    // Cleanup
    ug_uniform_buffer_destroy(uniform);
    ug_bind_group_builder_destroy(bg_builder);
    wgpuBindGroupRelease(bind_group);
    wgpuBindGroupLayoutRelease(bind_group_layout);
    wgpuPipelineLayoutRelease(pipeline_layout);
    wgpuRenderPipelineRelease(pipeline);
    ug_pipeline_builder_destroy(pipeline_builder);
    ug_context_destroy(context);
    ug_window_destroy(window);

    return 0;
}

