#include "ungrund.h"
#include <webgpu/webgpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

WGPUShaderModule ug_shader_module_create_from_source(WGPUDevice device, const char* source, const char* label) {
    if (!device || !source) {
        return NULL;
    }

    WGPUShaderSourceWGSL wgsl_source = {
        .chain = {
            .next = NULL,
            .sType = WGPUSType_ShaderSourceWGSL,
        },
        .code = {source, WGPU_STRLEN},
    };

    WGPUShaderModuleDescriptor shader_desc = {
        .nextInChain = (const WGPUChainedStruct*)&wgsl_source,
        .label = {label ? label : "Shader Module", WGPU_STRLEN},
    };

    return wgpuDeviceCreateShaderModule(device, &shader_desc);
}

WGPUShaderModule ug_shader_module_create_from_file(WGPUDevice device, const char* filepath, const char* label) {
    if (!device || !filepath) {
        return NULL;
    }

    char* source = ug_read_file(filepath);
    if (!source) {
        fprintf(stderr, "Failed to load shader file: %s\n", filepath);
        return NULL;
    }

    WGPUShaderModule module = ug_shader_module_create_from_source(device, source, label);
    free(source);

    return module;
}

