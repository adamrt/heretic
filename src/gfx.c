#include "cglm/struct/affine-pre.h"
#include "cglm/struct/affine.h"
#include "cglm/struct/mat4.h"
#include "cglm/struct/vec4.h"

#include "lighting.h"
#include "mesh.h"
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#include "shader.glsl.h"

#include "camera.h"
#include "gfx.h"
#include "gui.h"
#include "shape.h"

// Global gfx state
static gfx_t _state;

// Forward declarations
static void _init_images(void);
static void _init(void);
static void _render_display(void);
static mat4s model_matrix(transform_t);

static sg_face_winding face_winding;

// There are two passes so we can render the offscreen image to a fullscreen
// quad. The offscreen is rendered in a lower resolution and then upscaled to
// the window size to keep the pixelated look.
void gfx_init(void) {
    sg_setup(&(sg_desc) {
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });

    sg_backend backend = sg_query_backend();
    if (backend == SG_BACKEND_D3D11 || backend == SG_BACKEND_METAL_MACOS) {
        face_winding = SG_FACEWINDING_CW;
    } else {
        face_winding = SG_FACEWINDING_CCW;
    }

    _state.display.width = GFX_DISPLAY_WIDTH;
    _state.display.height = GFX_DISPLAY_HEIGHT;
    _state.display.scale_divisor = 2;

    _init_images();
    _init();
}

void gfx_render_begin(void) {
    // Render the scene and background to an offscreen image
    sg_begin_pass(&(sg_pass) {
        .attachments = _state.offscreen.attachments,
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
        _render_display();
        gui_update();
    }
    sg_end_pass();

    sg_commit();
}

void gfx_render_model(model_t* model, lighting_t* lighting) {
    mat4s model_mat = model_matrix(model->transform);

    vs_standard_params_t vs_params = {
        .u_proj = camera_get_proj(),
        .u_view = camera_get_view(),
        .u_model = model_mat,
    };

    fs_standard_params_t fs_params;
    fs_params.u_ambient_color = lighting->ambient_color;
    fs_params.u_ambient_strength = lighting->ambient_strength;

    int light_count = 0;
    for (int i = 0; i < LIGHTING_MAX_LIGHTS; i++) {

        light_t light = lighting->lights[i];
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
            [IMG_u_texture] = model->texture,
            [IMG_u_palette] = model->palette,
        },
    };

    sg_apply_pipeline(_state.offscreen.pipeline);
    sg_apply_bindings(&bindings);
    sg_apply_uniforms(0, &SG_RANGE(vs_params));
    sg_apply_uniforms(1, &SG_RANGE(fs_params));
    sg_draw(0, model->vertex_count, 1);
}

model_t gfx_map_to_model(map_t* map) {
    vertices_t vertices = geometry_to_vertices(&map->mesh.geometry);

    sg_buffer vbuf = sg_make_buffer(&(sg_buffer_desc) {
        .data = SG_RANGE(vertices),
        .label = "mesh-vertices",
    });

    sg_image texture = sg_make_image(&(sg_image_desc) {
        .width = TEXTURE_WIDTH,
        .height = TEXTURE_HEIGHT,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = SG_RANGE(map->texture.data),
    });

    texture_t grayscale_texture = texture_grascale(&map->texture);
    sg_image grayscale = sg_make_image(&(sg_image_desc) {
        .width = TEXTURE_WIDTH,
        .height = TEXTURE_HEIGHT,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = SG_RANGE(grayscale_texture.data),
    });

    sg_image palette = sg_make_image(&(sg_image_desc) {
        .width = PALETTE_WIDTH,
        .height = PALETTE_HEIGHT,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = SG_RANGE(map->mesh.palette.data),
    });

    vec3s centered_translation = vertices_centered(&vertices);

    model_t model = {
        .vertex_count = map->mesh.geometry.vertex_count,
        .center = centered_translation,
        .transform = {
            .translation = centered_translation, // Centering for now as it seems better.
            .scale = { { 1.0f, 1.0f, 1.0f } },
        },
        .vbuf = vbuf,
        .texture = texture,
        .grayscale = grayscale,
        .palette = palette,
    };
    return model;
}

void gfx_scale_change(void) {
    sg_destroy_image(_state.offscreen.color_image);
    sg_destroy_image(_state.offscreen.depth_image);
    sg_destroy_attachments(_state.offscreen.attachments);

    _init_images();

    _state.display.bindings.images[IMG_u_texture] = _state.offscreen.color_image;
    _state.display.bindings.samplers[SMP_u_sampler] = _state.sampler;
}

