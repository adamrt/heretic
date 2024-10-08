#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "cglm/struct.h"

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_STANDARD_VARARGS
#include "nuklear.h"
#include "util/sokol_nuklear.h"

#include "shader.glsl.h"

#include "bin.h"
#include "camera.h"
#include "model.h"

#define GFX_SCALE (2)

#define GFX_OFFSCREEN_WIDTH (640)
#define GFX_OFFSCREEN_HEIGHT (480)

#define GFX_DISPLAY_WIDTH (GFX_OFFSCREEN_WIDTH * GFX_SCALE)
#define GFX_DISPLAY_HEIGHT (GFX_OFFSCREEN_HEIGHT * GFX_SCALE)

#define STATE_MAX_MODELS (125)

// Fullscreen quad vertices
// clang-format off
vertex_t quad_vertices[] = {
    { .position = {{ -1.0f, -1.0f, 0.0f }}, .uv = {{ 0.0f, 1.0f }} }, // bottom-left
    { .position = {{  1.0f, -1.0f, 0.0f }}, .uv = {{ 1.0f, 1.0f }} }, // bottom-right
    { .position = {{ -1.0f,  1.0f, 0.0f }}, .uv = {{ 0.0f, 0.0f }} }, // top-left
    { .position = {{  1.0f,  1.0f, 0.0f }}, .uv = {{ 1.0f, 0.0f }} }  // top-right
};

uint16_t quad_indices[] = { 0, 1, 2, 1, 3, 2 };
// clang-format on

// Application state
static struct {
    struct {
        sg_image color_image;
        sg_image depth_image;

        sg_sampler default_sampler;
        sg_pass_action default_pass_action;

        struct {
            sg_pipeline pipeline;
            sg_pass pass;
        } offscreen;

        struct {
            sg_pipeline pipeline;
            sg_bindings bindings;
        } display;

    } gfx;

    struct {
        model_t models[STATE_MAX_MODELS];
        int num_models;
        bool center_model;

        struct {
            vec3s light_position;
            vec4s light_color;
            vec4s ambient_color;
            float ambient_strength;
        } lighting;
    } scene;

    struct {
        bool mouse_left;
        bool mouse_right;
    } input;

    struct {
        FILE* bin;
    } fft;
} state;

// Forward declarations
static void engine_init(void);
static void engine_event(const sapp_event* event);
static void engine_update(void);
static void engine_cleanup(void);

static void state_init(void);
static void state_update(void);
static void state_add_model(model_t model);
static void state_load_map(int num);

static void gfx_init(void);
static void gfx_frame(void);

static void ui_init(void);
static void ui_frame(void);
static void ui_draw(struct nk_context* ctx);

sapp_desc sokol_main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    return (sapp_desc) {
        .init_cb = engine_init,
        .event_cb = engine_event,
        .frame_cb = engine_update,
        .cleanup_cb = engine_cleanup,
        .width = GFX_DISPLAY_WIDTH,
        .height = GFX_DISPLAY_HEIGHT,
        .window_title = "Starterkit",
        .icon.sokol_default = true,
        .logger.func = slog_func,
    };
}

static void engine_init(void)
{
    gfx_init();
    state_init();
    ui_init();
}

static void engine_event(const sapp_event* event)
{
    bool handled = snk_handle_event(event);
    if (handled) {
        return;
    }

    switch (event->type) {
    case SAPP_EVENTTYPE_KEY_DOWN:
        if (event->key_code == SAPP_KEYCODE_ESCAPE) {
            sapp_request_quit();
        }
        break;

    case SAPP_EVENTTYPE_MOUSE_DOWN:
    case SAPP_EVENTTYPE_MOUSE_UP: {
        bool is_down = (event->type == SAPP_EVENTTYPE_MOUSE_DOWN);
        if (event->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
            sapp_lock_mouse(is_down);
            state.input.mouse_left = is_down;
        }
        if (event->mouse_button == SAPP_MOUSEBUTTON_RIGHT) {
            sapp_lock_mouse(is_down);
            state.input.mouse_right = is_down;
        }
        break;
    }

    case SAPP_EVENTTYPE_MOUSE_MOVE:
        if (state.input.mouse_left) {
            camera_rotate(event->mouse_dx * 0.01f, event->mouse_dy * 0.01f);
        }
        break;

    case SAPP_EVENTTYPE_MOUSE_SCROLL:
        camera_zoom(event->scroll_y * 0.01f);
        break;

    default:
        break;
    }
}

static void engine_update(void)
{
    state_update();
    gfx_frame();
}

static void engine_cleanup(void)
{
    sg_shutdown();
}

