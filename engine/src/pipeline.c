#include "ungrund.h"
#include <webgpu/webgpu.h>
#include <stdlib.h>
#include <string.h>

// Bind group entry for pipeline builder
typedef struct {
    enum { UG_BIND_UNIFORM, UG_BIND_TEXTURE } type;
    uint32_t binding;
    union {
        struct {
            UGUniformBuffer* uniform;
            WGPUShaderStage visibility;
        } uniform_data;
        struct {
            WGPUTextureView texture_view;
            WGPUSampler sampler;
        } texture_data;
    };
} UGBindEntry;

// Pipeline builder implementation
struct UGPipelineBuilder {
    WGPUDevice device;
    WGPUShaderModule shader_module;
    WGPUTextureFormat surface_format;
    WGPUPipelineLayout layout;
    WGPUVertexBufferLayout* vertex_buffers;
    size_t vertex_buffer_count;
    bool enable_blending;
    WGPUPrimitiveTopology topology;

    // Integrated bind group support
    UGBindEntry* bind_entries;
    size_t bind_entry_count;
    size_t bind_entry_capacity;
    bool auto_create_layout;
};

UGPipelineBuilder* ug_pipeline_builder_create(UGContext* context, const char* shader_path) {
    if (!context || !shader_path) {
        return NULL;
    }

    UGPipelineBuilder* builder = (UGPipelineBuilder*)calloc(1, sizeof(UGPipelineBuilder));
    if (!builder) {
        return NULL;
    }

    builder->device = ug_context_get_device(context);
    builder->surface_format = ug_context_get_surface_format(context);
    builder->topology = WGPUPrimitiveTopology_TriangleList;
    builder->enable_blending = false;
    builder->auto_create_layout = true;

    // Initialize bind entry storage
    builder->bind_entry_capacity = 8;
    builder->bind_entries = (UGBindEntry*)calloc(builder->bind_entry_capacity, sizeof(UGBindEntry));
    builder->bind_entry_count = 0;

    // Load shader
    builder->shader_module = ug_shader_module_create_from_file(
        builder->device, shader_path, "Shader");

    if (!builder->shader_module) {
        free(builder->bind_entries);
        free(builder);
        return NULL;
    }

    return builder;
}

void ug_pipeline_builder_set_layout(UGPipelineBuilder* builder, WGPUPipelineLayout layout) {
    if (builder) {
        builder->layout = layout;
    }
}

void ug_pipeline_builder_set_vertex_buffer(UGPipelineBuilder* builder, WGPUVertexBufferLayout* layout) {
    if (builder && layout) {
        builder->vertex_buffers = layout;
        builder->vertex_buffer_count = 1;
    }
}

void ug_pipeline_builder_enable_blending(UGPipelineBuilder* builder, bool enable) {
    if (builder) {
        builder->enable_blending = enable;
    }
}

void ug_pipeline_builder_set_topology(UGPipelineBuilder* builder, WGPUPrimitiveTopology topology) {
    if (builder) {
        builder->topology = topology;
    }
}

void ug_pipeline_builder_add_uniform(UGPipelineBuilder* builder, uint32_t binding,
                                      UGUniformBuffer* uniform, WGPUShaderStage visibility) {
    if (!builder || !uniform || builder->bind_entry_count >= builder->bind_entry_capacity) {
        return;
    }

    UGBindEntry* entry = &builder->bind_entries[builder->bind_entry_count++];
    entry->type = UG_BIND_UNIFORM;
    entry->binding = binding;
    entry->uniform_data.uniform = uniform;
    entry->uniform_data.visibility = visibility;
}

void ug_pipeline_builder_add_texture(UGPipelineBuilder* builder, uint32_t binding,
                                      WGPUTextureView texture_view, WGPUSampler sampler) {
    if (!builder || builder->bind_entry_count >= builder->bind_entry_capacity - 1) {
        return;
    }

    UGBindEntry* entry = &builder->bind_entries[builder->bind_entry_count++];
    entry->type = UG_BIND_TEXTURE;
    entry->binding = binding;
    entry->texture_data.texture_view = texture_view;
    entry->texture_data.sampler = sampler;
}

