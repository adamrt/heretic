
#include "sokol_gfx.h"

#include "gfx.h"
#include "gfx_background.h"
#include "mesh.h"

#include "shader.glsl.h"

static struct {
    sg_pipeline pipeline;
    sg_bindings bindings;

    vec4s top_color;
    vec4s bottom_color;
} _state;

void gfx_background_set(vec4s top, vec4s bottom) {
    _state.top_color = top;
    _state.bottom_color = bottom;
}

void gfx_background_init(void) {
    _state.pipeline = sg_make_pipeline(&(sg_pipeline_desc) {
        .layout = {
            .buffers[0].stride = sizeof(vertex_t),
            .attrs = {
                [ATTR_background_a_position].format = SG_VERTEXFORMAT_FLOAT3,
            },
        },
        .shader = sg_make_shader(background_shader_desc(sg_query_backend())),
        .face_winding = gfx_get_face_winding(),
        .cull_mode = SG_CULLMODE_BACK,
        .depth = {
            // Disable write and compare so thebg doesn't affect the depth buffer.
            .pixel_format = SG_PIXELFORMAT_DEPTH,
            .compare = SG_COMPAREFUNC_ALWAYS,
            .write_enabled = false,
        },
        .colors[0].pixel_format = SG_PIXELFORMAT_RGBA8,
        .label = "background-pipeline",
    });

    _state.bindings = (sg_bindings) {
        .vertex_buffers[0] = gfx_get_quad_vbuf(),
    };
}

void gfx_background_shutdown(void) {
    sg_destroy_pipeline(_state.pipeline);
}

void gfx_background_render(void) {
    sg_apply_pipeline(_state.pipeline);

    fs_background_params_t fs_params;
    fs_params.u_top_color = _state.top_color;
    fs_params.u_bottom_color = _state.bottom_color;

    sg_apply_pipeline(_state.pipeline);
    sg_apply_bindings(&_state.bindings);
    sg_apply_uniforms(0, &SG_RANGE(fs_params));
    sg_draw(0, 6, 1);
}
