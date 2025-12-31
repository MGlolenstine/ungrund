# Pong Game Example

A classic Pong game implementation using the Ungrund graphics library.

## Features

- **Two-player gameplay** with keyboard controls
- **7-segment style score display** rendered at the top of the screen
- **Physics-based ball movement** with paddle collision detection
- **Smooth paddle controls** with continuous movement
- **Visual feedback** with colored elements

## Controls

- **Left Player (Left Paddle):**
  - `W` - Move up
  - `S` - Move down

- **Right Player (Right Paddle):**
  - `I` - Move up
  - `K` - Move down

- **ESC** - Exit game

## Building and Running

```bash
# Build the pong example
make pong

# Run the game
make run-pong
# or
./build/pong
```

## Implementation Details

### Score Display

The game features a custom 7-segment display style score renderer that draws digits using simple rectangles. This approach:

- **No font loading required** - Uses geometric shapes only
- **Retro aesthetic** - Classic 7-segment display look
- **Efficient rendering** - Batched with other game geometry
- **Supports 0-99** - Automatically handles single and double-digit scores

The score is displayed at the top of the screen:
- Left player score on the left side
- Right player score on the right side

### Rendering Functions

```c
// Draw a single digit (0-9) in 7-segment style
void draw_digit(Vertex* vertices, size_t* count, int digit, 
                float x, float y, float size, 
                float r, float g, float b);

// Draw a number (0-99) with automatic digit positioning
void draw_number(Vertex* vertices, size_t* count, int number,
                 float x, float y, float size,
                 float r, float g, float b);
```

### Game Elements

- **Paddles:** White rectangles on left and right sides
- **Ball:** Yellow square that bounces between paddles
- **Center Line:** Dashed gray line dividing the court
- **Score Display:** Light gray 7-segment digits at the top

### Physics

- Ball bounces off top and bottom walls
- Ball bounces off paddles with spin based on hit position
- Paddle movement is clamped to screen boundaries
- Score increments when ball goes past a paddle

## Code Structure

- **Game State:** Tracks paddle positions, ball position/velocity, scores, and input state
- **Update Loop:** Handles input, physics, and collision detection
- **Render Loop:** Builds vertex data and renders all game elements
- **Input Callbacks:** Processes keyboard events for smooth controls

## Technical Notes

- Uses the Ungrund vertex buffer system for efficient rendering
- All game elements are rendered in a single draw call
- Callback-based input system for responsive controls
- Delta time-based movement for frame-rate independent physics
- Simple vertex format: position (2 floats) + color (3 floats)

## Shader

The game uses a simple vertex/fragment shader (`pong.wgsl`) that:
- Transforms vertex positions to clip space
- Passes through vertex colors
- Renders solid colored geometry

## Future Enhancements

Possible improvements:
- Sound effects for ball hits and scoring
- Particle effects on collisions
- AI opponent for single-player mode
- Power-ups and special abilities
- Difficulty levels with varying ball speeds
- Win condition (first to X points)
- Pause/resume functionality

