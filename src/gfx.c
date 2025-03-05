#include "cglm/struct/vec4.h"

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#include "shader.glsl.h"

#include "camera.h"
#include "gfx.h"
#include "gui.h"
#include "lighting.h"
#include "model.h"
#include "shape.h"

// Global gfx state
static gfx_t _state;

// Forward declarations
static void _init(void);

// There are two passes so we can render the offscreen image to a fullscreen
// quad. The offscreen is rendered in a lower resolution and then upscaled to
// the window size to keep the pixelated look.
void gfx_init(void) {
    sg_setup(&(sg_desc) {
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });

    _init();
}

void gfx_render_begin(void) {
    // Render the scene and background to an offscreen image
    sg_begin_pass(&(sg_pass) {
        .attachments = _state.attachments,
        .action = {
            .colors[0] = (sg_color_attachment_action) {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = { 0.0f, 0.0f, 0.0f, 1.0f },
            },
        },
        .label = "offscreen-pass",
    });
}

void gfx_render_end(void) {
    // End pass for user rendering
    sg_end_pass();

    // Display the offscreen image to a fullscreen quad and render the UI
    sg_begin_pass(&(sg_pass) {
        .swapchain = sglue_swapchain(),
        .action = {
            .colors[0] = (sg_color_attachment_action) {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = { 0.0f, 0.0f, 0.0f, 1.0f },
            },
        },
        .label = "display-pass",
    });
    {
        gui_update();
    }
    sg_end_pass();

    sg_commit();
}

void gfx_render_model(const model_t* model) {
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
        .samplers[SMP_u_sampler] = _state.sampler,
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

void gfx_shutdown(void) {
    sg_destroy_pipeline(_state.pipeline);
    sg_destroy_attachments(_state.attachments);
    sg_destroy_image(_state.color_image);
    sg_destroy_image(_state.depth_image);
    sg_destroy_sampler(_state.sampler);
    sg_shutdown();
}

sg_sampler gfx_get_sampler(void) {
    return _state.sampler;
}

static void _init(void) {

    _state.color_image = sg_make_image(&(sg_image_desc) {
        .render_target = true,
        .width = GFX_RENDER_WIDTH,
        .height = GFX_RENDER_HEIGHT,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .label = "color-image",
    });

    _state.depth_image = sg_make_image(&(sg_image_desc) {
        .render_target = true,
        .width = GFX_RENDER_WIDTH,
        .height = GFX_RENDER_HEIGHT,
        .pixel_format = SG_PIXELFORMAT_DEPTH,
        .label = "depth-image",
    });

    _state.attachments = sg_make_attachments(&(sg_attachments_desc) {
        .colors[0].image = _state.color_image,
        .depth_stencil.image = _state.depth_image,
        .label = "offscreen-attachments",
    });

    _state.sampler = sg_make_sampler(&(sg_sampler_desc) {
        .min_filter = SG_FILTER_NEAREST,
        .mag_filter = SG_FILTER_NEAREST,
        .wrap_u = SG_WRAP_REPEAT,
        .wrap_v = SG_WRAP_REPEAT,
    });

    _state.quad_vbuf = sg_make_buffer(&(sg_buffer_desc) {
        .data = SG_RANGE(shape_quad_vertices),
        .label = "quad-vertices",
    });

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

sg_buffer gfx_get_quad_vbuf(void) {
    return _state.quad_vbuf;
}

sg_image gfx_get_color_image(void) {
    return _state.color_image;
}

// API specific winding order. This is required because OpenGL uses CCW order
// for font-facing triangles, while Metal and DirectX use CW for front-facing
// triangles.
sg_face_winding gfx_get_face_winding(void) {
    sg_backend backend = sg_query_backend();
    // Return the appropriate face winding order based on the backend
    switch (backend) {
    case SG_BACKEND_D3D11:
    case SG_BACKEND_METAL_MACOS:
    case SG_BACKEND_METAL_IOS:
        return SG_FACEWINDING_CW;
    case SG_BACKEND_GLCORE:
    case SG_BACKEND_GLES3:
    default:
        return SG_FACEWINDING_CCW;
    }
}