static void state_init(void)
{

    camera_init();

    state.scene.center_model = true;
    state.scene.lighting.light_position = (vec3s) { { 0.0f, 0.0f, 10.0f } };
    state.scene.lighting.light_color = (vec4s) { { 1.0f, 1.0f, 1.0f, 1.0f } };
    state.scene.lighting.ambient_color = (vec4s) { { 1.0f, 1.0f, 1.0f, 1.0f } };
    state.scene.lighting.ambient_strength = 0.4f;

    state.fft.bin = fopen("/Users/adam/sync/emu/fft.bin", "rb");
    if (state.fft.bin == NULL) {
        assert(false);
    }

    state_load_map(12);
}

static void state_load_map(int num)
{
    model_t model = bin_map(state.fft.bin, num);

    sg_buffer vbuf = sg_make_buffer(&(sg_buffer_desc) {
        .data = SG_RANGE(model.geometry.vertices),
        .label = "mesh-vertices",
    });

    sg_image texture = sg_make_image(&(sg_image_desc) {
        .width = TEXTURE_WIDTH,
        .height = TEXTURE_HEIGHT,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = SG_RANGE(model.texture.data),
    });

    sg_image palette = sg_make_image(&(sg_image_desc) {
        .width = PALETTE_WIDTH,
        .height = PALETTE_HEIGHT,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = SG_RANGE(model.palette.data),
    });

    model.bindings = (sg_bindings) {
        .vertex_buffers[0] = vbuf,
        .fs = {
            .images[SLOT_u_texture] = texture,
            .images[SLOT_u_palette] = palette,
            .samplers[SLOT_u_sampler] = state.gfx.default_sampler,
        },
    };

    state_add_model(model);
}

static void state_add_model(model_t model)
{
    state.scene.models[state.scene.num_models] = model;
    state.scene.num_models++;
}

static void state_update(void)
{
    camera_update();

    for (int i = 0; i < state.scene.num_models; i++) {
        model_t* model = &state.scene.models[i];
        if (state.scene.center_model) {
            model->transform.translation = model->centered_translation;
        } else {
            model->transform.translation = (vec3s) { { 0.0f, 0.0f, 0.0f } };
        }

        mat4s model_matrix = glms_mat4_identity();
        model_matrix = glms_translate(model_matrix, model->transform.translation);
        model_matrix = glms_rotate_x(model_matrix, model->transform.rotation.x);
        model_matrix = glms_rotate_y(model_matrix, model->transform.rotation.y);
        model_matrix = glms_rotate_z(model_matrix, model->transform.rotation.z);
        model_matrix = glms_scale(model_matrix, model->transform.scale);

        model->model_matrix = model_matrix;
    }
}

