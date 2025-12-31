#include "ungrund.h"
#include <GLFW/glfw3.h>
#include <stdlib.h>

#if defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#elif defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#else
#define GLFW_EXPOSE_NATIVE_X11
#endif

#include <GLFW/glfw3native.h>

struct UGWindow {
    GLFWwindow* handle;
    int width;
    int height;
};

static bool glfw_initialized = false;

UGWindow* ug_window_create(const char* title, int width, int height) {
    if (!glfw_initialized) {
        if (!glfwInit()) {
            return NULL;
        }
        glfw_initialized = true;
    }

    // Don't create an OpenGL context - we're using WebGPU
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow* handle = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!handle) {
        return NULL;
    }

    UGWindow* window = (UGWindow*)malloc(sizeof(UGWindow));
    window->handle = handle;
    window->width = width;
    window->height = height;

    return window;
}

void ug_window_destroy(UGWindow* window) {
    if (window) {
        glfwDestroyWindow(window->handle);
        free(window);
    }
}

bool ug_window_should_close(UGWindow* window) {
    return glfwWindowShouldClose(window->handle);
}

void ug_window_poll_events(UGWindow* window) {
    glfwPollEvents();
}

void* ug_window_get_native_handle(UGWindow* window) {
#if defined(__APPLE__)
    return (void*)glfwGetCocoaWindow(window->handle);
#elif defined(_WIN32)
    return (void*)glfwGetWin32Window(window->handle);
#else
    return (void*)(uintptr_t)glfwGetX11Window(window->handle);
#endif
}

void ug_window_get_size(UGWindow* window, int* width, int* height) {
    glfwGetFramebufferSize(window->handle, width, height);
}

double ug_get_time(void) {
    return glfwGetTime();
}

