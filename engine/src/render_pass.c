#include "ungrund.h"
#include <webgpu/webgpu.h>
#include <stdlib.h>

// Simplified render pass for common rendering scenarios
struct UGRenderPass {
    WGPURenderPassEncoder encoder;
    WGPURenderPipeline pipeline;
    UGVertexBuffer* vertex_buffer;
};

UGRenderPass* ug_render_pass_begin(UGRenderFrame* frame, float r, float g, float b, float a) {
    if (!frame) {
        return NULL;
    }

    UGRenderPass* pass = (UGRenderPass*)calloc(1, sizeof(UGRenderPass));
    if (!pass) {
        return NULL;
    }

    WGPUTextureView view = ug_render_frame_get_view(frame);
    WGPUCommandEncoder encoder = ug_render_frame_get_encoder(frame);

    // Setup render pass with clear color
    WGPURenderPassColorAttachment color_attachment = {
        .view = view,
        .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
        .loadOp = WGPULoadOp_Clear,
        .storeOp = WGPUStoreOp_Store,
        .clearValue = {r, g, b, a},
    };

    WGPURenderPassDescriptor render_pass_desc = {
        .colorAttachmentCount = 1,
        .colorAttachments = &color_attachment,
    };

    pass->encoder = wgpuCommandEncoderBeginRenderPass(encoder, &render_pass_desc);
    pass->pipeline = NULL;
    pass->vertex_buffer = NULL;

    return pass;
}

void ug_render_pass_set_pipeline(UGRenderPass* pass, WGPURenderPipeline pipeline) {
    if (!pass || !pipeline) {
        return;
    }

    pass->pipeline = pipeline;
    wgpuRenderPassEncoderSetPipeline(pass->encoder, pipeline);
}

void ug_render_pass_set_vertex_buffer(UGRenderPass* pass, UGVertexBuffer* vertex_buffer) {
    if (!pass || !vertex_buffer) {
        return;
    }

    pass->vertex_buffer = vertex_buffer;
    WGPUBuffer buffer = ug_vertex_buffer_get_handle(vertex_buffer);
    wgpuRenderPassEncoderSetVertexBuffer(pass->encoder, 0, buffer, 0, WGPU_WHOLE_SIZE);
}

void ug_render_pass_set_bind_group(UGRenderPass* pass, uint32_t group_index, WGPUBindGroup bind_group) {
    if (!pass || !bind_group) {
        return;
    }

    wgpuRenderPassEncoderSetBindGroup(pass->encoder, group_index, bind_group, 0, NULL);
}

void ug_render_pass_draw(UGRenderPass* pass, uint32_t vertex_count) {
    if (!pass) {
        return;
    }

    wgpuRenderPassEncoderDraw(pass->encoder, vertex_count, 1, 0, 0);
}

void ug_render_pass_draw_indexed(UGRenderPass* pass, uint32_t index_count) {
    if (!pass) {
        return;
    }

    wgpuRenderPassEncoderDrawIndexed(pass->encoder, index_count, 1, 0, 0, 0);
}

void ug_render_pass_end(UGRenderPass* pass) {
    if (!pass) {
        return;
    }

    if (pass->encoder) {
        wgpuRenderPassEncoderEnd(pass->encoder);
        wgpuRenderPassEncoderRelease(pass->encoder);
    }
    
    free(pass);
}

