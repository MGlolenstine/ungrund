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

    // Input callbacks
    UGKeyCallback key_callback;
    void* key_userdata;

    UGMouseMoveCallback mouse_move_callback;
    void* mouse_move_userdata;

    UGMouseButtonCallback mouse_button_callback;
    void* mouse_button_userdata;
};

static bool glfw_initialized = false;

// Internal GLFW callback handlers
static void glfw_key_callback(GLFWwindow* handle, int key, int scancode, int action, int mods) {
    (void)scancode;
    (void)mods;

    UGWindow* window = (UGWindow*)glfwGetWindowUserPointer(handle);
    if (window && window->key_callback) {
        bool pressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
        window->key_callback(key, pressed, window->key_userdata);
    }
}

static void glfw_cursor_pos_callback(GLFWwindow* handle, double xpos, double ypos) {
    UGWindow* window = (UGWindow*)glfwGetWindowUserPointer(handle);
    if (window && window->mouse_move_callback) {
        window->mouse_move_callback(xpos, ypos, window->mouse_move_userdata);
    }
}

static void glfw_mouse_button_callback(GLFWwindow* handle, int button, int action, int mods) {
    (void)mods;

    UGWindow* window = (UGWindow*)glfwGetWindowUserPointer(handle);
    if (window && window->mouse_button_callback) {
        UGMouseButton ug_button;
        switch (button) {
            case GLFW_MOUSE_BUTTON_LEFT:
                ug_button = UG_MOUSE_BUTTON_LEFT;
                break;
            case GLFW_MOUSE_BUTTON_RIGHT:
                ug_button = UG_MOUSE_BUTTON_RIGHT;
                break;
            case GLFW_MOUSE_BUTTON_MIDDLE:
                ug_button = UG_MOUSE_BUTTON_MIDDLE;
                break;
            default:
                return; // Ignore other buttons
        }

        bool pressed = (action == GLFW_PRESS);
        window->mouse_button_callback(ug_button, pressed, window->mouse_button_userdata);
    }
}

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

    // Initialize callbacks to NULL
    window->key_callback = NULL;
    window->key_userdata = NULL;
    window->mouse_move_callback = NULL;
    window->mouse_move_userdata = NULL;
    window->mouse_button_callback = NULL;
    window->mouse_button_userdata = NULL;

    // Set window user pointer so we can retrieve it in callbacks
    glfwSetWindowUserPointer(handle, window);

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

bool ug_key_pressed(UGWindow* window, int key) {
    return glfwGetKey(window->handle, key) == GLFW_PRESS;
}

// Input callback setters
void ug_window_set_key_callback(UGWindow* window, UGKeyCallback callback, void* userdata) {
    if (!window) {
        return;
    }

    window->key_callback = callback;
    window->key_userdata = userdata;

    // Register GLFW callback if user callback is set, otherwise unregister
    if (callback) {
        glfwSetKeyCallback(window->handle, glfw_key_callback);
    } else {
        glfwSetKeyCallback(window->handle, NULL);
    }
}

void ug_window_set_mouse_move_callback(UGWindow* window, UGMouseMoveCallback callback, void* userdata) {
    if (!window) {
        return;
    }

    window->mouse_move_callback = callback;
    window->mouse_move_userdata = userdata;

    // Register GLFW callback if user callback is set, otherwise unregister
    if (callback) {
        glfwSetCursorPosCallback(window->handle, glfw_cursor_pos_callback);
    } else {
        glfwSetCursorPosCallback(window->handle, NULL);
    }
}

void ug_window_set_mouse_button_callback(UGWindow* window, UGMouseButtonCallback callback, void* userdata) {
    if (!window) {
        return;
    }

    window->mouse_button_callback = callback;
    window->mouse_button_userdata = userdata;

    // Register GLFW callback if user callback is set, otherwise unregister
    if (callback) {
        glfwSetMouseButtonCallback(window->handle, glfw_mouse_button_callback);
    } else {
        glfwSetMouseButtonCallback(window->handle, NULL);
    }
}

