#include "sokol_gfx.h"

#include "camera.h"
#include "color.h"
#include "line.h"

#include "shader.glsl.h"

static struct {
    sg_pipeline pipeline;

    sg_buffer axis_x_vbuf;
    sg_buffer axis_y_vbuf;
    sg_buffer axis_z_vbuf;
} _state;

// 28 represents the width and depth of a tile.
constexpr f32 dim = 28.0f * 5.0f; // Use constants
constexpr vec3s zero = { { 0, 0, 0 } };
constexpr vec3s axis_x = { { dim, 0, 0 } };
constexpr vec3s axis_y = { { 0, dim, 0 } };
constexpr vec3s axis_z = { { 0, 0, -dim } };

vec3s verts_x[2] = { zero, axis_x };
vec3s verts_y[2] = { zero, axis_y };
vec3s verts_z[2] = { zero, axis_z };

void line_init(void) {
    _state.pipeline = sg_make_pipeline(&(sg_pipeline_desc) {
        .shader = sg_make_shader(line_shader_desc(sg_query_backend())),
        .layout = {
            .attrs = {
                [0].format = SG_VERTEXFORMAT_FLOAT3,
            },
        },
        .primitive_type = SG_PRIMITIVETYPE_LINES,
        .depth = {
            .compare = SG_COMPAREFUNC_LESS,
            .pixel_format = SG_PIXELFORMAT_DEPTH,
            .write_enabled = false,
        },
        .colors[0].pixel_format = SG_PIXELFORMAT_RGBA8,
        .label = "line-pipeline",
    });

    //
    // Create the vertex buffers for the axis lines
    // We create these now so we don't have to create/detroy every frame.
    //

    _state.axis_x_vbuf = sg_make_buffer(&(sg_buffer_desc) {
        .data.ptr = verts_x,
        .data.size = 2 * sizeof(vec3s),
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .label = "line-axis-x",
    });

    _state.axis_y_vbuf = sg_make_buffer(&(sg_buffer_desc) {
        .data.ptr = verts_y,
        .data.size = 2 * sizeof(vec3s),
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .label = "line-axis-y",
    });

    _state.axis_z_vbuf = sg_make_buffer(&(sg_buffer_desc) {
        .data.ptr = verts_z,
        .data.size = 2 * sizeof(vec3s),
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .label = "line-axis-z",
    });
}

void line_shutdown(void) {
    sg_destroy_pipeline(_state.pipeline);
    sg_destroy_buffer(_state.axis_x_vbuf);
    sg_destroy_buffer(_state.axis_y_vbuf);
    sg_destroy_buffer(_state.axis_z_vbuf);
}

void line_render(sg_buffer vbuf, vec4s color) {
    vs_line_params_t vs_params = {
        .u_proj = camera_get_proj(),
        .u_view = camera_get_view(),
    };

    sg_apply_pipeline(_state.pipeline);
    sg_apply_bindings(&(sg_bindings) { .vertex_buffers[0] = vbuf });
    sg_apply_uniforms(0, &SG_RANGE(vs_params));
    sg_apply_uniforms(1, &SG_RANGE(color));
    sg_draw(0, 2, 1);
}

void line_render_axis(void) {
    line_render(_state.axis_x_vbuf, COLOR_RED);
    line_render(_state.axis_y_vbuf, COLOR_GREEN);
    line_render(_state.axis_z_vbuf, COLOR_BLUE);
}
