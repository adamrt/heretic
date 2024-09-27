#include <math.h>

#include "cglm/struct.h"

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#include "shader.glsl.h"

#include "cube.h"

// application state
static struct {
    struct {
        sg_pipeline pip;
        sg_bindings bind;
        sg_pass_action pass_action;
    } gfx;

    struct {
        mat4s mvp;
        float rx, ry;
    } model;

} state;

// forward declarations
static void engine_init(void);
static void engine_event(const sapp_event* event);
static void engine_update(void);
static void engine_cleanup(void);

static void gfx_init(void);
static void gfx_frame_begin(void);
static void gfx_frame_end(void);

static void update(void);

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
        .height = 1024,
        .window_title = "Starterkit",
        .icon.sokol_default = true,
        .logger.func = slog_func,
    };
}

static void engine_init(void)
{
    gfx_init();
}

static void engine_event(const sapp_event* event)
{
    if (event->type == SAPP_EVENTTYPE_KEY_DOWN) {
        if (event->key_code == SAPP_KEYCODE_ESCAPE) {
            sapp_request_quit();
        }
    }
}

static void engine_update(void)
{
    update();

    vs_params_t vs_params;
    vs_params.mvp = state.model.mvp;

    gfx_frame_begin();
    {
        sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &SG_RANGE(vs_params));
        sg_draw(0, 36, 1);
    }
    gfx_frame_end();
}

static void engine_cleanup(void)
{
    sg_shutdown();
}

static void update(void)
{
    const float w = sapp_widthf();
    const float h = sapp_heightf();
    const float t = (float)sapp_frame_duration();
    state.model.rx += 1.0f * t;
    state.model.ry += 2.0f * t;
    mat4s rxm = glms_rotate_make(state.model.rx, GLMS_XUP);
    mat4s rym = glms_rotate_make(state.model.ry, GLMS_YUP);

    mat4s proj = glms_perspective(glm_rad(60.0f), w / h, 0.01f, 100.0f);

    vec3s eye = { { 0.0f, 1.5f, 6.0f } };
    vec3s center = { { 0.0f, 0.0f, 0.0f } };
    vec3s up = { { 0.0f, 1.0f, 0.0f } };
    mat4s view = glms_lookat(eye, center, up);

    mat4s view_proj = glms_mat4_mul(proj, view);
    mat4s model = glms_mat4_mul(rxm, rym);

    state.model.mvp = glms_mat4_mul(view_proj, model);
}

static void gfx_init(void)
{
    sg_setup(&(sg_desc) {
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });

    sg_buffer vbuf = sg_make_buffer(&(sg_buffer_desc) {
        .data = SG_RANGE(vertices),
        .label = "cube-vertices" });

    sg_buffer ibuf = sg_make_buffer(&(sg_buffer_desc) {
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = SG_RANGE(indices),
        .label = "cube-indices" });

    state.gfx.pip = sg_make_pipeline(&(sg_pipeline_desc) {
        .layout = {
            .buffers[0].stride = 28,
            .attrs = {
                [ATTR_vs_position].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_vs_color0].format = SG_VERTEXFORMAT_FLOAT4,
            },
        },
        .shader = sg_make_shader(cube_shader_desc(sg_query_backend())),
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
}

static void gfx_frame_end(void)
{
    sg_end_pass();
    sg_commit();
}
