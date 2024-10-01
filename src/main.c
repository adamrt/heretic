#include <math.h>

#include "cglm/struct.h"
#include "cglm/util.h"

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#include "shader.glsl.h"

#include "cube.h"

static const int GFX_OFFSCREEN_WIDTH = 320;
static const int GFX_OFFSCREEN_HEIGHT = 240;
static const int GFX_DISPLAY_WIDTH = GFX_OFFSCREEN_WIDTH * 4;
static const int GFX_DISPLAY_HEIGHT = GFX_OFFSCREEN_HEIGHT * 4;

#define STATE_MAX_MODELS 125

typedef struct {
    vec3s translation;
    vec3s rotation;
    vec3s scale;

    mat4s mvp;
} model_t;

typedef struct {
    mat4s proj;

    vec3s eye;
    vec3s center;
    vec3s up;

    float azimuth;
    float elevation;
    float distance;
} camera_t;

// Application state
static struct {
    struct {
        struct {
            sg_pass pass;
            sg_pipeline pipeline;
            sg_bindings bindings;
            sg_image color_image;
        } offscreen;
        struct {
            sg_pass_action pass_action;
            sg_pipeline pipeline;
            sg_bindings bindings;
        } display;
    } gfx;

    struct {
        model_t models[STATE_MAX_MODELS];
        int num_models;
    } scene;

    camera_t camera;

    bool mouse_left;
    bool mouse_right;
} state;

// Forward declarations
static void engine_init(void);
static void engine_event(const sapp_event* event);
static void engine_update(void);
static void engine_cleanup(void);

static void state_init(void);
static void state_update(void);

static void gfx_init(void);
static void gfx_offscreen_init(void);
static void gfx_display_init(void);
static void gfx_frame(void);

static void camera_init(vec3s center, float distance, float azimuth, float elevation);
static void camera_update(void);
static void camera_rotate(float delta_azimuth, float delta_elevation);
static void camera_zoom(float delta);

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
    state_init();
    gfx_init();
}

static void engine_event(const sapp_event* event)
{
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
            state.mouse_left = is_down;
        }
        if (event->mouse_button == SAPP_MOUSEBUTTON_RIGHT) {
            sapp_lock_mouse(is_down);
            state.mouse_right = is_down;
        }
        break;
    }

    case SAPP_EVENTTYPE_MOUSE_MOVE:
        if (state.mouse_left) {
            camera_rotate(event->mouse_dx * 0.01f, event->mouse_dy * 0.01f);
        }
        break;

    case SAPP_EVENTTYPE_MOUSE_SCROLL:
        camera_zoom(event->scroll_y * 0.1f);
        break;

    default:
        break;
    }
}

static void engine_update(void)
{
    state_update();
    camera_update();
    gfx_frame();
}

static void engine_cleanup(void)
{
    sg_shutdown();
}

static void state_init(void)
{
    camera_init((vec3s) { { 0.0f, 0.0f, 0.0f } }, 15.0f, 0.0f, 0.0f);

    vec3s trans_base = (vec3s) { { -4.0f, -4.0f, -4.0f } };
    int index = 0;

    for (int i = 0; i < 5; i++) {
        float trans_x = trans_base.x + (2.0f * i);
        for (int j = 0; j < 5; j++) {
            float trans_y = trans_base.y + (2.0f * j);
            for (int k = 0; k < 5; k++) {
                float trans_z = trans_base.z + (2.0f * k);

                index = i * (5 * 5) + j * 5 + k;
                model_t* model = &state.scene.models[index];

                model->translation = (vec3s) { { trans_x, trans_y, trans_z } };
                model->rotation = (vec3s) { { 0.0f, 0.0f, 0.0f } };
                model->scale = (vec3s) { { 0.5f, 0.5f, 0.5f } };
                state.scene.num_models++;
            }
        }
    }
}

