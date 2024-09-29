#include <math.h>

#include "cglm/struct.h"

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#include "shader.glsl.h"

#include "cube.h"

#define MAX_MODELS 10

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

// application state
static struct {
    struct {
        sg_pass_action pass_action;
        sg_pipeline pip;
        sg_bindings bind;
    } gfx;

    struct {
        model_t models[MAX_MODELS];
        int num_models;
    } scene;

    camera_t camera;

    bool mouse_left;
    bool mouse_right;
} state;

// forward declarations
static void engine_init(void);
static void engine_event(const sapp_event* event);
static void engine_update(void);
static void engine_cleanup(void);

static void state_init(void);
static void state_update(void);

static void gfx_init(void);
static void gfx_frame_begin(void);
static void gfx_frame_end(void);

static mat4s model_world(const model_t* model);

static void camera_init(vec3s center, float distance, float azimuth, float elevation);
static void camera_update(void);
static void camera_rotate(float delta_azimuth, float delta_elevation);
static void camera_zoom(float delta);
static void camera_pan(float delta_x, float delta_y);
static mat4s camera_view_matrix(void);

sapp_desc sokol_main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    return (sapp_desc) {
        .init_cb = engine_init,
        .event_cb = engine_event,
        .frame_cb = engine_update,
        .cleanup_cb = engine_cleanup,
        .width = 1280,
        .height = 960,
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
        } else if (state.mouse_right) {
            camera_pan(event->mouse_dx * 0.01f, event->mouse_dy * 0.01f);
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

    gfx_frame_begin();
    {
        for (int i = 0; i < state.scene.num_models; i++) {
            vs_params_t vs_params;
            vs_params.mvp = state.scene.models[i].mvp;
            sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &SG_RANGE(vs_params));
            sg_draw(0, 36, 1);
        }
    }
    gfx_frame_end();
}

static void engine_cleanup(void)
{
    sg_shutdown();
}

static void state_init(void)
{
    camera_init((vec3s) { { 0.0f, 0.0f, 0.0f } }, 5.0f, 0.0f, 0.0f);

    float trans_x_base = -2.0f;
    for (int i = 0; i < 3; i++) {
        model_t* model = &state.scene.models[i];
        float trans_x = trans_x_base + (2.0f * i);
        model->translation = (vec3s) { { trans_x, 0.0f, 0.0f } };
        model->rotation = (vec3s) { { 0.0f, 0.0f, 0.0f } };
        model->scale = (vec3s) { { 0.5f, 0.5f, 0.5f } };

        state.scene.num_models++;
    }
}

static void gfx_init(void)
{
    sg_setup(&(sg_desc) {
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });

    sg_buffer vbuf = sg_make_buffer(&(sg_buffer_desc) {
        .data = SG_RANGE(vertices),
        .label = "cube-vertices",
    });

    sg_buffer ibuf = sg_make_buffer(&(sg_buffer_desc) {
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = SG_RANGE(indices),
        .label = "cube-indices",
    });

    sg_shader shd = sg_make_shader(cube_shader_desc(sg_query_backend()));

    state.gfx.pip = sg_make_pipeline(&(sg_pipeline_desc) {
        .layout = {
            .buffers[0].stride = 28,
            .attrs = {
                [ATTR_vs_position].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_vs_color0].format = SG_VERTEXFORMAT_FLOAT4,
            },
        },
        .shader = shd,
        .index_type = SG_INDEXTYPE_UINT16,
        .cull_mode = SG_CULLMODE_NONE,
        .depth = {
            .write_enabled = true,
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
        },
        .label = "cube-pipeline",
    });

    state.gfx.bind = (sg_bindings) {
        .vertex_buffers[0] = vbuf,
        .index_buffer = ibuf
    };
}

static void gfx_frame_begin(void)
{
    sg_begin_pass(&(sg_pass) {
        .action = {
            .colors[0] = {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = { 0.25f, 0.5f, 0.75f, 1.0f },
            },
        },
        .swapchain = sglue_swapchain(),
    });

    sg_apply_pipeline(state.gfx.pip);
    sg_apply_bindings(&state.gfx.bind);

    for (int i = 0; i < state.scene.num_models; i++) {
        model_t* model = &state.scene.models[i];

        vs_params_t vs_params = { .mvp = model->mvp };
        sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &SG_RANGE(vs_params));

        sg_draw(0, 36, 1);
    }
}

static void gfx_frame_end(void)
{
    sg_end_pass();
    sg_commit();
}

// state_update updates the application state each frame.
static void state_update(void)
{
    // const float t = (float)sapp_frame_duration();

    mat4s view = camera_view_matrix();
    mat4s view_proj = glms_mat4_mul(state.camera.proj, view);

    for (int i = 0; i < state.scene.num_models; i++) {
        model_t* model = &state.scene.models[i];
        // model->rotation.x += 1.0f * t;
        // model->rotation.y += 2.0f * t;

        mat4s world = model_world(model);
        mat4s mvp = glms_mat4_mul(view_proj, world);
        model->mvp = mvp;
    }
}

// model_world returns the world/transform matrix for a model.
static mat4s model_world(const model_t* model)
{
    mat4s world = glms_mat4_identity();
    world = glms_translate(world, model->translation);
    world = glms_rotate_x(world, model->rotation.x);
    world = glms_rotate_y(world, model->rotation.y);
    world = glms_rotate_z(world, model->rotation.z);
    world = glms_scale(world, model->scale);
    return world;
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

static void camera_update()
{
    // Convert azimuth and elevation (in radians) to spherical coordinates
    float x = state.camera.distance * cosf(state.camera.elevation) * sinf(state.camera.azimuth);
    float y = state.camera.distance * sinf(state.camera.elevation);
    float z = state.camera.distance * cosf(state.camera.elevation) * cosf(state.camera.azimuth);

    // Update the eye (camera position)
    state.camera.eye = (vec3s) { { x, y, z } };
}

static void camera_rotate(float delta_azimuth, float delta_elevation)
{
    state.camera.azimuth -= delta_azimuth;
    state.camera.elevation += delta_elevation;

    // Clamp elevation to avoid flipping at the poles (optional)
    float max_elevation = M_PI_2 - 0.01f; // Near 90 degrees
    float min_elevation = -max_elevation;
    if (state.camera.elevation > max_elevation)
        state.camera.elevation = max_elevation;
    if (state.camera.elevation < min_elevation)
        state.camera.elevation = min_elevation;

    camera_update();
}

static void camera_zoom(float delta)
{
    state.camera.distance -= delta;

    if (state.camera.distance < 0.1f) {
        state.camera.distance = 0.1f;
    }

    camera_update();
}

static void camera_pan(float delta_x, float delta_y)
{
    // Scale panning by distance to make it feel consistent
    float pan_speed = state.camera.distance * 0.1f;

    vec3s right = glms_cross(glms_vec3_sub(state.camera.eye, state.camera.center), state.camera.up);
    right = glms_normalize(right);

    vec3s up = glms_normalize(state.camera.up);

    state.camera.center = glms_vec3_add(state.camera.center, glms_vec3_scale(right, delta_x * pan_speed));
    state.camera.center = glms_vec3_add(state.camera.center, glms_vec3_scale(up, delta_y * pan_speed));

    camera_update();
}

static mat4s camera_view_matrix(void)
{
    return glms_lookat(state.camera.eye, state.camera.center, state.camera.up);
}
