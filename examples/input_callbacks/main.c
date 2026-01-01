#include "ungrund.h"
#include <stdio.h>
#include <stdlib.h>

// Callback function for key events
void on_key_event(int key, bool pressed, void* userdata) {
    (void)userdata;
    const char* action = pressed ? "pressed" : "released";
    printf("Key %d %s\n", key, action);
    
    // Print readable key names for common keys
    if (key == UG_KEY_ESCAPE) {
        printf("  (ESC key %s)\n", action);
    } else if (key == UG_KEY_SPACE) {
        printf("  (SPACE key %s)\n", action);
    } else if (key >= 32 && key <= 126) {
        printf("  ('%c' key %s)\n", (char)key, action);
    }
}

// Callback function for mouse movement
void on_mouse_move(double x, double y, void* userdata) {
    (void)userdata;
    printf("Mouse moved to: (%.1f, %.1f)\n", x, y);
}

// Callback function for mouse button events
void on_mouse_button(UGMouseButton button, bool pressed, void* userdata) {
    (void)userdata;
    const char* action = pressed ? "pressed" : "released";
    const char* button_name;
    
    switch (button) {
        case UG_MOUSE_BUTTON_LEFT:
            button_name = "LEFT";
            break;
        case UG_MOUSE_BUTTON_RIGHT:
            button_name = "RIGHT";
            break;
        case UG_MOUSE_BUTTON_MIDDLE:
            button_name = "MIDDLE";
            break;
        default:
            button_name = "UNKNOWN";
            break;
    }
    
    printf("Mouse button %s %s\n", button_name, action);
}

// Simple render callback
void render(UGContext* context, UGRenderFrame* frame, float delta_time, void* userdata) {
    (void)context;
    (void)delta_time;
    (void)userdata;
    
    // Simple clear to a nice blue color
    UGRenderPass* pass = ug_render_pass_begin(frame, 0.1f, 0.2f, 0.4f, 1.0f);
    ug_render_pass_end(pass);
}

int main(void) {
    printf("Input Callbacks Test\n");
    printf("====================\n");
    printf("Move your mouse, click buttons, and press keys to see callback output.\n");
    printf("Press ESC to close the window.\n\n");
    
    // Create window
    UGWindow* window = ug_window_create("Input Callbacks Test", 800, 600);
    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        return 1;
    }
    
    // Create context
    UGContext* context = ug_context_create(window);
    if (!context) {
        fprintf(stderr, "Failed to create context\n");
        ug_window_destroy(window);
        return 1;
    }
    
    // Set up input callbacks
    ug_window_set_key_callback(window, on_key_event, NULL);
    ug_window_set_mouse_move_callback(window, on_mouse_move, NULL);
    ug_window_set_mouse_button_callback(window, on_mouse_button, NULL);
    
    printf("Callbacks registered. Window is now active.\n\n");
    
    // Run the application loop
    ug_run(context, render, NULL);
    
    // Cleanup
    ug_context_destroy(context);
    ug_window_destroy(window);
    
    printf("\nApplication closed.\n");
    return 0;
}