// There are two passes so we can render the offscreen image to a fullscreen
// quad. The offscreen is rendered at the native PS1 FFT resolution and then
// upscaled to the window size to keep the pixelated look.
static void gfx_init(void)
{
    sg_setup(&(sg_desc) {
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });

    state.gfx.color_image = sg_make_image(&(sg_image_desc) {
        .render_target = true,
        .width = GFX_OFFSCREEN_WIDTH,
        .height = GFX_OFFSCREEN_HEIGHT,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .label = "color-image",
    });

    state.gfx.depth_image = sg_make_image(&(sg_image_desc) {
        .render_target = true,
        .width = GFX_OFFSCREEN_WIDTH,
        .height = GFX_OFFSCREEN_HEIGHT,
        .pixel_format = SG_PIXELFORMAT_DEPTH,
        .label = "depth-image",
    });

    state.gfx.default_sampler = sg_make_sampler(&(sg_sampler_desc) {
        .min_filter = SG_FILTER_NEAREST,
        .mag_filter = SG_FILTER_NEAREST,
        .wrap_u = SG_WRAP_REPEAT,
        .wrap_v = SG_WRAP_REPEAT,
    });

    state.gfx.default_pass_action = (sg_pass_action) {
        .colors[0] = (sg_color_attachment_action) {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = { 0.25f, 0.45f, 0.65f, 1.0f },
        },
    };

    // Offscreen setup
    {
        state.gfx.offscreen.pass = (sg_pass) {
            .attachments = sg_make_attachments(&(sg_attachments_desc) {
                .colors[0].image = state.gfx.color_image,
                .depth_stencil.image = state.gfx.depth_image,
                .label = "offscreen-attachments",
            }),
            .action = state.gfx.default_pass_action,
            .label = "offscreen-pass",
        };

        state.gfx.offscreen.pipeline = sg_make_pipeline(&(sg_pipeline_desc) {
            .layout = {
                .attrs = {
                    [ATTR_standard_vs_a_position].format = SG_VERTEXFORMAT_FLOAT3,
                    [ATTR_standard_vs_a_normal].format = SG_VERTEXFORMAT_FLOAT3,
                    [ATTR_standard_vs_a_uv].format = SG_VERTEXFORMAT_FLOAT2,
                    [ATTR_standard_vs_a_palette_index].format = SG_VERTEXFORMAT_FLOAT,
                },
            },
            .shader = sg_make_shader(standard_shader_desc(sg_query_backend())),
            .face_winding = SG_FACEWINDING_CW,
            .cull_mode = SG_CULLMODE_BACK,
            .depth = {
                .pixel_format = SG_PIXELFORMAT_DEPTH,
                .compare = SG_COMPAREFUNC_LESS_EQUAL,
                .write_enabled = true,
            },
            .colors[0].pixel_format = SG_PIXELFORMAT_RGBA8,
            .label = "standard-pipeline",
        });
    }

    // Display setup
    {
        state.gfx.display.pipeline = sg_make_pipeline(&(sg_pipeline_desc) {
            .layout = {
                .buffers[0].stride = sizeof(vertex_t),
                .attrs = {
                    [ATTR_quad_vs_a_position].format = SG_VERTEXFORMAT_FLOAT3,
                    [ATTR_quad_vs_a_position].offset = offsetof(vertex_t, position),
                    [ATTR_quad_vs_a_uv].format = SG_VERTEXFORMAT_FLOAT2,
                    [ATTR_quad_vs_a_uv].offset = offsetof(vertex_t, uv),
                },
            },
            .shader = sg_make_shader(quad_shader_desc(sg_query_backend())),
            .index_type = SG_INDEXTYPE_UINT16,
            .cull_mode = SG_CULLMODE_NONE,
            .depth = {
                .compare = SG_COMPAREFUNC_LESS_EQUAL,
                .write_enabled = true,
            },
            .label = "quad-pipeline",
        });

        sg_buffer quad_vbuf = sg_make_buffer(&(sg_buffer_desc) {
            .data = SG_RANGE(quad_vertices),
            .label = "quad-vertices",
        });

        sg_buffer quad_ibuf = sg_make_buffer(&(sg_buffer_desc) {
            .type = SG_BUFFERTYPE_INDEXBUFFER,
            .data = SG_RANGE(quad_indices),
            .label = "quad-indices",
        });

        state.gfx.display.bindings = (sg_bindings) {
            .vertex_buffers[0] = quad_vbuf,
            .index_buffer = quad_ibuf,
            .fs = {
                .images[SLOT_tex] = state.gfx.color_image,
                .samplers[SLOT_smp] = state.gfx.default_sampler,
            }
        };
    }
}

static void gfx_frame(void)
{
    // Offscreen pass
    {
        sg_begin_pass(&state.gfx.offscreen.pass);
        sg_apply_pipeline(state.gfx.offscreen.pipeline);

        for (int i = 0; i < state.scene.num_models; i++) {
            model_t* model = &state.scene.models[i];

            mat4s proj = camera_proj();
            mat4s view = camera_view();

            vs_standard_params_t vs_params = {
                .u_proj = proj,
                .u_view = view,
                .u_model = model->model_matrix,
            };

            fs_standard_params_t fs_params = {
                .u_light_position = state.scene.lighting.light_position,
                .u_light_color = state.scene.lighting.light_color,
                .u_ambient_color = state.scene.lighting.ambient_color,
                .u_ambient_strength = state.scene.lighting.ambient_strength,
            };

            sg_apply_bindings(&model->bindings);
            sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_standard_params, &SG_RANGE(vs_params));
            sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_standard_params, &SG_RANGE(fs_params));
            sg_draw(0, model->geometry.count, 1);
        }
        sg_end_pass();
    }

    // Display pass
    {
        sg_begin_pass(&(sg_pass) {
            .action = state.gfx.default_pass_action,
            .swapchain = sglue_swapchain(),
            .label = "swapchain-pass",
        });

        sg_apply_pipeline(state.gfx.display.pipeline);
        sg_apply_bindings(&state.gfx.display.bindings);

        sg_draw(0, 6, 1);
        ui_frame();
        sg_end_pass();
    }

    sg_commit();
}

static void ui_init(void)
{
    snk_setup(&(snk_desc_t) {
        .dpi_scale = sapp_dpi_scale(),
        .logger.func = slog_func,
    });
}

static void ui_frame(void)
{
    struct nk_context* ctx = snk_new_frame();

    ui_draw(ctx);

    snk_render(sapp_width(), sapp_height());
}

