#include "sokol_gfx.h"

#include "camera.h"
#include "color.h"
#include "gfx_line.h"

#include "shader.glsl.h"

static struct {
    sg_pipeline pipeline;
    sg_bindings bindings;
    sg_buffer vbuf;
} _state;

// 28 represents the width and depth of a tile.
static constexpr f32 dim = 28.0f * 3.0f; // Use constants
static const vec3s axis_verts[] = {
    { { 0, 0, 0 } }, { { dim, 0, 0 } }, // X
    { { 0, 0, 0 } }, { { 0, dim, 0 } }, // Y
    { { 0, 0, 0 } }, { { 0, 0, dim } }, // Z
};

void gfx_line_init(void) {
    _state.pipeline = sg_make_pipeline(&(sg_pipeline_desc) {
        .shader = sg_make_shader(line_shader_desc(sg_query_backend())),
        .layout = {
            .attrs = {
                [0].format = SG_VERTEXFORMAT_FLOAT3,
            },
        },
        .primitive_type = SG_PRIMITIVETYPE_LINES,
        .depth = {
            .pixel_format = SG_PIXELFORMAT_DEPTH,
            .compare = SG_COMPAREFUNC_GREATER,
            .write_enabled = false,
        },
        .colors[0].pixel_format = SG_PIXELFORMAT_RGBA8,
        .label = "line-pipeline",
    });

    _state.vbuf = sg_make_buffer(&(sg_buffer_desc) {
        .data.ptr = axis_verts,
        .data.size = 6 * sizeof(vec3s),
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .label = "line-axis-vbuf",
    });

    _state.bindings = (sg_bindings) {
        .vertex_buffers[0] = _state.vbuf,
    };
}

void gfx_line_shutdown(void) {
    sg_destroy_pipeline(_state.pipeline);
    sg_destroy_buffer(_state.vbuf);
}

void gfx_line_render_axis(void) {
    vs_line_params_t vs_params = {
        .u_proj = camera_get_proj(),
        .u_view = camera_get_view(),
    };

    sg_apply_pipeline(_state.pipeline);
    sg_apply_bindings(&_state.bindings);

    sg_apply_uniforms(0, &SG_RANGE(vs_params));

    sg_apply_uniforms(1, &SG_RANGE(COLOR_RED));
    sg_draw(0, 2, 1);
    sg_apply_uniforms(1, &SG_RANGE(COLOR_GREEN));
    sg_draw(2, 2, 1);
    sg_apply_uniforms(1, &SG_RANGE(COLOR_BLUE));
    sg_draw(4, 2, 1);
}