// state_update updates the application state each frame.
static void state_update(void)
{
    mat4s view = glms_lookat(state.camera.eye, state.camera.center, state.camera.up);
    mat4s view_proj = glms_mat4_mul(state.camera.proj, view);

    for (int i = 0; i < state.scene.num_models; i++) {
        model_t* model = &state.scene.models[i];
        mat4s world = glms_mat4_identity();
        world = glms_translate(world, model->translation);
        world = glms_rotate_x(world, model->rotation.x);
        world = glms_rotate_y(world, model->rotation.y);
        world = glms_rotate_z(world, model->rotation.z);
        world = glms_scale(world, model->scale);
        mat4s mvp = glms_mat4_mul(view_proj, world);
        model->mvp = mvp;
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

    gfx_offscreen_init();
    gfx_display_init();
}

static void gfx_offscreen_init(void)
{
    // This is shared with gfx.display so its kept in state.
    state.gfx.offscreen.color_image = sg_make_image(&(sg_image_desc) {
        .render_target = true,
        .width = GFX_OFFSCREEN_WIDTH,
        .height = GFX_OFFSCREEN_HEIGHT,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .label = "color-image",
    });

    sg_image depth_image = sg_make_image(&(sg_image_desc) {
        .render_target = true,
        .width = GFX_OFFSCREEN_WIDTH,
        .height = GFX_OFFSCREEN_HEIGHT,
        .pixel_format = SG_PIXELFORMAT_DEPTH,
        .label = "depth-image",
    });

    state.gfx.offscreen.pass = (sg_pass) {
        .attachments = sg_make_attachments(&(sg_attachments_desc) {
            .colors[0].image = state.gfx.offscreen.color_image,
            .depth_stencil.image = depth_image,
            .label = "offscreen-attachments",
        }),
        .action = {
            .colors[0] = {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = { 0.25f, 0.45f, 0.65f, 1.0f },
            },
        },
        .label = "offscreen-pass"
    };

    state.gfx.offscreen.pipeline = sg_make_pipeline(&(sg_pipeline_desc) {
        .layout = {
            .attrs = {
                [ATTR_cube_vs_position].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_cube_vs_color0].format = SG_VERTEXFORMAT_FLOAT4,
            },
        },
        .shader = sg_make_shader(cube_shader_desc(sg_query_backend())),
        .index_type = SG_INDEXTYPE_UINT16,
        .cull_mode = SG_CULLMODE_BACK,
        .depth = {
            .pixel_format = SG_PIXELFORMAT_DEPTH,
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true,
        },
        .colors[0].pixel_format = SG_PIXELFORMAT_RGBA8,
        .label = "cube-pipeline",
    });

    sg_buffer cube_vbuf = sg_make_buffer(&(sg_buffer_desc) {
        .data = SG_RANGE(cube_vertices),
        .label = "cube-vertices",
    });

    sg_buffer cube_ibuf = sg_make_buffer(&(sg_buffer_desc) {
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = SG_RANGE(cube_indices),
        .label = "cube-indices",
    });

    state.gfx.offscreen.bindings = (sg_bindings) {
        .vertex_buffers[0] = cube_vbuf,
        .index_buffer = cube_ibuf
    };
}

static void gfx_display_init(void)
{
    state.gfx.display.pass_action = (sg_pass_action) {
        .colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = { 0.25f, 0.45f, 0.65f, 1.0f },
        }
    };

    state.gfx.display.pipeline = sg_make_pipeline(&(sg_pipeline_desc) {
        .layout = {
            .attrs = {
                [ATTR_quad_vs_position].format = SG_VERTEXFORMAT_FLOAT2,
                [ATTR_quad_vs_texcoord0].format = SG_VERTEXFORMAT_FLOAT2,
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

    sg_sampler smp = sg_make_sampler(&(sg_sampler_desc) {
        .min_filter = SG_FILTER_NEAREST,
        .mag_filter = SG_FILTER_NEAREST,
        .wrap_u = SG_WRAP_REPEAT,
        .wrap_v = SG_WRAP_REPEAT,
    });

    state.gfx.display.bindings = (sg_bindings) {
        .vertex_buffers[0] = quad_vbuf,
        .index_buffer = quad_ibuf,
        .fs = {
            .images[SLOT_tex] = state.gfx.offscreen.color_image,
            .samplers[SLOT_smp] = smp,
        }
    };
}

static void gfx_frame(void)
{
    // Offscreen pass
    sg_begin_pass(&state.gfx.offscreen.pass);
    sg_apply_pipeline(state.gfx.offscreen.pipeline);
    sg_apply_bindings(&state.gfx.offscreen.bindings);
    for (int i = 0; i < state.scene.num_models; i++) {
        model_t* model = &state.scene.models[i];
        vs_params_t vs_params = { .mvp = model->mvp };
        sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &SG_RANGE(vs_params));
        sg_draw(0, 36, 1);
    }
    sg_end_pass();

    // Display pass
    sg_begin_pass(&(sg_pass) {
        .action = state.gfx.display.pass_action,
        .swapchain = sglue_swapchain(),
        .label = "swapchain-pass",
    });
    sg_apply_pipeline(state.gfx.display.pipeline);
    sg_apply_bindings(&state.gfx.display.bindings);
    sg_draw(0, 6, 1);
    sg_end_pass();

    // Commit passes
    sg_commit();
}

static void camera_init(vec3s center, float distance, float azimuth, float elevation)
{
    const float w = sapp_widthf();
    const float h = sapp_heightf();
    state.camera.proj = glms_perspective(glm_rad(60.0f), w / h, 0.01f, 100.0f);
    state.camera.center = center;
    state.camera.distance = distance;
    state.camera.azimuth = azimuth;
    state.camera.elevation = elevation;
    state.camera.up = (vec3s) { { 0.0f, 1.0f, 0.0f } };

    camera_update();
}

static void camera_update(void)
{
    // Convert azimuth and elevation (in radians) to spherical coordinates
    float x = state.camera.distance * cosf(state.camera.elevation) * sinf(state.camera.azimuth);
    float y = state.camera.distance * sinf(state.camera.elevation);
    float z = state.camera.distance * cosf(state.camera.elevation) * cosf(state.camera.azimuth);

    state.camera.eye = (vec3s) { { x, y, z } };
}

static void camera_rotate(float delta_azimuth, float delta_elevation)
{
    state.camera.azimuth -= delta_azimuth;
    state.camera.elevation += delta_elevation;

    float max_elevation = M_PI_2 - 0.01f; // Near 90 degrees
    float min_elevation = -max_elevation;
    state.camera.elevation = glm_clamp(state.camera.elevation, min_elevation, max_elevation);
}

static void camera_zoom(float delta)
{
    state.camera.distance -= delta;

    if (state.camera.distance < 0.1f) {
        state.camera.distance = 0.1f;
    }
}
