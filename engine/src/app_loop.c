#include "ungrund.h"
#include <webgpu/webgpu.h>

void ug_run(UGContext* context, UGRenderCallback render_callback, void* userdata) {
    if (!context || !render_callback) {
        return;
    }

    UGWindow* window = ug_context_get_window(context);
    if (!window) {
        return;
    }

    // Main loop
    while (!ug_window_should_close(window)) {
        ug_window_poll_events(window);

        // Begin render frame - handles surface texture acquisition and setup
        UGRenderFrame* frame = ug_begin_render_frame(context);
        if (!frame) {
            continue;
        }

        // Call user's render callback
        render_callback(context, frame, userdata);

        // End frame - handles command submission and presentation
        ug_end_render_frame(frame);
    }
}

