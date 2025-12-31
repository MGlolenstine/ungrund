#include "ungrund.h"
#include <stdio.h>
#include <math.h>

// Maximum vertices for our demo
#define MAX_VERTICES 4096

// Render data
typedef struct {
    UGVertexBuffer* vertex_buffer;
    WGPURenderPipeline pipeline;
    size_t vertex_count;
} RenderData;

// Render callback
void render(UGContext* context, UGRenderFrame* frame, void* userdata) {
    RenderData* data = (RenderData*)userdata;
    (void)context;

    // Build vertex data using the new geometry helpers
    UGVertex2DColor vertices[MAX_VERTICES];
    size_t vertex_count = 0;

    // Animate based on time
    float time = (float)ug_get_time();

    // Draw a rectangle (red)
    ug_add_rect_2d_color(vertices, &vertex_count, 
                         -0.5f, 0.5f, 0.15f, 0.15f,
                         1.0f, 0.0f, 0.0f);

    // Draw a perfect circle (green) - height = 0 means use width for both
    ug_add_circle_2d_color(vertices, &vertex_count,
                          0.5f, 0.5f, 0.15f, 0.0f,
                          0.0f, 1.0f, 0.0f, 32);

    // Draw an ellipse (blue) - different width and height
    ug_add_circle_2d_color(vertices, &vertex_count,
                          -0.5f, -0.5f, 0.2f, 0.1f,
                          0.0f, 0.0f, 1.0f, 32);

    // Draw an animated rotating circle (yellow)
    float angle = time * 2.0f;
    float x = cosf(angle) * 0.3f;
    float y = sinf(angle) * 0.3f;
    ug_add_circle_2d_color(vertices, &vertex_count,
                          x, y, 0.08f, 0.0f,
                          1.0f, 1.0f, 0.0f, 24);

    // Draw multiple small rectangles in a pattern (cyan)
    for (int i = 0; i < 8; i++) {
        float circle_angle = (i / 8.0f) * 6.28318530718f + time;
        float cx = cosf(circle_angle) * 0.5f;
        float cy = sinf(circle_angle) * 0.5f;
        ug_add_rect_2d_color(vertices, &vertex_count,
                            cx, cy, 0.03f, 0.03f,
                            0.0f, 1.0f, 1.0f);
    }

    // Draw a pulsing circle at center (magenta)
    float pulse = 0.1f + 0.05f * sinf(time * 3.0f);
    ug_add_circle_2d_color(vertices, &vertex_count,
                          0.0f, 0.0f, pulse, 0.0f,
                          1.0f, 0.0f, 1.0f, 48);

    // Update vertex buffer
    ug_vertex_buffer_update(data->vertex_buffer, vertices, vertex_count);
    data->vertex_count = vertex_count;

    // Render
    UGRenderPass* pass = ug_render_pass_begin(frame, 0.1f, 0.1f, 0.15f, 1.0f);
    ug_render_pass_set_pipeline(pass, data->pipeline);
    ug_render_pass_set_vertex_buffer(pass, data->vertex_buffer);
    ug_render_pass_draw(pass, data->vertex_count);
    ug_render_pass_end(pass);
}

int main(void) {
    // Create window and context
    UGWindow* window = ug_window_create("Geometry Demo - Library Helpers", 800, 600);
    if (!window) {
        fprintf(stderr, "Failed to create window\n");
        return 1;
    }

    UGContext* context = ug_context_create(window);
    if (!context) {
        fprintf(stderr, "Failed to create context\n");
        ug_window_destroy(window);
        return 1;
    }

    // Create vertex buffer using the standard UGVertex2DColor format
    UGVertexBuffer* vertex_buffer = ug_vertex_buffer_create(context, sizeof(UGVertex2DColor), MAX_VERTICES);

    // Define vertex layout for UGVertex2DColor
    UGVertexAttribute vertex_attributes[2] = {
        {.format = WGPUVertexFormat_Float32x2, .offset = 0, .shader_location = 0},
        {.format = WGPUVertexFormat_Float32x3, .offset = 2 * sizeof(float), .shader_location = 1},
    };
    ug_vertex_buffer_set_layout(vertex_buffer, vertex_attributes, 2);

    // Build pipeline
    UGPipelineBuilder* pipeline_builder = ug_pipeline_builder_create(context, "examples/geometry_demo/shader.wgsl");
    if (!pipeline_builder) {
        fprintf(stderr, "Failed to create pipeline builder\n");
        return 1;
    }

    ug_pipeline_builder_set_vertex_buffer(pipeline_builder, ug_vertex_buffer_get_layout(vertex_buffer));
    WGPURenderPipeline pipeline = ug_pipeline_builder_build(pipeline_builder);

    printf("Geometry Demo - Using Library Helper Functions\n");
    printf("===============================================\n");
    printf("Demonstrating ug_add_rect_2d_color() and ug_add_circle_2d_color()\n");
    printf("- Red rectangle (top-left)\n");
    printf("- Green circle (top-right)\n");
    printf("- Blue ellipse (bottom-left)\n");
    printf("- Yellow orbiting circle\n");
    printf("- Cyan rectangles in a circle pattern\n");
    printf("- Magenta pulsing circle at center\n");
    printf("\nPress ESC to exit.\n\n");

    // Setup render data
    RenderData render_data = {
        .vertex_buffer = vertex_buffer,
        .pipeline = pipeline,
        .vertex_count = 0,
    };

    // Run
    ug_run(context, render, &render_data);

    // Cleanup
    ug_vertex_buffer_destroy(vertex_buffer);
    wgpuRenderPipelineRelease(pipeline);
    ug_pipeline_builder_destroy(pipeline_builder);
    ug_context_destroy(context);
    ug_window_destroy(window);

    return 0;
}

