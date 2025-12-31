#include "ungrund.h"
#include <webgpu/webgpu.h>
#include <stdlib.h>
#include <string.h>

// Uniform buffer helper
struct UGUniformBuffer {
    WGPUBuffer buffer;
    WGPUQueue queue;
    size_t size;
};

UGUniformBuffer* ug_uniform_buffer_create(UGContext* context, size_t size) {
    if (!context || size == 0) {
        return NULL;
    }

    UGUniformBuffer* uniform = (UGUniformBuffer*)calloc(1, sizeof(UGUniformBuffer));
    if (!uniform) {
        return NULL;
    }

    // Align size to 16 bytes (WebGPU requirement)
    size_t aligned_size = (size + 15) & ~15;

    WGPUDevice device = ug_context_get_device(context);
    WGPUBufferDescriptor buffer_desc = {
        .size = aligned_size,
        .usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
        .mappedAtCreation = false,
    };

    uniform->buffer = wgpuDeviceCreateBuffer(device, &buffer_desc);
    uniform->queue = ug_context_get_queue(context);
    uniform->size = aligned_size;

    return uniform;
}

void ug_uniform_buffer_update(UGUniformBuffer* uniform, const void* data, size_t size) {
    if (!uniform || !data) {
        return;
    }

    size_t write_size = size < uniform->size ? size : uniform->size;
    wgpuQueueWriteBuffer(uniform->queue, uniform->buffer, 0, data, write_size);
}

WGPUBuffer ug_uniform_buffer_get_handle(UGUniformBuffer* uniform) {
    return uniform ? uniform->buffer : NULL;
}

void ug_uniform_buffer_destroy(UGUniformBuffer* uniform) {
    if (uniform) {
        if (uniform->buffer) {
            wgpuBufferRelease(uniform->buffer);
        }
        free(uniform);
    }
}

// Bind group builder
struct UGBindGroupBuilder {
    WGPUDevice device;
    WGPUBindGroupLayoutEntry* layout_entries;
    WGPUBindGroupEntry* entries;
    size_t entry_count;
    size_t capacity;
};

UGBindGroupBuilder* ug_bind_group_builder_create(UGContext* context) {
    if (!context) {
        return NULL;
    }

    UGBindGroupBuilder* builder = (UGBindGroupBuilder*)calloc(1, sizeof(UGBindGroupBuilder));
    if (!builder) {
        return NULL;
    }

    builder->device = ug_context_get_device(context);
    builder->capacity = 8;
    builder->layout_entries = (WGPUBindGroupLayoutEntry*)calloc(builder->capacity, sizeof(WGPUBindGroupLayoutEntry));
    builder->entries = (WGPUBindGroupEntry*)calloc(builder->capacity, sizeof(WGPUBindGroupEntry));

    return builder;
}

void ug_bind_group_builder_add_uniform(UGBindGroupBuilder* builder, uint32_t binding,
                                        UGUniformBuffer* uniform, WGPUShaderStage visibility) {
    if (!builder || !uniform || builder->entry_count >= builder->capacity) {
        return;
    }

    size_t idx = builder->entry_count++;

    builder->layout_entries[idx] = (WGPUBindGroupLayoutEntry){
        .binding = binding,
        .visibility = visibility,
        .buffer = {
            .type = WGPUBufferBindingType_Uniform,
            .minBindingSize = uniform->size,
        },
    };

    builder->entries[idx] = (WGPUBindGroupEntry){
        .binding = binding,
        .buffer = uniform->buffer,
        .size = uniform->size,
    };
}

void ug_bind_group_builder_add_texture(UGBindGroupBuilder* builder, uint32_t binding,
                                        WGPUTextureView texture_view, WGPUSampler sampler) {
    if (!builder || builder->entry_count >= builder->capacity - 1) {
        return;
    }

    // Add texture
    size_t idx = builder->entry_count++;
    builder->layout_entries[idx] = (WGPUBindGroupLayoutEntry){
        .binding = binding,
        .visibility = WGPUShaderStage_Fragment,
        .texture = {
            .sampleType = WGPUTextureSampleType_Float,
            .viewDimension = WGPUTextureViewDimension_2D,
        },
    };

    builder->entries[idx] = (WGPUBindGroupEntry){
        .binding = binding,
        .textureView = texture_view,
    };

    // Add sampler
    idx = builder->entry_count++;
    builder->layout_entries[idx] = (WGPUBindGroupLayoutEntry){
        .binding = binding + 1,
        .visibility = WGPUShaderStage_Fragment,
        .sampler = {
            .type = WGPUSamplerBindingType_Filtering,
        },
    };

    builder->entries[idx] = (WGPUBindGroupEntry){
        .binding = binding + 1,
        .sampler = sampler,
    };
}

WGPUBindGroupLayout ug_bind_group_builder_create_layout(UGBindGroupBuilder* builder) {
    if (!builder || builder->entry_count == 0) {
        return NULL;
    }

    WGPUBindGroupLayoutDescriptor layout_desc = {
        .entryCount = builder->entry_count,
        .entries = builder->layout_entries,
    };

    return wgpuDeviceCreateBindGroupLayout(builder->device, &layout_desc);
}

WGPUBindGroup ug_bind_group_builder_build(UGBindGroupBuilder* builder, WGPUBindGroupLayout layout) {
    if (!builder || !layout || builder->entry_count == 0) {
        return NULL;
    }

    WGPUBindGroupDescriptor bind_group_desc = {
        .layout = layout,
        .entryCount = builder->entry_count,
        .entries = builder->entries,
    };

    return wgpuDeviceCreateBindGroup(builder->device, &bind_group_desc);
}

void ug_bind_group_builder_destroy(UGBindGroupBuilder* builder) {
    if (builder) {
        free(builder->layout_entries);
        free(builder->entries);
        free(builder);
    }
}

