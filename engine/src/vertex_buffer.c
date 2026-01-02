#include "ungrund.h"
#include <webgpu/webgpu.h>
#include <stdlib.h>
#include <string.h>

// Dynamic vertex buffer for efficient per-frame updates
struct UGVertexBuffer {
    WGPUBuffer buffer;
    WGPUQueue queue;
    size_t capacity;        // Maximum number of vertices
    size_t vertex_size;     // Size of each vertex in bytes
    WGPUVertexBufferLayout layout;
    WGPUVertexAttribute* attributes;
    size_t attribute_count;
};

UGVertexBuffer* ug_vertex_buffer_create(UGContext* context, size_t vertex_size, size_t max_vertices) {
    if (!context || vertex_size == 0 || max_vertices == 0) {
        return NULL;
    }

    UGVertexBuffer* vb = (UGVertexBuffer*)calloc(1, sizeof(UGVertexBuffer));
    if (!vb) {
        return NULL;
    }

    WGPUDevice device = ug_context_get_device(context);
    
    // Create buffer with Vertex and CopyDst usage
    WGPUBufferDescriptor buffer_desc = {
        .size = vertex_size * max_vertices,
        .usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst,
        .mappedAtCreation = false,
    };
    
    vb->buffer = wgpuDeviceCreateBuffer(device, &buffer_desc);
    vb->queue = ug_context_get_queue(context);
    vb->capacity = max_vertices;
    vb->vertex_size = vertex_size;
    vb->attributes = NULL;
    vb->attribute_count = 0;

    return vb;
}

void ug_vertex_buffer_set_layout(UGVertexBuffer* vb, const UGVertexAttribute* attributes, size_t attribute_count) {
    if (!vb || !attributes || attribute_count == 0) {
        return;
    }

    // Free old attributes if any
    if (vb->attributes) {
        free(vb->attributes);
    }

    // Allocate and copy attributes
    vb->attribute_count = attribute_count;
    vb->attributes = (WGPUVertexAttribute*)calloc(attribute_count, sizeof(WGPUVertexAttribute));
    
    for (size_t i = 0; i < attribute_count; i++) {
        vb->attributes[i].format = attributes[i].format;
        vb->attributes[i].offset = attributes[i].offset;
        vb->attributes[i].shaderLocation = attributes[i].shader_location;
    }

    // Setup layout
    vb->layout.arrayStride = vb->vertex_size;
    vb->layout.stepMode = WGPUVertexStepMode_Vertex;
    vb->layout.attributeCount = attribute_count;
    vb->layout.attributes = vb->attributes;
}

void ug_vertex_buffer_update(UGVertexBuffer* vb, const void* data, size_t vertex_count) {
    if (!vb || !data || vertex_count == 0) {
        return;
    }

    size_t data_size = vertex_count * vb->vertex_size;
    size_t max_size = vb->capacity * vb->vertex_size;
    
    if (data_size > max_size) {
        data_size = max_size;
    }

    wgpuQueueWriteBuffer(vb->queue, vb->buffer, 0, data, data_size);
}

WGPUBuffer ug_vertex_buffer_get_handle(UGVertexBuffer* vb) {
    return vb ? vb->buffer : NULL;
}

WGPUVertexBufferLayout* ug_vertex_buffer_get_layout(UGVertexBuffer* vb) {
    return vb ? &vb->layout : NULL;
}

void ug_vertex_buffer_destroy(UGVertexBuffer* vb) {
    if (vb) {
        if (vb->buffer) {
            wgpuBufferRelease(vb->buffer);
        }
        if (vb->attributes) {
            free(vb->attributes);
        }
        free(vb);
    }
}

// Convenience function for UGVertex2DColor format
UGVertexBuffer* ug_vertex_buffer_create_2d_color(UGContext* context, size_t max_vertices) {
    if (!context || max_vertices == 0) {
        return NULL;
    }

    // UGVertex2DColor: position (vec2) + color (vec3)
    size_t vertex_size = sizeof(float) * 5;  // 2 + 3

    UGVertexBuffer* vb = ug_vertex_buffer_create(context, vertex_size, max_vertices);
    if (!vb) {
        return NULL;
    }

    // Set up standard layout for UGVertex2DColor
    UGVertexAttribute attributes[2] = {
        {
            .format = WGPUVertexFormat_Float32x2,
            .offset = 0,
            .shader_location = 0,
        },
        {
            .format = WGPUVertexFormat_Float32x3,
            .offset = 2 * sizeof(float),
            .shader_location = 1,
        },
    };

    ug_vertex_buffer_set_layout(vb, attributes, 2);

    return vb;
}

// Convenience function for UGVertex2DTextured format
UGVertexBuffer* ug_vertex_buffer_create_2d_textured(UGContext* context, size_t max_vertices) {
    if (!context || max_vertices == 0) {
        return NULL;
    }

    // UGVertex2DTextured: position (vec2) + uv (vec2)
    size_t vertex_size = sizeof(float) * 4;  // 2 + 2

    UGVertexBuffer* vb = ug_vertex_buffer_create(context, vertex_size, max_vertices);
    if (!vb) {
        return NULL;
    }

    // Set up standard layout for UGVertex2DTextured
    UGVertexAttribute attributes[2] = {
        {
            .format = WGPUVertexFormat_Float32x2,
            .offset = 0,
            .shader_location = 0,
        },
        {
            .format = WGPUVertexFormat_Float32x2,
            .offset = 2 * sizeof(float),
            .shader_location = 1,
        },
    };

    ug_vertex_buffer_set_layout(vb, attributes, 2);

    return vb;
}


