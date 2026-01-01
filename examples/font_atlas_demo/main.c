#include "ungrund.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_VERTICES 10000

typedef struct {
    UGFontAtlas* font;
    UGVertexBuffer* vertex_buffer;
    void* vertices;
    size_t vertex_count;
    float time;
    UGWindow* window;
} AppState;

void on_key_event(int key, bool pressed, void* userdata) {
    AppState* state = (AppState*)userdata;

    if (key == UG_KEY_ESCAPE && pressed) {
        // Close window on ESC
        // Note: ug_run will exit when window should close
        (void)state;  // Window will be closed by GLFW automatically
    }
}

void render(UGContext* context, UGRenderFrame* frame, float delta_time, void* userdata) {
    AppState* state = (AppState*)userdata;

    // Update time
    state->time += delta_time;

    // Reset vertex count
    state->vertex_count = 0;

    // Get screen dimensions
    int width, height;
    ug_window_get_size(ug_context_get_window(context), &width, &height);

    // Add various text samples using pixel coordinates
    // Title
    ug_font_atlas_add_text_px(state->font, state->vertices, &state->vertex_count,
                              "Font Atlas Demo", 50, 50, context, 1.0f, 1.0f, 1.0f, 1.0f);

    // Different colors
    ug_font_atlas_add_text_px(state->font, state->vertices, &state->vertex_count,
                              "Red Text", 50, 100, context, 1.0f, 0.0f, 0.0f, 1.0f);

    ug_font_atlas_add_text_px(state->font, state->vertices, &state->vertex_count,
                              "Green Text", 50, 130, context, 0.0f, 1.0f, 0.0f, 1.0f);

    ug_font_atlas_add_text_px(state->font, state->vertices, &state->vertex_count,
                              "Blue Text", 50, 160, context, 0.0f, 0.5f, 1.0f, 1.0f);

    // Animated text with pulsing alpha
    float alpha = (sinf(state->time * 2.0f) + 1.0f) * 0.5f;
    ug_font_atlas_add_text_px(state->font, state->vertices, &state->vertex_count,
                              "Pulsing Text", 50, 210, context, 1.0f, 1.0f, 0.0f, alpha);

    // Multiple lines
    ug_font_atlas_add_text_px(state->font, state->vertices, &state->vertex_count,
                              "Line 1: The quick brown fox", 50, 260, context, 0.8f, 0.8f, 0.8f, 1.0f);

    ug_font_atlas_add_text_px(state->font, state->vertices, &state->vertex_count,
                              "Line 2: jumps over the lazy dog", 50, 290, context, 0.8f, 0.8f, 0.8f, 1.0f);

    // Numbers and symbols
    ug_font_atlas_add_text_px(state->font, state->vertices, &state->vertex_count,
                              "Numbers: 0123456789", 50, 340, context, 0.5f, 1.0f, 0.5f, 1.0f);

    ug_font_atlas_add_text_px(state->font, state->vertices, &state->vertex_count,
                              "Symbols: !@#$%^&*()", 50, 370, context, 1.0f, 0.5f, 0.5f, 1.0f);

    // FPS counter
    char fps_text[64] = {0};
    snprintf(fps_text, sizeof(fps_text), "FPS: %.1f", 1.0/delta_time);
    ug_font_atlas_add_text_px(state->font, state->vertices, &state->vertex_count,
                              fps_text, width - 350, 30, context, 0.0f, 1.0f, 1.0f, 1.0f);

    // Instructions
    ug_font_atlas_add_text_px(state->font, state->vertices, &state->vertex_count,
                              "Press ESC to exit", 50, height - 50, context, 0.6f, 0.6f, 0.6f, 1.0f);
    
    // Debug: Print vertex count on first frame
    static int frame_count = 0;
    if (frame_count == 0) {
        printf("First frame: Generated %zu vertices\n", state->vertex_count);
    }
    frame_count++;

    // Update vertex buffer
    ug_vertex_buffer_update(state->vertex_buffer, state->vertices, state->vertex_count);

    // Render
    UGRenderPass* pass = ug_render_pass_begin(frame, 0.1f, 0.1f, 0.15f, 1.0f);

    ug_render_pass_set_pipeline(pass, ug_font_atlas_get_pipeline(state->font));
    ug_render_pass_set_bind_group(pass, 0, ug_font_atlas_get_bind_group(state->font));
    ug_render_pass_set_vertex_buffer(pass, state->vertex_buffer);
    ug_render_pass_draw(pass, state->vertex_count);

    ug_render_pass_end(pass);
}

int main(void) {
    // Create window and context
    UGWindow* window = ug_window_create("Font Atlas Demo", 800, 600);
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
    
    // Create font atlas
    // Try to find a system font (macOS path, adjust for your system)
    const char* font_path = "/System/Library/Fonts/Helvetica.ttc";
    UGFontAtlas* font = ug_font_atlas_create(context, font_path, 32, 512, 512);
    if (!font) {
        fprintf(stderr, "Failed to create font atlas\n");
        fprintf(stderr, "Make sure font file exists: %s\n", font_path);
        ug_context_destroy(context);
        ug_window_destroy(window);
        return 1;
    }

    printf("Font atlas created successfully!\n");
    printf("Atlas size: 512x512, Font size: 32px\n");

    // Create vertex buffer for text rendering
    size_t vertex_size = ug_font_atlas_get_vertex_size();
    UGVertexBuffer* vertex_buffer = ug_vertex_buffer_create(context, vertex_size, MAX_VERTICES);

    // Set vertex layout
    UGVertexAttribute attributes[3];
    ug_font_atlas_get_vertex_attributes(attributes);
    ug_vertex_buffer_set_layout(vertex_buffer, attributes, 3);

    // Allocate vertex array
    void* vertices = malloc(vertex_size * MAX_VERTICES);
    if (!vertices) {
        fprintf(stderr, "Failed to allocate vertex buffer\n");
        ug_font_atlas_destroy(font);
        ug_context_destroy(context);
        ug_window_destroy(window);
        return 1;
    }

    // Setup app state
    AppState state = {
        .font = font,
        .vertex_buffer = vertex_buffer,
        .vertices = vertices,
        .vertex_count = 0,
        .time = 0.0f,
        .window = window,
    };

    // Set up input callback for ESC key
    ug_window_set_key_callback(window, on_key_event, &state);

    printf("Starting render loop...\n");
    printf("Press ESC to exit\n");

    // Run application loop
    ug_run(context, render, &state);

    // Cleanup
    printf("Cleaning up...\n");
    free(vertices);
    ug_vertex_buffer_destroy(vertex_buffer);
    ug_font_atlas_destroy(font);
    ug_context_destroy(context);
    ug_window_destroy(window);

    printf("Done!\n");
    return 0;
}

