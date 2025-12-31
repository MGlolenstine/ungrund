#include "ungrund.h"
#include <math.h>

// Add a rectangle to a vertex array (2D position + color format)
void ug_add_rect_2d_color(UGVertex2DColor* vertices, size_t* count,
                          float x, float y, float w, float h,
                          float r, float g, float b) {
    if (!vertices || !count) {
        return;
    }

    // Two triangles to form a rectangle
    // Triangle 1
    vertices[(*count)++] = (UGVertex2DColor){{x - w, y - h}, {r, g, b}};
    vertices[(*count)++] = (UGVertex2DColor){{x + w, y - h}, {r, g, b}};
    vertices[(*count)++] = (UGVertex2DColor){{x - w, y + h}, {r, g, b}};
    
    // Triangle 2
    vertices[(*count)++] = (UGVertex2DColor){{x - w, y + h}, {r, g, b}};
    vertices[(*count)++] = (UGVertex2DColor){{x + w, y - h}, {r, g, b}};
    vertices[(*count)++] = (UGVertex2DColor){{x + w, y + h}, {r, g, b}};
}

// Add a circle (or ellipse) to a vertex array (2D position + color format)
void ug_add_circle_2d_color(UGVertex2DColor* vertices, size_t* count,
                            float x, float y, float width, float height,
                            float r, float g, float b, int segments) {
    if (!vertices || !count || segments < 3) {
        return;
    }

    // If height is 0, use width for both (perfect circle)
    float radius_x = width;
    float radius_y = (height == 0.0f) ? width : height;

    // Generate circle/ellipse as a triangle fan
    // Center vertex is at (x, y)
    
    const float TWO_PI = 6.28318530718f;
    float angle_step = TWO_PI / segments;

    for (int i = 0; i < segments; i++) {
        float angle1 = i * angle_step;
        float angle2 = (i + 1) * angle_step;

        // Center point
        float cx = x;
        float cy = y;

        // First edge point
        float x1 = x + cosf(angle1) * radius_x;
        float y1 = y + sinf(angle1) * radius_y;

        // Second edge point
        float x2 = x + cosf(angle2) * radius_x;
        float y2 = y + sinf(angle2) * radius_y;

        // Create triangle
        vertices[(*count)++] = (UGVertex2DColor){{cx, cy}, {r, g, b}};
        vertices[(*count)++] = (UGVertex2DColor){{x1, y1}, {r, g, b}};
        vertices[(*count)++] = (UGVertex2DColor){{x2, y2}, {r, g, b}};
    }
}

// Add a rectangle to a vertex array (2D position + UV format)
void ug_add_rect_2d_textured(UGVertex2DTextured* vertices, size_t* count,
                             float x, float y, float w, float h,
                             float u0, float v0, float u1, float v1) {
    if (!vertices || !count) {
        return;
    }

    // Two triangles to form a rectangle
    // Triangle 1
    vertices[(*count)++] = (UGVertex2DTextured){{x - w, y - h}, {u0, v0}};
    vertices[(*count)++] = (UGVertex2DTextured){{x + w, y - h}, {u1, v0}};
    vertices[(*count)++] = (UGVertex2DTextured){{x - w, y + h}, {u0, v1}};
    
    // Triangle 2
    vertices[(*count)++] = (UGVertex2DTextured){{x - w, y + h}, {u0, v1}};
    vertices[(*count)++] = (UGVertex2DTextured){{x + w, y - h}, {u1, v0}};
    vertices[(*count)++] = (UGVertex2DTextured){{x + w, y + h}, {u1, v1}};
}

// Add a circle (or ellipse) to a vertex array (2D position + UV format)
void ug_add_circle_2d_textured(UGVertex2DTextured* vertices, size_t* count,
                               float x, float y, float width, float height,
                               float u0, float v0, float u1, float v1,
                               int segments) {
    if (!vertices || !count || segments < 3) {
        return;
    }

    // If height is 0, use width for both (perfect circle)
    float radius_x = width;
    float radius_y = (height == 0.0f) ? width : height;

    // UV center and radius
    float u_center = (u0 + u1) * 0.5f;
    float v_center = (v0 + v1) * 0.5f;
    float u_radius = (u1 - u0) * 0.5f;
    float v_radius = (v1 - v0) * 0.5f;

    const float TWO_PI = 6.28318530718f;
    float angle_step = TWO_PI / segments;

    for (int i = 0; i < segments; i++) {
        float angle1 = i * angle_step;
        float angle2 = (i + 1) * angle_step;

        // Center point
        float cx = x;
        float cy = y;
        float cu = u_center;
        float cv = v_center;

        // First edge point
        float x1 = x + cosf(angle1) * radius_x;
        float y1 = y + sinf(angle1) * radius_y;
        float u1_coord = u_center + cosf(angle1) * u_radius;
        float v1_coord = v_center + sinf(angle1) * v_radius;

        // Second edge point
        float x2 = x + cosf(angle2) * radius_x;
        float y2 = y + sinf(angle2) * radius_y;
        float u2 = u_center + cosf(angle2) * u_radius;
        float v2 = v_center + sinf(angle2) * v_radius;

        // Create triangle
        vertices[(*count)++] = (UGVertex2DTextured){{cx, cy}, {cu, cv}};
        vertices[(*count)++] = (UGVertex2DTextured){{x1, y1}, {u1_coord, v1_coord}};
        vertices[(*count)++] = (UGVertex2DTextured){{x2, y2}, {u2, v2}};
    }
}

