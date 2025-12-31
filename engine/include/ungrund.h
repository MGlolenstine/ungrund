#ifndef UNGRUND_H
#define UNGRUND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct UGWindow UGWindow;
typedef struct UGRenderer UGRenderer;

// Window management
UGWindow* ug_window_create(const char* title, int width, int height);
void ug_window_destroy(UGWindow* window);
bool ug_window_should_close(UGWindow* window);
void ug_window_poll_events(UGWindow* window);
void* ug_window_get_native_handle(UGWindow* window);
void ug_window_get_size(UGWindow* window, int* width, int* height);

// Renderer management
UGRenderer* ug_renderer_create(UGWindow* window);
void ug_renderer_destroy(UGRenderer* renderer);
void ug_renderer_begin_frame(UGRenderer* renderer);
void ug_renderer_end_frame(UGRenderer* renderer);
void ug_renderer_clear(UGRenderer* renderer, float r, float g, float b, float a);

// Utility functions
double ug_get_time(void);

// Platform-specific helpers
#if defined(__APPLE__)
void* ug_create_metal_layer(void* ns_window_ptr);
#endif

#ifdef __cplusplus
}
#endif

#endif // UNGRUND_H

