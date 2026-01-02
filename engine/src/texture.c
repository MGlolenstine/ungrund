#include "ungrund.h"
#include <webgpu/webgpu.h>
#include <stdlib.h>
#include <stdio.h>

// stb_image implementation
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct UGTexture {
    WGPUDevice device;
    WGPUTexture texture;
    WGPUTextureView texture_view;
    WGPUSampler sampler;
    int width;
    int height;
    int channels;
};

UGTexture* ug_texture_create_from_file(UGContext* context, const char* filepath) {
    if (!context || !filepath) {
        return NULL;
    }
    
    UGTexture* tex = (UGTexture*)calloc(1, sizeof(UGTexture));
    if (!tex) {
        return NULL;
    }
    
    tex->device = ug_context_get_device(context);
    
    // Load image using stb_image
    int width, height, channels;
    unsigned char* image_data = stbi_load(filepath, &width, &height, &channels, 4); // Force RGBA
    if (!image_data) {
        fprintf(stderr, "Failed to load image: %s\n", filepath);
        free(tex);
        return NULL;
    }
    
    tex->width = width;
    tex->height = height;
    tex->channels = 4; // We forced RGBA
    
    // Create WebGPU texture
    WGPUTextureDescriptor texture_desc = {
        .size = {(uint32_t)width, (uint32_t)height, 1},
        .format = WGPUTextureFormat_RGBA8Unorm,
        .usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst,
        .dimension = WGPUTextureDimension_2D,
        .mipLevelCount = 1,
        .sampleCount = 1,
    };
    tex->texture = wgpuDeviceCreateTexture(tex->device, &texture_desc);
    
    // Upload image data to texture
    WGPUTexelCopyTextureInfo dest = {
        .texture = tex->texture,
        .mipLevel = 0,
        .origin = {0, 0, 0},
        .aspect = WGPUTextureAspect_All,
    };
    
    WGPUTexelCopyBufferLayout data_layout = {
        .offset = 0,
        .bytesPerRow = (uint32_t)(width * 4),
        .rowsPerImage = (uint32_t)height,
    };
    
    WGPUQueue queue = ug_context_get_queue(context);
    wgpuQueueWriteTexture(queue, &dest, image_data, width * height * 4, &data_layout, &texture_desc.size);
    
    // Free image data
    stbi_image_free(image_data);
    
    // Create texture view
    WGPUTextureViewDescriptor view_desc = {
        .format = WGPUTextureFormat_RGBA8Unorm,
        .dimension = WGPUTextureViewDimension_2D,
        .baseMipLevel = 0,
        .mipLevelCount = 1,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .aspect = WGPUTextureAspect_All,
    };
    tex->texture_view = wgpuTextureCreateView(tex->texture, &view_desc);
    
    // Create sampler
    WGPUSamplerDescriptor sampler_desc = {
        .addressModeU = WGPUAddressMode_ClampToEdge,
        .addressModeV = WGPUAddressMode_ClampToEdge,
        .addressModeW = WGPUAddressMode_ClampToEdge,
        .magFilter = WGPUFilterMode_Linear,
        .minFilter = WGPUFilterMode_Linear,
        .mipmapFilter = WGPUMipmapFilterMode_Nearest,
        .lodMinClamp = 0.0f,
        .lodMaxClamp = 1.0f,
        .compare = WGPUCompareFunction_Undefined,
        .maxAnisotropy = 1,
    };
    tex->sampler = wgpuDeviceCreateSampler(tex->device, &sampler_desc);
    
    return tex;
}

void ug_texture_destroy(UGTexture* texture) {
    if (!texture) {
        return;
    }
    
    if (texture->sampler) {
        wgpuSamplerRelease(texture->sampler);
    }
    if (texture->texture_view) {
        wgpuTextureViewRelease(texture->texture_view);
    }
    if (texture->texture) {
        wgpuTextureRelease(texture->texture);
    }
    
    free(texture);
}

WGPUTextureView ug_texture_get_view(UGTexture* texture) {
    return texture ? texture->texture_view : NULL;
}

WGPUSampler ug_texture_get_sampler(UGTexture* texture) {
    return texture ? texture->sampler : NULL;
}

void ug_texture_get_size(UGTexture* texture, int* width, int* height) {
    if (texture) {
        if (width) *width = texture->width;
        if (height) *height = texture->height;
    }
}

