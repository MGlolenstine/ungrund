#include "ungrund.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Vertex structure
typedef struct {
    float position[2];
    float color[3];
} Vertex;

// Game constants
#define PADDLE_WIDTH 0.02f
#define PADDLE_HEIGHT 0.2f
#define BALL_SIZE 0.02f
#define PADDLE_SPEED 1.0f
#define BALL_SPEED 0.8f
#define MAX_VERTICES 1024

// Game state
typedef struct {
    // Paddles
    float left_paddle_y;
    float right_paddle_y;

    // Ball
    float ball_x;
    float ball_y;
    float ball_vx;
    float ball_vy;

    // Scores
    int left_score;
    int right_score;

    // Rendering (simplified - no direct WebGPU types!)
    UGVertexBuffer* vertex_buffer;
    WGPURenderPipeline pipeline;
    size_t vertex_count;

    // Timing
    double last_time;

    // Input state (for callback-based input)
    bool key_w_pressed;
    bool key_s_pressed;
    bool key_i_pressed;
    bool key_k_pressed;
} GameState;

// Initialize game state
void init_game(GameState* game) {
    game->left_paddle_y = 0.0f;
    game->right_paddle_y = 0.0f;
    game->ball_x = 0.0f;
    game->ball_y = 0.0f;
    game->ball_vx = BALL_SPEED;
    game->ball_vy = BALL_SPEED * 0.5f;
    game->left_score = 0;
    game->right_score = 0;
    game->last_time = ug_get_time();

    // Initialize input state
    game->key_w_pressed = false;
    game->key_s_pressed = false;
    game->key_i_pressed = false;
    game->key_k_pressed = false;
}

// Reset ball to center
void reset_ball(GameState* game) {
    game->ball_x = 0.0f;
    game->ball_y = 0.0f;
    // Randomize direction slightly
    game->ball_vx = (game->ball_vx > 0 ? -BALL_SPEED : BALL_SPEED);
    game->ball_vy = BALL_SPEED * ((float)rand() / RAND_MAX - 0.5f);
}

// Add a rectangle to the vertex buffer
void add_rect(Vertex* vertices, size_t* count, float x, float y, float w, float h, float r, float g, float b) {
    // Two triangles to form a rectangle
    vertices[(*count)++] = (Vertex){{x - w, y - h}, {r, g, b}};
    vertices[(*count)++] = (Vertex){{x + w, y - h}, {r, g, b}};
    vertices[(*count)++] = (Vertex){{x - w, y + h}, {r, g, b}};

    vertices[(*count)++] = (Vertex){{x - w, y + h}, {r, g, b}};
    vertices[(*count)++] = (Vertex){{x + w, y - h}, {r, g, b}};
    vertices[(*count)++] = (Vertex){{x + w, y + h}, {r, g, b}};
}

// 7-segment display style digit rendering
// Segments:  0
//           1 2
//            3
//           4 5
//            6
void draw_digit(Vertex* vertices, size_t* count, int digit, float x, float y, float size, float r, float g, float b) {
    // Define which segments are on for each digit (0-9)
    static const bool segments[10][7] = {
        {1, 1, 1, 0, 1, 1, 1}, // 0
        {0, 0, 1, 0, 0, 1, 0}, // 1
        {1, 0, 1, 1, 1, 0, 1}, // 2
        {1, 0, 1, 1, 0, 1, 1}, // 3
        {0, 1, 1, 1, 0, 1, 0}, // 4
        {1, 1, 0, 1, 0, 1, 1}, // 5
        {1, 1, 0, 1, 1, 1, 1}, // 6
        {1, 0, 1, 0, 0, 1, 0}, // 7
        {1, 1, 1, 1, 1, 1, 1}, // 8
        {1, 1, 1, 1, 0, 1, 1}, // 9
    };

    if (digit < 0 || digit > 9) return;

    float seg_width = size * 0.15f;
    float seg_length = size * 0.4f;

    // Segment 0 (top horizontal)
    if (segments[digit][0]) {
        add_rect(vertices, count, x, y + size, seg_length, seg_width, r, g, b);
    }

    // Segment 1 (top-left vertical)
    if (segments[digit][1]) {
        add_rect(vertices, count, x - seg_length, y + size * 0.5f, seg_width, seg_length, r, g, b);
    }

    // Segment 2 (top-right vertical)
    if (segments[digit][2]) {
        add_rect(vertices, count, x + seg_length, y + size * 0.5f, seg_width, seg_length, r, g, b);
    }

    // Segment 3 (middle horizontal)
    if (segments[digit][3]) {
        add_rect(vertices, count, x, y, seg_length, seg_width, r, g, b);
    }

    // Segment 4 (bottom-left vertical)
    if (segments[digit][4]) {
        add_rect(vertices, count, x - seg_length, y - size * 0.5f, seg_width, seg_length, r, g, b);
    }

    // Segment 5 (bottom-right vertical)
    if (segments[digit][5]) {
        add_rect(vertices, count, x + seg_length, y - size * 0.5f, seg_width, seg_length, r, g, b);
    }

    // Segment 6 (bottom horizontal)
    if (segments[digit][6]) {
        add_rect(vertices, count, x, y - size, seg_length, seg_width, r, g, b);
    }
}

