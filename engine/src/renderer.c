#include "ungrund.h"
#include <stdlib.h>

// Minimal renderer stub - examples will use wgpu directly
struct UGRenderer {
    UGWindow* window;
};

UGRenderer* ug_renderer_create(UGWindow* window) {
    UGRenderer* renderer = (UGRenderer*)malloc(sizeof(UGRenderer));
    renderer->window = window;
    return renderer;
}

void ug_renderer_destroy(UGRenderer* renderer) {
    if (renderer) {
        free(renderer);
    }
}

void ug_renderer_begin_frame(UGRenderer* renderer) {
    // Stub - examples will handle their own rendering
}

void ug_renderer_end_frame(UGRenderer* renderer) {
    // Stub - examples will handle their own rendering
}

void ug_renderer_clear(UGRenderer* renderer, float r, float g, float b, float a) {
    // Stub - examples will handle their own rendering
}

