@ctype mat4 mat4s

@vs cube_vs
uniform vs_params {
    mat4 mvp;
};

in vec4 position;
in vec4 color0;

out vec4 color;

void main() {
    gl_Position = mvp * position;
    color = color0;
}
@end

@fs cube_fs
in vec4 color;

out vec4 frag_color;

void main() {
    frag_color = color;
}
@end

@vs quad_vs
in vec2 position;
in vec2 texcoord0;

out vec2 fs_texcoord;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    fs_texcoord = texcoord0;
}
@end

@fs quad_fs
uniform texture2D tex;
uniform sampler smp;

in vec2 fs_texcoord;

out vec4 frag_color;

void main() {
    frag_color = texture(sampler2D(tex, smp), fs_texcoord);
}
@end

@program cube cube_vs cube_fs
@program quad quad_vs quad_fs
