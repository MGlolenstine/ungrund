#include "ungrund.h"
#include <webgpu/webgpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#endif

// Internal structures
typedef struct {
    WGPUAdapter adapter;
    bool request_ended;
} AdapterUserData;

typedef struct {
    WGPUDevice device;
    bool request_ended;
} DeviceUserData;

struct UGContext {
    UGWindow* window;
    WGPUInstance instance;
    WGPUAdapter adapter;
    WGPUDevice device;
    WGPUQueue queue;
    WGPUSurface surface;
    WGPUTextureFormat surface_format;
    WGPUPresentMode present_mode;
};

struct UGContextBuilder {
    UGWindow* window;
    WGPUPowerPreference power_preference;
    WGPUPresentMode present_mode;
    WGPUTextureFormat surface_format;
};

// Adapter request callback
static void on_adapter_request_ended(WGPURequestAdapterStatus status, WGPUAdapter adapter,
                                      WGPUStringView message, void* userdata1, void* userdata2) {
    (void)userdata2;
    AdapterUserData* data = (AdapterUserData*)userdata1;
    if (status == WGPURequestAdapterStatus_Success) {
        data->adapter = adapter;
    } else {
        fprintf(stderr, "Failed to get adapter: %.*s\n", (int)message.length, 
                message.data ? message.data : "unknown error");
    }
    data->request_ended = true;
}

// Device request callback
static void on_device_request_ended(WGPURequestDeviceStatus status, WGPUDevice device,
                                     WGPUStringView message, void* userdata1, void* userdata2) {
    (void)userdata2;
    DeviceUserData* data = (DeviceUserData*)userdata1;
    if (status == WGPURequestDeviceStatus_Success) {
        data->device = device;
    } else {
        fprintf(stderr, "Failed to get device: %.*s\n", (int)message.length, 
                message.data ? message.data : "unknown error");
    }
    data->request_ended = true;
}

// Create surface for the window
static WGPUSurface create_surface(WGPUInstance instance, UGWindow* window) {
    WGPUSurface surface = NULL;

#if defined(__APPLE__)
    void* ns_window = ug_window_get_native_handle(window);
    void* metal_layer = ug_create_metal_layer(ns_window);

    WGPUSurfaceSourceMetalLayer metal_surface_source = {
        .chain = {
            .next = NULL,
            .sType = WGPUSType_SurfaceSourceMetalLayer,
        },
        .layer = metal_layer,
    };

    WGPUSurfaceDescriptor surface_desc = {
        .nextInChain = (const WGPUChainedStruct*)&metal_surface_source,
        .label = {"Main Surface", WGPU_STRLEN},
    };

    surface = wgpuInstanceCreateSurface(instance, &surface_desc);
#elif defined(_WIN32)
    // Windows surface creation
    void* hwnd = ug_window_get_native_handle(window);
    void* hinstance = GetModuleHandle(NULL);

    WGPUSurfaceSourceWindowsHWND windows_surface_source = {
        .chain = {
            .next = NULL,
            .sType = WGPUSType_SurfaceSourceWindowsHWND,
        },
        .hinstance = hinstance,
        .hwnd = hwnd,
    };

    WGPUSurfaceDescriptor surface_desc = {
        .nextInChain = (const WGPUChainedStruct*)&windows_surface_source,
        .label = {"Main Surface", WGPU_STRLEN},
    };

    surface = wgpuInstanceCreateSurface(instance, &surface_desc);
#else
    // Linux X11 surface creation
    void* x11_window = ug_window_get_native_handle(window);
    void* x11_display = ug_window_get_x11_display();

    WGPUSurfaceSourceXlibWindow x11_surface_source = {
        .chain = {
            .next = NULL,
            .sType = WGPUSType_SurfaceSourceXlibWindow,
        },
        .display = x11_display,
        .window = (uint32_t)(uintptr_t)x11_window,
    };

    WGPUSurfaceDescriptor surface_desc = {
        .nextInChain = (const WGPUChainedStruct*)&x11_surface_source,
        .label = {"Main Surface", WGPU_STRLEN},
    };

    surface = wgpuInstanceCreateSurface(instance, &surface_desc);
#endif

    return surface;
}