// Draw a number (supports 0-99)
void draw_number(Vertex* vertices, size_t* count, int number, float x, float y, float size, float r, float g, float b) {
    if (number < 0) number = 0;
    if (number > 99) number = 99;

    if (number >= 10) {
        // Draw tens digit
        draw_digit(vertices, count, number / 10, x - size * 0.6f, y, size, r, g, b);
        // Draw ones digit
        draw_digit(vertices, count, number % 10, x + size * 0.6f, y, size, r, g, b);
    } else {
        // Single digit, centered
        draw_digit(vertices, count, number, x, y, size, r, g, b);
    }
}

// Key callback handler
void on_key_event(int key, bool pressed, void* userdata) {
    GameState* game = (GameState*)userdata;

    // Update key state based on which key was pressed/released
    if (key == UG_KEY_W) {
        game->key_w_pressed = pressed;
    } else if (key == UG_KEY_S) {
        game->key_s_pressed = pressed;
    } else if (key == UG_KEY_I) {
        game->key_i_pressed = pressed;
    } else if (key == UG_KEY_K) {
        game->key_k_pressed = pressed;
    }
}

// Update game logic
void update_game(GameState* game, float dt) {
    // Handle input for left paddle (W/S) - now using callback state
    if (game->key_w_pressed) {
        game->left_paddle_y += PADDLE_SPEED * dt;
        if (game->left_paddle_y > 1.0f - PADDLE_HEIGHT) game->left_paddle_y = 1.0f - PADDLE_HEIGHT;
    }
    if (game->key_s_pressed) {
        game->left_paddle_y -= PADDLE_SPEED * dt;
        if (game->left_paddle_y < -1.0f + PADDLE_HEIGHT) game->left_paddle_y = -1.0f + PADDLE_HEIGHT;
    }

    // Handle input for right paddle (I/K) - now using callback state
    if (game->key_i_pressed) {
        game->right_paddle_y += PADDLE_SPEED * dt;
        if (game->right_paddle_y > 1.0f - PADDLE_HEIGHT) game->right_paddle_y = 1.0f - PADDLE_HEIGHT;
    }
    if (game->key_k_pressed) {
        game->right_paddle_y -= PADDLE_SPEED * dt;
        if (game->right_paddle_y < -1.0f + PADDLE_HEIGHT) game->right_paddle_y = -1.0f + PADDLE_HEIGHT;
    }
    
    // Update ball position
    game->ball_x += game->ball_vx * dt;
    game->ball_y += game->ball_vy * dt;
    
    // Ball collision with top/bottom walls
    if (game->ball_y > 1.0f - BALL_SIZE || game->ball_y < -1.0f + BALL_SIZE) {
        game->ball_vy = -game->ball_vy;
    }
    
    // Ball collision with paddles
    float left_paddle_x = -0.95f;
    float right_paddle_x = 0.95f;
    
    // Left paddle collision
    if (game->ball_x - BALL_SIZE < left_paddle_x + PADDLE_WIDTH &&
        game->ball_x + BALL_SIZE > left_paddle_x - PADDLE_WIDTH &&
        game->ball_y < game->left_paddle_y + PADDLE_HEIGHT &&
        game->ball_y > game->left_paddle_y - PADDLE_HEIGHT) {
        game->ball_vx = fabsf(game->ball_vx);
        // Add some spin based on where it hit the paddle
        float hit_pos = (game->ball_y - game->left_paddle_y) / PADDLE_HEIGHT;
        game->ball_vy += hit_pos * 0.5f;
    }
    
    // Right paddle collision
    if (game->ball_x + BALL_SIZE > right_paddle_x - PADDLE_WIDTH &&
        game->ball_x - BALL_SIZE < right_paddle_x + PADDLE_WIDTH &&
        game->ball_y < game->right_paddle_y + PADDLE_HEIGHT &&
        game->ball_y > game->right_paddle_y - PADDLE_HEIGHT) {
        game->ball_vx = -fabsf(game->ball_vx);
        float hit_pos = (game->ball_y - game->right_paddle_y) / PADDLE_HEIGHT;
        game->ball_vy += hit_pos * 0.5f;
    }
    
    // Ball out of bounds - score
    if (game->ball_x < -1.0f) {
        game->right_score++;
        printf("Score: %d - %d\n", game->left_score, game->right_score);
        reset_ball(game);
    }
    if (game->ball_x > 1.0f) {
        game->left_score++;
        printf("Score: %d - %d\n", game->left_score, game->right_score);
        reset_ball(game);
    }
}