static void ui_draw(struct nk_context* ctx)
{
    if (nk_begin(ctx, "Starterkit", nk_rect(10, 25, 250, 400), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE | NK_WINDOW_MINIMIZABLE)) {
        nk_layout_row_dynamic(ctx, 30, 1);

        nk_bool centered = state.scene.center_model;
        nk_checkbox_label(ctx, "Centered", &centered);
        state.scene.center_model = centered;

        nk_label(ctx, "Light Color", NK_TEXT_LEFT);
        vec4s* light_color = &state.scene.lighting.light_color;
        struct nk_colorf light_color_nk = { light_color->r, light_color->g, light_color->b, light_color->a };
        if (nk_combo_begin_color(ctx, nk_rgba_f(light_color->r, light_color->g, light_color->b, light_color->a), nk_vec2(200, 400))) {
            nk_layout_row_dynamic(ctx, 120, 1);

            light_color_nk = nk_color_picker(ctx, light_color_nk, NK_RGBA);

            nk_layout_row_dynamic(ctx, 25, 2);
            nk_layout_row_dynamic(ctx, 25, 2);
            nk_option_label(ctx, "RGBA", true);
            nk_layout_row_dynamic(ctx, 25, 1);

            light_color_nk.r = nk_propertyf(ctx, "#R:", 0, light_color_nk.r, 1.0f, 0.01f, 0.005f);
            light_color_nk.g = nk_propertyf(ctx, "#G:", 0, light_color_nk.g, 1.0f, 0.01f, 0.005f);
            light_color_nk.b = nk_propertyf(ctx, "#B:", 0, light_color_nk.b, 1.0f, 0.01f, 0.005f);
            light_color_nk.a = nk_propertyf(ctx, "#A:", 0, light_color_nk.a, 1.0f, 0.01f, 0.005f);

            *light_color = (vec4s) { { light_color_nk.r, light_color_nk.g, light_color_nk.b, light_color_nk.a } };
            nk_combo_end(ctx);
        }

        // Light position
        nk_label(ctx, "Light Position", NK_TEXT_LEFT);
        vec3s* position = &state.scene.lighting.light_position;
        char buffer[64];
        sprintf(buffer, "%.2f, %.2f, %.2f", position->x, position->y, position->z);
        if (nk_combo_begin_label(ctx, buffer, nk_vec2(200, 200))) {
            nk_layout_row_dynamic(ctx, 25, 1);
            nk_property_float(ctx, "#X:", -30.0f, &position->x, 30.0f, 1, 0.5f);
            nk_property_float(ctx, "#Y:", -30.0f, &position->y, 30.0f, 1, 0.5f);
            nk_property_float(ctx, "#Z:", -30.0f, &position->z, 30.0f, 1, 0.5f);
            nk_combo_end(ctx);
        }

        nk_label(ctx, "Ambient Color", NK_TEXT_LEFT);
        vec4s* ambient_color = &state.scene.lighting.ambient_color;
        struct nk_colorf ambient_color_nk = { ambient_color->r, ambient_color->g, ambient_color->b, ambient_color->a };
        if (nk_combo_begin_color(ctx, nk_rgba_f(ambient_color->r, ambient_color->g, ambient_color->b, ambient_color->a), nk_vec2(200, 400))) {
            nk_layout_row_dynamic(ctx, 120, 1);

            ambient_color_nk = nk_color_picker(ctx, ambient_color_nk, NK_RGBA);

            nk_layout_row_dynamic(ctx, 25, 2);
            nk_option_label(ctx, "RGBA", true);
            nk_layout_row_dynamic(ctx, 25, 1);

            ambient_color_nk.r = nk_propertyf(ctx, "#R:", 0, ambient_color_nk.r, 1.0f, 0.01f, 0.005f);
            ambient_color_nk.g = nk_propertyf(ctx, "#G:", 0, ambient_color_nk.g, 1.0f, 0.01f, 0.005f);
            ambient_color_nk.b = nk_propertyf(ctx, "#B:", 0, ambient_color_nk.b, 1.0f, 0.01f, 0.005f);
            ambient_color_nk.a = nk_propertyf(ctx, "#A:", 0, ambient_color_nk.a, 1.0f, 0.01f, 0.005f);

            *ambient_color = (vec4s) { { ambient_color_nk.r, ambient_color_nk.g, ambient_color_nk.b, ambient_color_nk.a } };
            nk_combo_end(ctx);
        }

        nk_label(ctx, "Ambient Strength", NK_TEXT_LEFT);
        size_t prog_value = (size_t)(state.scene.lighting.ambient_strength * 100.0f);
        nk_progress(ctx, &prog_value, 100, NK_MODIFIABLE);
        state.scene.lighting.ambient_strength = (float)prog_value / 100.0f;
    }

    nk_end(ctx);
    return;
}