void gfx_shutdown(void) {
    sg_destroy_pipeline(_state.offscreen.pipeline);
    sg_destroy_pipeline(_state.display.pipeline);

    sg_destroy_attachments(_state.offscreen.attachments);
    sg_destroy_image(_state.offscreen.color_image);
    sg_destroy_image(_state.offscreen.depth_image);

    sg_destroy_sampler(_state.sampler);

    sg_shutdown();
}

int gfx_get_scale_divisor(void) {
    return _state.display.scale_divisor;
}

void gfx_set_scale_divisor(int divisor) {
    _state.display.scale_divisor = divisor;
    gfx_scale_change();
}

sg_sampler gfx_get_sampler(void) {
    return _state.sampler;
}

// This is split out from _init() because it can be called separately when
// changing the resolution.
static void _init_images(void) {

    int scaled_width = _state.display.width / _state.display.scale_divisor;
    int scaled_height = _state.display.height / _state.display.scale_divisor;

    _state.offscreen.color_image = sg_make_image(&(sg_image_desc) {
        .render_target = true,
        .width = scaled_width,
        .height = scaled_height,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .label = "color-image",
    });

    _state.offscreen.depth_image = sg_make_image(&(sg_image_desc) {
        .render_target = true,
        .width = scaled_width,
        .height = scaled_height,
        .pixel_format = SG_PIXELFORMAT_DEPTH,
        .label = "depth-image",
    });

    _state.offscreen.attachments = sg_make_attachments(&(sg_attachments_desc) {
        .colors[0].image = _state.offscreen.color_image,
        .depth_stencil.image = _state.offscreen.depth_image,
        .label = "offscreen-attachments",
    });
}

static void _init(void) {
    _init_images();

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

    _state.quad_ibuf = sg_make_buffer(&(sg_buffer_desc) {
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = SG_RANGE(shape_quad_indices),
        .label = "quad-indices",
    });

    //
    // Offscreen
    //

    _state.offscreen.pipeline = sg_make_pipeline(&(sg_pipeline_desc) {
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
        .face_winding = face_winding,
        .cull_mode = SG_CULLMODE_BACK,
        .depth = {
            .pixel_format = SG_PIXELFORMAT_DEPTH,
            .compare = SG_COMPAREFUNC_LESS,
            .write_enabled = true,
        },
        .colors[0].pixel_format = SG_PIXELFORMAT_RGBA8,
        .label = "standard-pipeline",
    });

    //
    // Display
    //

    _state.display.pipeline = sg_make_pipeline(&(sg_pipeline_desc) {
        .layout = {
            .buffers[0].stride = sizeof(vertex_t),
            .attrs = {
                [ATTR_display_a_position].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_display_a_position].offset = offsetof(vertex_t, position),
                [ATTR_display_a_uv].format = SG_VERTEXFORMAT_FLOAT2,
                [ATTR_display_a_uv].offset = offsetof(vertex_t, uv),
            },
        },
        .shader = sg_make_shader(display_shader_desc(sg_query_backend())),
        .index_type = SG_INDEXTYPE_UINT16,
        .cull_mode = SG_CULLMODE_NONE,
        .depth = {
            .compare = SG_COMPAREFUNC_LESS,
            .write_enabled = true,
        },
        .label = "quad-pipeline",
    });

    // Fullscreen quad vertices
    // clang-format off
    _state.display.bindings = (sg_bindings) {
        .vertex_buffers[0] = gfx_get_quad_vbuf(),
        .index_buffer = gfx_get_quad_ibuf(),
        .images[IMG_u_texture] = _state.offscreen.color_image,
        .samplers[SMP_u_sampler] = _state.sampler,
    };
}

sg_buffer gfx_get_quad_vbuf(void) {
    return _state.quad_vbuf;
}

sg_buffer gfx_get_quad_ibuf(void) {
    return _state.quad_ibuf;
}

static void _render_display(void)
{
    sg_apply_pipeline(_state.display.pipeline);
    sg_apply_bindings(&_state.display.bindings);
    sg_draw(0, 6, 1);
}

static mat4s model_matrix(transform_t transform)
{
    mat4s model_matrix = glms_mat4_identity();
    model_matrix = glms_translate(model_matrix, transform.translation);
    model_matrix = glms_rotate_x(model_matrix, transform.rotation.x);
    model_matrix = glms_rotate_y(model_matrix, transform.rotation.y);
    model_matrix = glms_rotate_z(model_matrix, transform.rotation.z);
    model_matrix = glms_scale(model_matrix, transform.scale);
    return model_matrix;
}