WGPURenderPipeline ug_pipeline_builder_build(UGPipelineBuilder* builder) {
    if (!builder) {
        return NULL;
    }

    // Auto-create pipeline layout from bind entries if needed
    WGPUPipelineLayout layout_to_use = builder->layout;
    if (builder->auto_create_layout && builder->bind_entry_count > 0 && !builder->layout) {
        // Create bind group layout from entries
        WGPUBindGroupLayoutEntry* layout_entries =
            (WGPUBindGroupLayoutEntry*)calloc(builder->bind_entry_count * 2, sizeof(WGPUBindGroupLayoutEntry));
        size_t layout_entry_count = 0;

        for (size_t i = 0; i < builder->bind_entry_count; i++) {
            UGBindEntry* entry = &builder->bind_entries[i];
            if (entry->type == UG_BIND_UNIFORM) {
                layout_entries[layout_entry_count].binding = entry->binding;
                layout_entries[layout_entry_count].visibility = entry->uniform_data.visibility;
                layout_entries[layout_entry_count].buffer.type = WGPUBufferBindingType_Uniform;
                layout_entries[layout_entry_count].buffer.minBindingSize = 0;
                layout_entry_count++;
            } else if (entry->type == UG_BIND_TEXTURE) {
                // Texture binding
                layout_entries[layout_entry_count].binding = entry->binding;
                layout_entries[layout_entry_count].visibility = WGPUShaderStage_Fragment;
                layout_entries[layout_entry_count].texture.sampleType = WGPUTextureSampleType_Float;
                layout_entries[layout_entry_count].texture.viewDimension = WGPUTextureViewDimension_2D;
                layout_entry_count++;

                // Sampler binding
                layout_entries[layout_entry_count].binding = entry->binding + 1;
                layout_entries[layout_entry_count].visibility = WGPUShaderStage_Fragment;
                layout_entries[layout_entry_count].sampler.type = WGPUSamplerBindingType_Filtering;
                layout_entry_count++;
            }
        }

        WGPUBindGroupLayoutDescriptor bg_layout_desc = {
            .entryCount = layout_entry_count,
            .entries = layout_entries,
        };
        WGPUBindGroupLayout bind_group_layout = wgpuDeviceCreateBindGroupLayout(builder->device, &bg_layout_desc);

        WGPUPipelineLayoutDescriptor pipeline_layout_desc = {
            .bindGroupLayoutCount = 1,
            .bindGroupLayouts = &bind_group_layout,
        };
        layout_to_use = wgpuDeviceCreatePipelineLayout(builder->device, &pipeline_layout_desc);

        // Store for cleanup
        builder->layout = layout_to_use;

        wgpuBindGroupLayoutRelease(bind_group_layout);
        free(layout_entries);
    } else if (!layout_to_use) {
        // Create empty pipeline layout
        WGPUPipelineLayoutDescriptor pipeline_layout_desc = {
            .bindGroupLayoutCount = 0,
            .bindGroupLayouts = NULL,
        };
        layout_to_use = wgpuDeviceCreatePipelineLayout(builder->device, &pipeline_layout_desc);
        builder->layout = layout_to_use;
    }

    WGPUColorTargetState color_target = {
        .format = builder->surface_format,
        .writeMask = WGPUColorWriteMask_All,
    };

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

    if (builder->enable_blending) {
        color_target.blend = &blend_state;
    }

    WGPUFragmentState fragment_state = {
        .module = builder->shader_module,
        .entryPoint = {"fs_main", WGPU_STRLEN},
        .targetCount = 1,
        .targets = &color_target,
    };

    WGPURenderPipelineDescriptor pipeline_desc = {
        .layout = layout_to_use,
        .vertex = {
            .module = builder->shader_module,
            .entryPoint = {"vs_main", WGPU_STRLEN},
            .bufferCount = builder->vertex_buffer_count,
            .buffers = builder->vertex_buffers,
        },
        .primitive = {
            .topology = builder->topology,
        },
        .multisample = {
            .count = 1,
            .mask = ~0u,
            .alphaToCoverageEnabled = false,
        },
        .fragment = &fragment_state,
    };

    return wgpuDeviceCreateRenderPipeline(builder->device, &pipeline_desc);
}

WGPUBindGroup ug_pipeline_builder_build_bind_group(UGPipelineBuilder* builder, WGPUBindGroupLayout layout) {
    if (!builder || !layout || builder->bind_entry_count == 0) {
        return NULL;
    }

    WGPUBindGroupEntry* entries = (WGPUBindGroupEntry*)calloc(builder->bind_entry_count * 2, sizeof(WGPUBindGroupEntry));
    size_t entry_count = 0;

    for (size_t i = 0; i < builder->bind_entry_count; i++) {
        UGBindEntry* bind_entry = &builder->bind_entries[i];
        if (bind_entry->type == UG_BIND_UNIFORM) {
            entries[entry_count].binding = bind_entry->binding;
            entries[entry_count].buffer = ug_uniform_buffer_get_handle(bind_entry->uniform_data.uniform);
            entries[entry_count].size = WGPU_WHOLE_SIZE;
            entry_count++;
        } else if (bind_entry->type == UG_BIND_TEXTURE) {
            // Texture
            entries[entry_count].binding = bind_entry->binding;
            entries[entry_count].textureView = bind_entry->texture_data.texture_view;
            entry_count++;

            // Sampler
            entries[entry_count].binding = bind_entry->binding + 1;
            entries[entry_count].sampler = bind_entry->texture_data.sampler;
            entry_count++;
        }
    }

    WGPUBindGroupDescriptor bind_group_desc = {
        .layout = layout,
        .entryCount = entry_count,
        .entries = entries,
    };

    WGPUBindGroup bind_group = wgpuDeviceCreateBindGroup(builder->device, &bind_group_desc);
    free(entries);

    return bind_group;
}

void ug_pipeline_builder_destroy(UGPipelineBuilder* builder) {
    if (builder) {
        if (builder->shader_module) {
            wgpuShaderModuleRelease(builder->shader_module);
        }
        if (builder->layout && builder->auto_create_layout) {
            wgpuPipelineLayoutRelease(builder->layout);
        }
        free(builder->bind_entries);
        free(builder);
    }
}

