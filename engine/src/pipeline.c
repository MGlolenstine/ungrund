#include "ungrund.h"
#include <webgpu/webgpu.h>
#include <stdlib.h>
#include <string.h>

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

    // Load shader
    builder->shader_module = ug_shader_module_create_from_file(
        builder->device, shader_path, "Shader");
    
    if (!builder->shader_module) {
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

WGPURenderPipeline ug_pipeline_builder_build(UGPipelineBuilder* builder) {
    if (!builder) {
        return NULL;
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
        .layout = builder->layout,
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

void ug_pipeline_builder_destroy(UGPipelineBuilder* builder) {
    if (builder) {
        if (builder->shader_module) {
            wgpuShaderModuleRelease(builder->shader_module);
        }
        free(builder);
    }
}

