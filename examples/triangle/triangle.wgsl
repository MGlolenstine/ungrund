// Uniform buffer for rotation
struct Uniforms {
    rotation: f32,
};

@group(0) @binding(0) var<uniform> uniforms: Uniforms;

// Vertex shader
struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec3<f32>,
};

// 2D rotation matrix helper
fn rotate2D(pos: vec2<f32>, angle: f32) -> vec2<f32> {
    let c = cos(angle);
    let s = sin(angle);
    return vec2<f32>(
        pos.x * c - pos.y * s,
        pos.x * s + pos.y * c
    );
}

@vertex
fn vs_main(@builtin(vertex_index) vertex_index: u32) -> VertexOutput {
    var out: VertexOutput;

    // Triangle vertices in NDC space
    var positions = array<vec2<f32>, 3>(
        vec2<f32>(0.0, 0.5),    // Top
        vec2<f32>(-0.5, -0.5),  // Bottom left
        vec2<f32>(0.5, -0.5)    // Bottom right
    );

    // RGB colors for each vertex
    var colors = array<vec3<f32>, 3>(
        vec3<f32>(1.0, 0.0, 0.0),  // Red
        vec3<f32>(0.0, 1.0, 0.0),  // Green
        vec3<f32>(0.0, 0.0, 1.0)   // Blue
    );

    // Apply rotation to the position
    let rotated_pos = rotate2D(positions[vertex_index], uniforms.rotation);
    out.position = vec4<f32>(rotated_pos, 0.0, 1.0);
    out.color = colors[vertex_index];

    return out;
}

// Fragment shader
@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
    return vec4<f32>(in.color, 1.0);
}

