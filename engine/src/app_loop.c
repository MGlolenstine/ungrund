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

    // Track time for delta_time calculation
    double last_time = ug_get_time();

    // Main loop
    while (!ug_window_should_close(window)) {
        ug_window_poll_events(window);

        // Calculate delta time
        double current_time = ug_get_time();
        float delta_time = (float)(current_time - last_time);
        last_time = current_time;

        // Begin render frame - handles surface texture acquisition and setup
        UGRenderFrame* frame = ug_begin_render_frame(context);
        if (!frame) {
            continue;
        }

        // Call user's render callback with delta_time
        render_callback(context, frame, delta_time, userdata);

        // End frame - handles command submission and presentation
        ug_end_render_frame(frame);
    }
}

