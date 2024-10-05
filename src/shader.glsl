@ctype mat4 mat4s

@vs cube_vs
uniform vs_params {
    mat4 u_mvp;
};

in vec3 a_position;
in vec4 a_color;

out vec4 v_color;

void main() {
    gl_Position = u_mvp * vec4(a_position, 1.0);
    v_color = a_color;
}
@end

@fs cube_fs
in vec4 v_color;

out vec4 frag_color;

void main() {
    frag_color = v_color;
}
@end

@vs quad_vs
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