// Internal context creation function
static UGContext* create_context_internal(UGWindow* window, WGPUPowerPreference power_preference,
                                          WGPUPresentMode present_mode, WGPUTextureFormat surface_format) {
    UGContext* context = (UGContext*)calloc(1, sizeof(UGContext));
    if (!context) {
        return NULL;
    }

    context->window = window;
    context->surface_format = surface_format;
    context->present_mode = present_mode;

    // Create WebGPU instance
    WGPUInstanceDescriptor instance_desc = {0};
    context->instance = wgpuCreateInstance(&instance_desc);
    if (!context->instance) {
        fprintf(stderr, "Failed to create WebGPU instance\n");
        free(context);
        return NULL;
    }

    // Create surface
    context->surface = create_surface(context->instance, window);
    if (!context->surface) {
        fprintf(stderr, "Failed to create surface\n");
#if defined(__APPLE__)
        fprintf(stderr, "  Platform: macOS (Metal)\n");
#elif defined(_WIN32)
        fprintf(stderr, "  Platform: Windows\n");
#else
        fprintf(stderr, "  Platform: Linux (X11)\n");
        fprintf(stderr, "  Make sure X11 display is available\n");
#endif
        wgpuInstanceRelease(context->instance);
        free(context);
        return NULL;
    }

    // Request adapter
    WGPURequestAdapterOptions adapter_opts = {
        .compatibleSurface = context->surface,
        .powerPreference = power_preference,
    };

    AdapterUserData adapter_data = {0};
    WGPURequestAdapterCallbackInfo adapter_callback_info = {
        .mode = WGPUCallbackMode_AllowSpontaneous,
        .callback = on_adapter_request_ended,
        .userdata1 = &adapter_data,
    };
    wgpuInstanceRequestAdapter(context->instance, &adapter_opts, adapter_callback_info);

    if (!adapter_data.adapter) {
        fprintf(stderr, "Failed to get adapter\n");
        wgpuSurfaceRelease(context->surface);
        wgpuInstanceRelease(context->instance);
        free(context);
        return NULL;
    }
    context->adapter = adapter_data.adapter;

    // Request device
    WGPUDeviceDescriptor device_desc = {0};
    DeviceUserData device_data = {0};
    WGPURequestDeviceCallbackInfo device_callback_info = {
        .mode = WGPUCallbackMode_AllowSpontaneous,
        .callback = on_device_request_ended,
        .userdata1 = &device_data,
    };
    wgpuAdapterRequestDevice(context->adapter, &device_desc, device_callback_info);

    if (!device_data.device) {
        fprintf(stderr, "Failed to get device\n");
        wgpuAdapterRelease(context->adapter);
        wgpuSurfaceRelease(context->surface);
        wgpuInstanceRelease(context->instance);
        free(context);
        return NULL;
    }
    context->device = device_data.device;

    // Get queue
    context->queue = wgpuDeviceGetQueue(context->device);

    // Configure surface
    int width, height;
    ug_window_get_size(window, &width, &height);

    WGPUSurfaceConfiguration config = {
        .device = context->device,
        .format = surface_format,
        .usage = WGPUTextureUsage_RenderAttachment,
        .width = (uint32_t)width,
        .height = (uint32_t)height,
        .presentMode = present_mode,
    };
    wgpuSurfaceConfigure(context->surface, &config);

    return context;
}

// Simple context creation with defaults
UGContext* ug_context_create(UGWindow* window) {
    return create_context_internal(window,
                                   WGPUPowerPreference_HighPerformance,
                                   WGPUPresentMode_Fifo,
                                   WGPUTextureFormat_BGRA8Unorm);
}

// Builder pattern implementation
UGContextBuilder* ug_context_builder_create(UGWindow* window) {
    UGContextBuilder* builder = (UGContextBuilder*)calloc(1, sizeof(UGContextBuilder));
    if (!builder) {
        return NULL;
    }

    builder->window = window;
    // Set defaults
    builder->power_preference = WGPUPowerPreference_HighPerformance;
    builder->present_mode = WGPUPresentMode_Fifo;
    builder->surface_format = WGPUTextureFormat_BGRA8Unorm;

    return builder;
}

void ug_context_builder_set_power_preference(UGContextBuilder* builder, WGPUPowerPreference preference) {
    if (builder) {
        builder->power_preference = preference;
    }
}

void ug_context_builder_set_present_mode(UGContextBuilder* builder, WGPUPresentMode mode) {
    if (builder) {
        builder->present_mode = mode;
    }
}

void ug_context_builder_set_surface_format(UGContextBuilder* builder, WGPUTextureFormat format) {
    if (builder) {
        builder->surface_format = format;
    }
}

UGContext* ug_context_builder_build(UGContextBuilder* builder) {
    if (!builder) {
        return NULL;
    }

    return create_context_internal(builder->window,
                                   builder->power_preference,
                                   builder->present_mode,
                                   builder->surface_format);
}

void ug_context_builder_destroy(UGContextBuilder* builder) {
    if (builder) {
        free(builder);
    }
}

// Context accessors
UGWindow* ug_context_get_window(UGContext* context) {
    return context ? context->window : NULL;
}

WGPUDevice ug_context_get_device(UGContext* context) {
    return context ? context->device : NULL;
}

WGPUQueue ug_context_get_queue(UGContext* context) {
    return context ? context->queue : NULL;
}

WGPUSurface ug_context_get_surface(UGContext* context) {
    return context ? context->surface : NULL;
}

WGPUTextureFormat ug_context_get_surface_format(UGContext* context) {
    return context ? context->surface_format : WGPUTextureFormat_Undefined;
}

void ug_context_get_surface_size(UGContext* context, uint32_t* width, uint32_t* height) {
    if (context && width && height) {
        int w, h;
        ug_window_get_size(context->window, &w, &h);
        *width = (uint32_t)w;
        *height = (uint32_t)h;
    }
}

// Context cleanup
void ug_context_destroy(UGContext* context) {
    if (context) {
        if (context->queue) wgpuQueueRelease(context->queue);
        if (context->device) wgpuDeviceRelease(context->device);
        if (context->adapter) wgpuAdapterRelease(context->adapter);
        if (context->surface) wgpuSurfaceRelease(context->surface);
        if (context->instance) wgpuInstanceRelease(context->instance);
        free(context);
    }
}


