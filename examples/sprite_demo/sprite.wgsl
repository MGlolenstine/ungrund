// Vertex shader
struct VertexInput {
    @location(0) position: vec2f,
    @location(1) uv: vec2f,
}

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) uv: vec2f,
}

@vertex
fn vs_main(input: VertexInput) -> VertexOutput {
    var output: VertexOutput;
    output.position = vec4f(input.position, 0.0, 1.0);
    output.uv = input.uv;
    return output;
}

// Fragment shader
@group(0) @binding(0) var texture_data: texture_2d<f32>;
@group(0) @binding(1) var texture_sampler: sampler;

@fragment
fn fs_main(input: VertexOutput) -> @location(0) vec4f {
    return textureSample(texture_data, texture_sampler, input.uv);
}

