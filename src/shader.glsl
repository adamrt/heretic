@ctype mat4 mat4s
@ctype mat3 mat3s
@ctype vec4 vec4s
@ctype vec3 vec3s
@ctype vec2 vec2s

//
// Standard - Used for rendering 3D models/maps.
//

@vs standard_vs

layout(binding=0) uniform vs_standard_params {
    mat4 u_proj;
    mat4 u_view;
    mat4 u_model;
};

in vec3 a_position;
in vec3 a_normal;
in vec2 a_uv;
in float a_palette_index;
in float a_is_textured;

out vec4 v_position;
out vec3 v_normal;
out vec2 v_uv;
out float v_palette_index;
out float v_is_textured;

void main() {
    v_position = u_model * vec4(a_position, 1.0);
    gl_Position = u_proj * u_view * v_position;

    mat3 normal_matrix = transpose(inverse(mat3(u_model)));
    v_normal = normalize(normal_matrix * a_normal);

    v_uv = a_uv;
    v_palette_index = a_palette_index;
    v_is_textured = a_is_textured;
}
@end

@fs standard_fs

layout(binding=1) uniform fs_standard_params {
    vec4  u_ambient_color;
    float u_ambient_strength;
    vec4  u_light_directions[10];
    vec4  u_light_colors[10];
    int   u_light_count;
};

layout(binding=0) uniform texture2D u_texture;
layout(binding=1) uniform texture2D u_palette;
layout(binding=0) uniform sampler u_sampler;

in vec4 v_position;
in vec3 v_normal;
in vec2 v_uv;
in float v_palette_index;
in float v_is_textured;

out vec4 frag_color;

// PS1 4x4 Bayer pattern
const float bayer[16] = float[16](
     0.0,  8.0,  2.0, 10.0,
    12.0,  4.0, 14.0,  6.0,
     3.0, 11.0,  1.0,  9.0,
    15.0,  7.0, 13.0,  5.0
);

vec4 applyDither(vec4 color) {
    ivec2 pixel = ivec2(gl_FragCoord.xy) & 3;  // 4x4 pattern
    int index = pixel.x + pixel.y * 4;
    float offset = bayer[index] / 256.0;       // scale to 0-1 range
    return color + vec4(offset);               // add to all components
}

vec4 applyLight(vec3 norm, vec4 color) {
    vec4 diffuse_light = vec4(0.0, 0.0, 0.0, 1.0);
    for (int i = 0; i < u_light_count; i++) {
        vec3 dir = normalize(u_light_directions[i].xyz);
        float intensity = clamp(dot(norm, dir), 0.0, 1.0);
        diffuse_light += u_light_colors[i] * intensity;
    }
    vec4 light = u_ambient_color * u_ambient_strength + diffuse_light;
    return light * color;
}

vec4 samplePalettedTexture(vec2 uv, float paletteIndex) {
    vec4 indexColor = texture(sampler2D(u_texture, u_sampler), uv);
    float palette_x = float(uint(indexColor.r * 255.0));
    float palette_y = float(uint(paletteIndex));
    vec2 pal_uv = vec2(palette_x / 16.0, palette_y / 16.0);
    return texture(sampler2D(u_palette, u_sampler), pal_uv);
}

vec4 quantizeTo555(vec4 color) {
    color.r = floor(color.r * 31.0) / 31.0;
    color.g = floor(color.g * 31.0) / 31.0;
    color.b = floor(color.b * 31.0) / 31.0;
    return color;
}

void main() {
   // Handle untextured triangles
    if (v_is_textured < 0.5) { // Assuming a_is_textured is 1.0 for textured and 0.0 for untextured
        frag_color = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    vec4 color = samplePalettedTexture(v_uv, v_palette_index);
    if (color.a < 0.5) discard;

    vec3 norm = normalize(v_normal);
    color = applyLight(norm, color);
    color = quantizeTo555(color);     // optional PS1-style quantization
    color = applyDither(color);

    frag_color = color;
}
@end

//
// Background - used for render the gradient backgorund
//

@vs background_vs

in vec3 a_position;

out vec2 v_uv;

void main() {
    gl_Position = vec4(a_position, 1.0);
    v_uv = a_position.xy;
}
@end

@fs background_fs
layout(binding=0) uniform fs_background_params {
    vec4 u_top_color;
    vec4 u_bottom_color;
};

in vec2 v_uv;

out vec4 frag_color;

// PS1 4x4 Bayer pattern
const float bayer[16] = float[16](
     0.0,  8.0,  2.0, 10.0,
    12.0,  4.0, 14.0,  6.0,
     3.0, 11.0,  1.0,  9.0,
    15.0,  7.0, 13.0,  5.0
);


vec4 applyDither(vec4 color) {
    ivec2 pixel = ivec2(gl_FragCoord.xy) & 3;  // 4x4 pattern
    int index = pixel.x + pixel.y * 4;
    float offset = bayer[index] / 256.0;       // scale to 0-1 range
    return color + vec4(offset);               // add to all components
}

void main() {
    vec4 color = mix(u_top_color, u_bottom_color, v_uv.y);
    color = applyDither(color);
    frag_color = color;

}
@end

//
// Lines - used for rendering lines
//

@vs line_vs

layout(binding=0) uniform vs_line_params {
    mat4 u_proj;
    mat4 u_view;
};

in vec3 a_position;

void main() {
    vec4 v_position = vec4(a_position, 1.0);
    gl_Position = u_proj * u_view * v_position;
}
@end

@fs line_fs
layout(binding=1) uniform fs_line_params {
    vec4 u_color;
};

out vec4 frag_color;

void main() {
    frag_color = u_color;
}
@end

//
// Sprite
//

@vs sprite_vs

layout(binding=0) uniform vs_sprite_params {
    mat4 u_proj;
    mat4 u_view;
    mat4 u_model;
    vec2 u_uv_min;
    vec2 u_uv_max;
};

in vec3 a_position;
in vec2 a_uv;

out vec2 v_uv;

void main() {
    // Simple 3D transform for sprites if you’re drawing in a 3D scene.
    gl_Position = u_proj * u_view * u_model * vec4(a_position, 1.0);
    v_uv = mix(u_uv_min, u_uv_max, a_uv);
}
@end

@fs sprite_fs
layout(binding=0) uniform texture2D  u_texture;
layout(binding=0) uniform sampler    u_sampler;

in vec2 v_uv;

out vec4 frag_color;

void main() {
    frag_color = texture(sampler2D(u_texture, u_sampler), v_uv);
    if (frag_color.a < 0.5)
        discard;

}
@end

@program standard   standard_vs   standard_fs
@program background background_vs background_fs
@program line       line_vs       line_fs
@program sprite     sprite_vs     sprite_fs
