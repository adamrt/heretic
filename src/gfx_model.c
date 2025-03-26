#include "cglm/struct/vec4.h"

#include "camera.h"
#include "gfx.h"
#include "gfx_model.h"
#include "lighting.h"

#include "shader.glsl.h"

static struct {
    sg_pipeline pipeline;
} _state;

void gfx_model_init(void) {
    _state.pipeline = sg_make_pipeline(&(sg_pipeline_desc) {
        .layout = {
            .attrs = {
                [ATTR_standard_a_position].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_standard_a_normal].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_standard_a_uv].format = SG_VERTEXFORMAT_FLOAT2,
                [ATTR_standard_a_palette_index].format = SG_VERTEXFORMAT_FLOAT,
                [ATTR_standard_a_is_textured].format = SG_VERTEXFORMAT_FLOAT,
            },
        },
        .shader = sg_make_shader(standard_shader_desc(sg_query_backend())),
        .face_winding = gfx_get_face_winding(),
        .cull_mode = SG_CULLMODE_BACK,
        .depth = {
            .pixel_format = SG_PIXELFORMAT_DEPTH,
            .compare = SG_COMPAREFUNC_LESS,
            .write_enabled = true,
        },
        .colors[0].pixel_format = SG_PIXELFORMAT_RGBA8,
        .label = "standard-pipeline",
    });
}

void gfx_model_shutdown(void) {
    sg_destroy_pipeline(_state.pipeline);
}

void gfx_model_render(const model_t* model) {
    mat4s model_mat = transform_to_matrix(model->transform);

    vs_standard_params_t vs_params = {
        .u_proj = camera_get_proj(),
        .u_view = camera_get_view(),
        .u_model = model_mat,
    };

    fs_standard_params_t fs_params;
    fs_params.u_ambient_color = model->lighting.ambient_color;
    fs_params.u_ambient_strength = model->lighting.ambient_strength;

    int light_count = 0;
    for (int i = 0; i < LIGHTING_MAX_LIGHTS; i++) {

        light_t light = model->lighting.lights[i];
        if (!light.valid) {
            continue;
        }

        fs_params.u_light_colors[light_count] = light.color;
        fs_params.u_light_directions[light_count] = glms_vec4(light.direction, 1.0f);
        light_count++;
    }
    fs_params.u_light_count = light_count;

    sg_bindings bindings = {
        .vertex_buffers[0] = model->vbuf,
        .index_buffer = model->ibuf,
        .samplers[SMP_u_sampler] = gfx_get_sampler(),
        .images = {
            [IMG_u_texture] = model->texture.gpu_image,
            [IMG_u_palette] = model->palette.gpu_image,
        },
    };

    sg_apply_pipeline(_state.pipeline);
    sg_apply_bindings(&bindings);
    sg_apply_uniforms(0, &SG_RANGE(vs_params));
    sg_apply_uniforms(1, &SG_RANGE(fs_params));
    sg_draw(0, model->vertex_count, 1);
}

void gfx_model_destroy(model_t model) {
    sg_destroy_buffer(model.vbuf);
    sg_destroy_buffer(model.ibuf);

    texture_destroy(model.texture);
    texture_destroy(model.palette);
}
