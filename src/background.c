
#include "gfx.h"
#include "mesh.h"
#include "sokol_gfx.h"

#include "background.h"
#include "shader.glsl.h"

static struct {
    sg_pipeline pipeline;
    sg_bindings bindings;
} _state;

void background_init(void) {
    _state.pipeline = sg_make_pipeline(&(sg_pipeline_desc) {
        .layout = {
            .buffers[0].stride = sizeof(vertex_t),
            .attrs = {
                [ATTR_background_a_position].format = SG_VERTEXFORMAT_FLOAT3,
            },
        },
        .shader = sg_make_shader(background_shader_desc(sg_query_backend())),
        .index_type = SG_INDEXTYPE_UINT16,
        .cull_mode = SG_CULLMODE_NONE,
        .depth = {
            .pixel_format = SG_PIXELFORMAT_DEPTH,
            // disable write and compre so the bg doesn't affect to the depth buffer.
            // .compare = SG_COMPAREFUNC_LESS,
            // .write_enabled = true,
        },
        .colors[0].pixel_format = SG_PIXELFORMAT_RGBA8,
        .label = "background-pipeline",
    });

    _state.bindings = (sg_bindings) {
        .vertex_buffers[0] = gfx_get_quad_vbuf(),
        .index_buffer = gfx_get_quad_ibuf(),
    };
}

void background_shutdown(void) {
    sg_destroy_pipeline(_state.pipeline);
}

void background_render(vec4s top_color, vec4s bottom_color) {
    sg_apply_pipeline(_state.pipeline);

    fs_background_params_t fs_params;
    fs_params.u_top_color = top_color;
    fs_params.u_bottom_color = bottom_color;

    sg_apply_pipeline(_state.pipeline);
    sg_apply_bindings(&_state.bindings);
    sg_apply_uniforms(0, &SG_RANGE(fs_params));
    sg_draw(0, 6, 1);
}
