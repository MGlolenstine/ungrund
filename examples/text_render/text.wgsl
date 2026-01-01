// Ubershader for text rendering with multiple rendering modes
// Supports: standard text, outlined text, shadow, and color tinting

// Uniform buffer for rendering parameters
struct RenderUniforms {
    text_color: vec4<f32>,      // RGBA color for text
    outline_color: vec4<f32>,   // RGBA color for outline
    render_mode: u32,           // 0=standard, 1=outline, 2=shadow, 3=glow
    outline_width: f32,         // Width of outline in pixels
    shadow_offset: vec2<f32>,   // Shadow offset in texture space
};

@group(0) @binding(0) var font_texture: texture_2d<f32>;
@group(0) @binding(1) var font_sampler: sampler;
@group(0) @binding(2) var<uniform> uniforms: RenderUniforms;

// Vertex shader for text rendering
struct VertexInput {
    @location(0) position: vec2<f32>,
    @location(1) texcoord: vec2<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) texcoord: vec2<f32>,
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = vec4<f32>(in.position, 0.0, 1.0);
    out.texcoord = in.texcoord;
    return out;
}

// Fragment shader with multiple rendering modes
@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
    let alpha = textureSample(font_texture, font_sampler, in.texcoord).r;

    // Mode 0: Standard text rendering
    if (uniforms.render_mode == 0u) {
        return vec4<f32>(uniforms.text_color.rgb, uniforms.text_color.a * alpha);
    }

    // Mode 1: Outlined text
    else if (uniforms.render_mode == 1u) {
        // Sample surrounding pixels for outline detection
        let texel_size = 1.0 / vec2<f32>(textureDimensions(font_texture));
        let offset = texel_size * uniforms.outline_width;

        var max_alpha = alpha;
        max_alpha = max(max_alpha, textureSample(font_texture, font_sampler, in.texcoord + vec2<f32>(-offset.x, 0.0)).r);
        max_alpha = max(max_alpha, textureSample(font_texture, font_sampler, in.texcoord + vec2<f32>(offset.x, 0.0)).r);
        max_alpha = max(max_alpha, textureSample(font_texture, font_sampler, in.texcoord + vec2<f32>(0.0, -offset.y)).r);
        max_alpha = max(max_alpha, textureSample(font_texture, font_sampler, in.texcoord + vec2<f32>(0.0, offset.y)).r);

        // If we're on the edge (outline area), use outline color, otherwise use text color
        let is_outline = max_alpha > alpha;
        if (is_outline) {
            return vec4<f32>(uniforms.outline_color.rgb, uniforms.outline_color.a * max_alpha);
        } else {
            return vec4<f32>(uniforms.text_color.rgb, uniforms.text_color.a * alpha);
        }
    }

    // Mode 2: Shadow effect
    else if (uniforms.render_mode == 2u) {
        let shadow_alpha = textureSample(font_texture, font_sampler, in.texcoord + uniforms.shadow_offset).r;

        // Blend shadow with text
        if (alpha > 0.1) {
            return vec4<f32>(uniforms.text_color.rgb, uniforms.text_color.a * alpha);
        } else if (shadow_alpha > 0.1) {
            return vec4<f32>(0.0, 0.0, 0.0, 0.5 * shadow_alpha);
        }
        return vec4<f32>(0.0, 0.0, 0.0, 0.0);
    }

    // Mode 3: Glow effect
    else if (uniforms.render_mode == 3u) {
        // Sample surrounding pixels for glow
        let texel_size = 1.0 / vec2<f32>(textureDimensions(font_texture));
        let glow_radius = uniforms.outline_width;

        var glow = 0.0;
        for (var x = -2.0; x <= 2.0; x += 1.0) {
            for (var y = -2.0; y <= 2.0; y += 1.0) {
                let offset = vec2<f32>(x, y) * texel_size * glow_radius;
                glow += textureSample(font_texture, font_sampler, in.texcoord + offset).r;
            }
        }
        glow /= 25.0; // Average of 5x5 samples

        // Combine glow with text
        let final_alpha = max(alpha, glow * 0.5);
        let glow_color = mix(uniforms.outline_color.rgb, uniforms.text_color.rgb, alpha / max(final_alpha, 0.001));
        return vec4<f32>(glow_color, final_alpha);
    }

    // Default fallback
    return vec4<f32>(uniforms.text_color.rgb, uniforms.text_color.a * alpha);
}

