@ctype mat4 mat4s
@ctype mat3 mat3s
@ctype vec4 vec4s
@ctype vec3 vec3s

@vs cube_vs
uniform vs_cube_params {
    mat4 u_proj;
    mat4 u_view;
    mat4 u_model;
};

in vec3 a_position;
in vec3 a_normal;
in vec4 a_color;

out vec4 v_position;
out vec3 v_normal;
out vec4 v_color;

void main() {
    v_position = u_model * vec4(a_position, 1.0);

    mat3 normal_matrix = transpose(inverse(mat3(u_model)));
    v_normal = normalize(normal_matrix * a_normal);

    v_color = a_color;

    mat4 view_proj = u_proj * u_view;
    gl_Position = view_proj * v_position;
}
@end

@fs cube_fs
uniform fs_cube_params {
    vec3 u_light_position;
    vec4 u_light_color;
    vec4 u_ambient_color;
    float u_ambient_strength;
};

in vec4 v_position;
in vec3 v_normal;
in vec4 v_color;

out vec4 frag_color;

void main() {
    vec3 norm = normalize(v_normal);
    vec3 direction = normalize(u_light_position - v_position.xyz);
    float intensity = clamp(dot(norm, direction), 0.0, 1.0);

    vec4 ambient = u_ambient_color * u_ambient_strength;
    vec4 diffuse = u_light_color * intensity;
    vec4 light = ambient + diffuse;

    frag_color = v_color * light;
}
@end


@vs quad_vs

// flip_vert_y fixes the discrepancy between Y-axis orientation in APIs.
@glsl_options flip_vert_y

in vec3 a_position;
in vec2 a_uv;

out vec2 v_uv;

void main() {
    gl_Position = vec4(a_position.xy, 0.0, 1.0);
    v_uv = a_uv;
}
@end

@fs quad_fs
uniform texture2D tex;
uniform sampler smp;

in vec2 v_uv;

out vec4 frag_color;

void main() {
    frag_color = texture(sampler2D(tex, smp), v_uv);
}
@end

@program cube cube_vs cube_fs
@program quad quad_vs quad_fs
