# Sprite Sheet Animation Demo

This example demonstrates the sprite sheet animation system in the Ungrund engine.

## Features

- Load sprite sheets from image files
- Animate sprites by cycling through frames
- Multiple animated sprites on screen
- Simple API for game developers

## Setup

### 1. Create a Sprite Sheet

You need a sprite sheet image at `examples/sprite_demo/spritesheet.png`.

The sprite sheet should be organized in a grid:
- Sprites are arranged left-to-right, top-to-bottom
- All sprites must be the same size
- The example expects 32x32 pixel sprites

#### Use Your Own Sprite Sheet

Place your sprite sheet at `examples/sprite_demo/spritesheet.png` and update the sprite size in `main.c`:

```c
// Change this line to match your sprite size:
UGSpriteSheet* sprite_sheet = ug_sprite_sheet_create(texture, 32, 32);
```

### 2. Build and Run

```bash
make sprite_demo
./build/sprite_demo
```

Or use the run target:

```bash
make run-sprite
```

## API Usage

### Loading a Texture

```c
UGTexture* texture = ug_texture_create_from_file(context, "path/to/image.png");
```

Supports PNG, JPG, BMP, TGA, and other formats via stb_image.

### Creating a Sprite Sheet

```c
// Create sprite sheet with 32x32 pixel sprites
UGSpriteSheet* sprite_sheet = ug_sprite_sheet_create(texture, 32, 32);

// Get info about the sprite sheet
int total_sprites = ug_sprite_sheet_get_sprite_count(sprite_sheet);
```

### Rendering Sprites

```c
UGVertex2DTextured vertices[MAX_VERTICES];
size_t vertex_count = 0;

// Add a sprite at position (x, y) with size (w, h)
// sprite_index: which sprite to draw (0-based, left-to-right, top-to-bottom)
ug_sprite_sheet_add_sprite(sprite_sheet, vertices, &vertex_count,
                          sprite_index, x, y, w, h);

// Update vertex buffer and render
ug_vertex_buffer_update(vertex_buffer, vertices, vertex_count);
```

### Animation Example

```c
typedef struct {
    float animation_time;
    int current_frame;
    int frame_count;
    float frame_duration;
} Animation;

void update_animation(Animation* anim, float delta_time) {
    anim->animation_time += delta_time;
    if (anim->animation_time >= anim->frame_duration) {
        anim->current_frame = (anim->current_frame + 1) % anim->frame_count;
        anim->animation_time = 0.0f;
    }
}

// In render loop:
update_animation(&anim, delta_time);
ug_sprite_sheet_add_sprite(sprite_sheet, vertices, &vertex_count,
                          anim.current_frame, x, y, w, h);
```

## What the Demo Shows

1. **Animated Sprites**: Two sprites moving horizontally with frame animation
2. **Static Sprite**: A larger sprite showing a single frame
3. **Frame Strip**: All frames displayed in a row at the bottom

## Cleanup

```c
ug_sprite_sheet_destroy(sprite_sheet);
ug_texture_destroy(texture);
```

Note: The sprite sheet doesn't own the texture, so you must destroy both separately.

