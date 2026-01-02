#include "ungrund.h"
#include <webgpu/webgpu.h>

#if defined(__linux__)

#include <GLFW/glfw3.h>

// Define both X11 and Wayland native support
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3native.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Platform detection
typedef enum {
    UG_PLATFORM_UNKNOWN,
    UG_PLATFORM_X11,
    UG_PLATFORM_WAYLAND
} UGLinuxPlatform;

// Detect which platform we're running on
static UGLinuxPlatform detect_platform(void) {
    // Check environment variables to determine the platform
    const char* wayland_display = getenv("WAYLAND_DISPLAY");
    const char* xdg_session_type = getenv("XDG_SESSION_TYPE");
    
    // If WAYLAND_DISPLAY is set, we're likely on Wayland
    if (wayland_display && strlen(wayland_display) > 0) {
        return UG_PLATFORM_WAYLAND;
    }
    
    // Check XDG_SESSION_TYPE
    if (xdg_session_type) {
        if (strcmp(xdg_session_type, "wayland") == 0) {
            return UG_PLATFORM_WAYLAND;
        } else if (strcmp(xdg_session_type, "x11") == 0) {
            return UG_PLATFORM_X11;
        }
    }
    
    // Default to X11 if we can't determine
    // Most systems will have DISPLAY set for X11
    const char* display = getenv("DISPLAY");
    if (display && strlen(display) > 0) {
        return UG_PLATFORM_X11;
    }
    
    return UG_PLATFORM_UNKNOWN;
}

// Create WebGPU surface for Linux (supports both X11 and Wayland)
WGPUSurface ug_create_linux_surface(WGPUInstance instance, void* window_handle) {
    GLFWwindow* glfw_window = (GLFWwindow*)window_handle;
    WGPUSurface surface = NULL;
    
    UGLinuxPlatform platform = detect_platform();
    
    if (platform == UG_PLATFORM_WAYLAND) {
        // Wayland surface creation
        struct wl_display* wl_display = glfwGetWaylandDisplay();
        struct wl_surface* wl_surface = glfwGetWaylandWindow(glfw_window);
        
        if (!wl_display || !wl_surface) {
            fprintf(stderr, "Failed to get Wayland display or surface from GLFW\n");
            fprintf(stderr, "Make sure GLFW was compiled with Wayland support\n");
            return NULL;
        }
        
        WGPUSurfaceSourceWaylandSurface wayland_surface_source = {
            .chain = {
                .next = NULL,
                .sType = WGPUSType_SurfaceSourceWaylandSurface,
            },
            .display = wl_display,
            .surface = wl_surface,
        };
        
        WGPUSurfaceDescriptor surface_desc = {
            .nextInChain = (const WGPUChainedStruct*)&wayland_surface_source,
            .label = {"Main Surface (Wayland)", WGPU_STRLEN},
        };
        
        surface = wgpuInstanceCreateSurface(instance, &surface_desc);
        
        if (surface) {
            printf("Created Wayland surface\n");
        }
    } else if (platform == UG_PLATFORM_X11) {
        // X11 surface creation
        Display* x11_display = glfwGetX11Display();
        Window x11_window = glfwGetX11Window(glfw_window);
        
        if (!x11_display) {
            fprintf(stderr, "Failed to get X11 display from GLFW\n");
            fprintf(stderr, "Make sure GLFW was compiled with X11 support\n");
            return NULL;
        }
        
        WGPUSurfaceSourceXlibWindow x11_surface_source = {
            .chain = {
                .next = NULL,
                .sType = WGPUSType_SurfaceSourceXlibWindow,
            },
            .display = x11_display,
            .window = (uint32_t)x11_window,
        };
        
        WGPUSurfaceDescriptor surface_desc = {
            .nextInChain = (const WGPUChainedStruct*)&x11_surface_source,
            .label = {"Main Surface (X11)", WGPU_STRLEN},
        };
        
        surface = wgpuInstanceCreateSurface(instance, &surface_desc);
        
        if (surface) {
            printf("Created X11 surface\n");
        }
    } else {
        fprintf(stderr, "Unknown Linux platform - could not detect X11 or Wayland\n");
        fprintf(stderr, "Please set WAYLAND_DISPLAY or DISPLAY environment variable\n");
        return NULL;
    }
    
    return surface;
}

// Get X11 display (for compatibility with existing API)
void* ug_get_x11_display(void) {
    UGLinuxPlatform platform = detect_platform();
    
    if (platform == UG_PLATFORM_X11) {
        return glfwGetX11Display();
    }
    
    return NULL;
}

// Get Wayland display
void* ug_get_wayland_display(void) {
    UGLinuxPlatform platform = detect_platform();
    
    if (platform == UG_PLATFORM_WAYLAND) {
        return glfwGetWaylandDisplay();
    }
    
    return NULL;
}

#endif // __linux__