// Render callback
void render(UGContext* context, UGRenderFrame* frame, void* userdata) {
    GameState* game = (GameState*)userdata;
    (void)context;

    // Calculate delta time
    double current_time = ug_get_time();
    float dt = (float)(current_time - game->last_time);
    game->last_time = current_time;

    // Update game logic
    update_game(game, dt);

    // Build vertex data
    Vertex vertices[MAX_VERTICES];
    size_t vertex_count = 0;

    // Draw center line (dashed)
    for (int i = 0; i < 20; i++) {
        float y = -1.0f + (i * 0.1f);
        add_rect(vertices, &vertex_count, 0.0f, y, 0.005f, 0.04f, 0.5f, 0.5f, 0.5f);
    }

    // Draw scores at the top
    draw_number(vertices, &vertex_count, game->left_score, -0.3f, 0.75f, 0.15f, 0.8f, 0.8f, 0.8f);
    draw_number(vertices, &vertex_count, game->right_score, 0.3f, 0.75f, 0.15f, 0.8f, 0.8f, 0.8f);

    // Draw paddles
    add_rect(vertices, &vertex_count, -0.95f, game->left_paddle_y, PADDLE_WIDTH, PADDLE_HEIGHT, 1.0f, 1.0f, 1.0f);
    add_rect(vertices, &vertex_count, 0.95f, game->right_paddle_y, PADDLE_WIDTH, PADDLE_HEIGHT, 1.0f, 1.0f, 1.0f);

    // Draw ball
    add_rect(vertices, &vertex_count, game->ball_x, game->ball_y, BALL_SIZE, BALL_SIZE, 1.0f, 1.0f, 0.0f);

    // Update vertex buffer (simplified!)
    ug_vertex_buffer_update(game->vertex_buffer, vertices, vertex_count);
    game->vertex_count = vertex_count;

    // Render (simplified - no WebGPU boilerplate!)
    UGRenderPass* pass = ug_render_pass_begin(frame, 0.0f, 0.0f, 0.0f, 1.0f);
    ug_render_pass_set_pipeline(pass, game->pipeline);
    ug_render_pass_set_vertex_buffer(pass, game->vertex_buffer);
    ug_render_pass_draw(pass, game->vertex_count);
    ug_render_pass_end(pass);
}

int main(void) {
    // Create window and context
    UGWindow* window = ug_window_create("Pong", 800, 600);
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

    // Create vertex buffer (simplified!)
    UGVertexBuffer* vertex_buffer = ug_vertex_buffer_create(context, sizeof(Vertex), MAX_VERTICES);

    // Define vertex layout (simplified!)
    UGVertexAttribute vertex_attributes[2] = {
        {.format = WGPUVertexFormat_Float32x2, .offset = 0, .shader_location = 0},
        {.format = WGPUVertexFormat_Float32x3, .offset = 2 * sizeof(float), .shader_location = 1},
    };
    ug_vertex_buffer_set_layout(vertex_buffer, vertex_attributes, 2);

    // Build pipeline (simplified!)
    UGPipelineBuilder* pipeline_builder = ug_pipeline_builder_create(context, "examples/pong/pong.wgsl");
    if (!pipeline_builder) {
        fprintf(stderr, "Failed to create pipeline builder\n");
        return 1;
    }

    ug_pipeline_builder_set_vertex_buffer(pipeline_builder, ug_vertex_buffer_get_layout(vertex_buffer));
    WGPURenderPipeline pipeline = ug_pipeline_builder_build(pipeline_builder);

    // Initialize game state
    GameState game_state;
    init_game(&game_state);
    game_state.pipeline = pipeline;
    game_state.vertex_buffer = vertex_buffer;

    printf("Pong Game!\n");
    printf("Left player: W/S keys\n");
    printf("Right player: I/K keys\n");
    printf("Press ESC to exit.\n\n");

    // Set up input callback
    ug_window_set_key_callback(window, on_key_event, &game_state);

    // Run
    ug_run(context, render, &game_state);

    // Cleanup (simplified!)
    ug_vertex_buffer_destroy(vertex_buffer);
    wgpuRenderPipelineRelease(pipeline);
    ug_pipeline_builder_destroy(pipeline_builder);
    ug_context_destroy(context);
    ug_window_destroy(window);

    return 0;
}


