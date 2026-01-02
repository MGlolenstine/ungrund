#include "ungrund.h"
#include <stdlib.h>
#include <stdio.h>

struct UGSpriteSheet {
    UGTexture* texture;
    int sprite_width;
    int sprite_height;
    int sprites_per_row;
    int sprites_per_column;
    int total_sprites;
    int texture_width;
    int texture_height;
};

UGSpriteSheet* ug_sprite_sheet_create(UGTexture* texture, int sprite_width, int sprite_height) {
    if (!texture || sprite_width <= 0 || sprite_height <= 0) {
        return NULL;
    }
    
    UGSpriteSheet* sheet = (UGSpriteSheet*)calloc(1, sizeof(UGSpriteSheet));
    if (!sheet) {
        return NULL;
    }
    
    sheet->texture = texture;
    sheet->sprite_width = sprite_width;
    sheet->sprite_height = sprite_height;
    
    // Get texture dimensions
    ug_texture_get_size(texture, &sheet->texture_width, &sheet->texture_height);
    
    // Calculate grid layout
    sheet->sprites_per_row = sheet->texture_width / sprite_width;
    sheet->sprites_per_column = sheet->texture_height / sprite_height;
    sheet->total_sprites = sheet->sprites_per_row * sheet->sprites_per_column;
    
    return sheet;
}

void ug_sprite_sheet_destroy(UGSpriteSheet* sheet) {
    if (sheet) {
        // Note: We don't destroy the texture here - the user owns it
        free(sheet);
    }
}

void ug_sprite_sheet_add_sprite(UGSpriteSheet* sheet, UGVertex2DTextured* vertices, size_t* count,
                                int sprite_index, float x, float y, float w, float h) {
    if (!sheet || !vertices || !count) {
        return;
    }
    
    // Clamp sprite index to valid range
    if (sprite_index < 0) sprite_index = 0;
    if (sprite_index >= sheet->total_sprites) sprite_index = sheet->total_sprites - 1;
    
    // Calculate sprite position in the sheet
    int sprite_x = sprite_index % sheet->sprites_per_row;
    int sprite_y = sprite_index / sheet->sprites_per_row;
    
    // Calculate UV coordinates
    float u0 = (float)(sprite_x * sheet->sprite_width) / (float)sheet->texture_width;
    float v0 = (float)(sprite_y * sheet->sprite_height) / (float)sheet->texture_height;
    float u1 = (float)((sprite_x + 1) * sheet->sprite_width) / (float)sheet->texture_width;
    float v1 = (float)((sprite_y + 1) * sheet->sprite_height) / (float)sheet->texture_height;
    
    // Use the existing geometry helper to add the textured rectangle
    ug_add_rect_2d_textured(vertices, count, x, y, w, h, u0, v0, u1, v1);
}

UGTexture* ug_sprite_sheet_get_texture(UGSpriteSheet* sheet) {
    return sheet ? sheet->texture : NULL;
}

void ug_sprite_sheet_get_sprite_size(UGSpriteSheet* sheet, int* width, int* height) {
    if (sheet) {
        if (width) *width = sheet->sprite_width;
        if (height) *height = sheet->sprite_height;
    }
}

int ug_sprite_sheet_get_sprite_count(UGSpriteSheet* sheet) {
    return sheet ? sheet->total_sprites : 0;
}

