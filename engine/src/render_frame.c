#include "ungrund.h"
#include <webgpu/webgpu.h>
#include <stdio.h>
#include <stdlib.h>

struct UGRenderFrame {
    UGContext* context;
    WGPUSurfaceTexture surface_texture;
    WGPUTextureView view;
    WGPUCommandEncoder encoder;
};

UGRenderFrame* ug_begin_render_frame(UGContext* context) {
    if (!context) {
        return NULL;
    }

    UGRenderFrame* frame = (UGRenderFrame*)calloc(1, sizeof(UGRenderFrame));
    if (!frame) {
        return NULL;
    }

    frame->context = context;

    // Get current surface texture
    WGPUSurface surface = ug_context_get_surface(context);
    wgpuSurfaceGetCurrentTexture(surface, &frame->surface_texture);

    if (frame->surface_texture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
        frame->surface_texture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal) {
        fprintf(stderr, "Failed to get surface texture: %d\n", frame->surface_texture.status);
        free(frame);
        return NULL;
    }

    // Create texture view
    frame->view = wgpuTextureCreateView(frame->surface_texture.texture, NULL);
    if (!frame->view) {
        fprintf(stderr, "Failed to create texture view\n");
        free(frame);
        return NULL;
    }

    // Create command encoder
    WGPUDevice device = ug_context_get_device(context);
    frame->encoder = wgpuDeviceCreateCommandEncoder(device, NULL);
    if (!frame->encoder) {
        fprintf(stderr, "Failed to create command encoder\n");
        wgpuTextureViewRelease(frame->view);
        free(frame);
        return NULL;
    }

    return frame;
}

WGPUTextureView ug_render_frame_get_view(UGRenderFrame* frame) {
    return frame ? frame->view : NULL;
}

WGPUCommandEncoder ug_render_frame_get_encoder(UGRenderFrame* frame) {
    return frame ? frame->encoder : NULL;
}

void ug_end_render_frame(UGRenderFrame* frame) {
    if (!frame) {
        return;
    }

    // Finish command encoder
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(frame->encoder, NULL);

    // Submit to queue
    WGPUQueue queue = ug_context_get_queue(frame->context);
    wgpuQueueSubmit(queue, 1, &command);

    // Present surface
    WGPUSurface surface = ug_context_get_surface(frame->context);
    wgpuSurfacePresent(surface);

    // Cleanup
    wgpuCommandBufferRelease(command);
    wgpuCommandEncoderRelease(frame->encoder);
    wgpuTextureViewRelease(frame->view);
    free(frame);
}

