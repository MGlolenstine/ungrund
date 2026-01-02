#include "ungrund.h"
#include <webgpu/webgpu.h>
#include <stdlib.h>
#include <string.h>

// UGPipeline - owns all pipeline-related resources
struct UGPipeline {
    WGPUDevice device;
    WGPURenderPipeline pipeline;
    WGPUPipelineLayout pipeline_layout;
    WGPUBindGroupLayout* bind_group_layouts;
    WGPUBindGroup* bind_groups;
    UGUniformBuffer** uniforms;
    size_t bind_group_count;
    size_t uniform_count;
};

// Internal helper to create pipeline layout from bind group layouts
static WGPUPipelineLayout create_pipeline_layout(WGPUDevice device, 
                                                   WGPUBindGroupLayout* layouts, 
                                                   size_t layout_count) {
    if (layout_count == 0) {
        // Create empty pipeline layout
        WGPUPipelineLayoutDescriptor desc = {
            .bindGroupLayoutCount = 0,
            .bindGroupLayouts = NULL,
        };
        return wgpuDeviceCreatePipelineLayout(device, &desc);
    }
    
    WGPUPipelineLayoutDescriptor desc = {
        .bindGroupLayoutCount = layout_count,
        .bindGroupLayouts = layouts,
    };
    return wgpuDeviceCreatePipelineLayout(device, &desc);
}

UGPipeline* ug_pipeline_create(UGContext* context) {
    if (!context) {
        return NULL;
    }
    
    UGPipeline* pipeline = (UGPipeline*)calloc(1, sizeof(UGPipeline));
    if (!pipeline) {
        return NULL;
    }
    
    pipeline->device = ug_context_get_device(context);
    pipeline->bind_group_layouts = NULL;
    pipeline->bind_groups = NULL;
    pipeline->uniforms = NULL;
    pipeline->bind_group_count = 0;
    pipeline->uniform_count = 0;
    pipeline->pipeline = NULL;
    pipeline->pipeline_layout = NULL;
    
    return pipeline;
}

void ug_pipeline_set_render_pipeline(UGPipeline* pipeline, WGPURenderPipeline render_pipeline) {
    if (pipeline) {
        pipeline->pipeline = render_pipeline;
    }
}

void ug_pipeline_set_pipeline_layout(UGPipeline* pipeline, WGPUPipelineLayout layout) {
    if (pipeline) {
        pipeline->pipeline_layout = layout;
    }
}

void ug_pipeline_add_bind_group(UGPipeline* pipeline, WGPUBindGroup bind_group, 
                                 WGPUBindGroupLayout layout) {
    if (!pipeline) {
        return;
    }
    
    size_t new_count = pipeline->bind_group_count + 1;
    
    WGPUBindGroup* new_bind_groups = (WGPUBindGroup*)realloc(
        pipeline->bind_groups, new_count * sizeof(WGPUBindGroup));
    WGPUBindGroupLayout* new_layouts = (WGPUBindGroupLayout*)realloc(
        pipeline->bind_group_layouts, new_count * sizeof(WGPUBindGroupLayout));
    
    if (!new_bind_groups || !new_layouts) {
        return;
    }
    
    pipeline->bind_groups = new_bind_groups;
    pipeline->bind_group_layouts = new_layouts;
    pipeline->bind_groups[pipeline->bind_group_count] = bind_group;
    pipeline->bind_group_layouts[pipeline->bind_group_count] = layout;
    pipeline->bind_group_count = new_count;
}

void ug_pipeline_add_uniform(UGPipeline* pipeline, UGUniformBuffer* uniform) {
    if (!pipeline || !uniform) {
        return;
    }
    
    size_t new_count = pipeline->uniform_count + 1;
    UGUniformBuffer** new_uniforms = (UGUniformBuffer**)realloc(
        pipeline->uniforms, new_count * sizeof(UGUniformBuffer*));
    
    if (!new_uniforms) {
        return;
    }
    
    pipeline->uniforms = new_uniforms;
    pipeline->uniforms[pipeline->uniform_count] = uniform;
    pipeline->uniform_count = new_count;
}

WGPURenderPipeline ug_pipeline_get_handle(UGPipeline* pipeline) {
    return pipeline ? pipeline->pipeline : NULL;
}

WGPUBindGroup ug_pipeline_get_bind_group(UGPipeline* pipeline, size_t index) {
    if (!pipeline || index >= pipeline->bind_group_count) {
        return NULL;
    }
    return pipeline->bind_groups[index];
}

void ug_pipeline_destroy(UGPipeline* pipeline) {
    if (!pipeline) {
        return;
    }
    
    // Release all uniforms
    for (size_t i = 0; i < pipeline->uniform_count; i++) {
        if (pipeline->uniforms[i]) {
            ug_uniform_buffer_destroy(pipeline->uniforms[i]);
        }
    }
    free(pipeline->uniforms);
    
    // Release all bind groups
    for (size_t i = 0; i < pipeline->bind_group_count; i++) {
        if (pipeline->bind_groups[i]) {
            wgpuBindGroupRelease(pipeline->bind_groups[i]);
        }
    }
    free(pipeline->bind_groups);
    
    // Release all bind group layouts
    for (size_t i = 0; i < pipeline->bind_group_count; i++) {
        if (pipeline->bind_group_layouts[i]) {
            wgpuBindGroupLayoutRelease(pipeline->bind_group_layouts[i]);
        }
    }
    free(pipeline->bind_group_layouts);
    
    // Release pipeline layout
    if (pipeline->pipeline_layout) {
        wgpuPipelineLayoutRelease(pipeline->pipeline_layout);
    }
    
    // Release render pipeline
    if (pipeline->pipeline) {
        wgpuRenderPipelineRelease(pipeline->pipeline);
    }
    
    free(pipeline);
}

