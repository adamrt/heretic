@ctype mat4 mat4s
@ctype mat3 mat3s
@ctype vec4 vec4s
@ctype vec3 vec3s

@vs standard_vs

// This is necessary because the Y-axis is flipped in OpenGL.
@glsl_options flip_vert_y
// This is necessary because the clip space is different in Metal.
@msl_options fixup_clipspace

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

void main() {
    vec3 norm = normalize(v_normal);
    vec4 diffuse_light = vec4(0.0, 0.0, 0.0, 1.0);
    for (int i = 0; i < u_light_count; i++) {
        vec3 direction = normalize(u_light_directions[i].xyz);
        float intensity = clamp(dot(norm, direction), 0.0, 1.0);
        diffuse_light += u_light_colors[i] * intensity;
    }

    vec4 light = u_ambient_color * u_ambient_strength + diffuse_light;

   // Handle untextured triangles
    if (v_is_textured < 0.5) { // Assuming a_is_textured is 1.0 for textured and 0.0 for untextured
        frag_color = light * vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    // Get the index color from the palette
    vec4 tex_color = texture(sampler2D(u_texture, u_sampler), v_uv);
    // Scale the index color from 0.0-1.0 to 0-255
    float palette_x = float(uint(tex_color.r * 255.0));
    float palette_y = float(uint(v_palette_index));

    // Scale the x and y back down to 0.0-1.0; There are 16x16 colors in the palette.
    vec2 uv = vec2(palette_x / 16, palette_y / 16);

    vec4 color = texture(sampler2D(u_palette, u_sampler), uv);
    if (color.a < 0.5)
        discard;

    frag_color = color * light;
}
@end


@vs background_vs

// This is necessary because the Y-axis is flipped in OpenGL.
@glsl_options flip_vert_y
// This is necessary because the clip space is different in Metal.
@msl_options fixup_clipspace

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

void main() {
    frag_color = mix(u_bottom_color, u_top_color, v_uv.y);
}
@end

@vs display_vs
in vec3 a_position;
in vec2 a_uv;

out vec2 v_uv;

void main() {
    gl_Position = vec4(a_position.xy, 0.0, 1.0);
    v_uv = a_uv;
}
@end

@fs display_fs
layout(binding=0) uniform texture2D u_texture;
layout(binding=0) uniform sampler u_sampler;

in vec2 v_uv;

out vec4 frag_color;

void main() {
    frag_color = texture(sampler2D(u_texture, u_sampler), v_uv);
}
@end

@program standard   standard_vs   standard_fs
@program background background_vs background_fs
@program display    display_vs    display_fs
