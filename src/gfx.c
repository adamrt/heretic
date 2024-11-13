#include "cglm/struct.h"

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#include "shader.glsl.h"

#include "camera.h"
#include "game.h"
#include "gfx.h"
#include "model.h"
#include "ui.h"

// Global gfx state
gfx_t gfx;

#define GFX_SCALE            (2)
#define GFX_OFFSCREEN_WIDTH  (GFX_DISPLAY_WIDTH / GFX_SCALE)
#define GFX_OFFSCREEN_HEIGHT (GFX_DISPLAY_HEIGHT / GFX_SCALE)

// Forward declarations
static void init_shared(void);
static void init_offscreen(void);
static void init_background(void);
static void init_display(void);

static void frame_offscreen(void);
static void frame_background(void);
static void frame_display(void);

// There are two passes so we can render the offscreen image to a fullscreen
// quad. The offscreen is rendered in a lower resolution and then upscaled to
// the window size to keep the pixelated look.
void gfx_init(void)
{
    sg_setup(&(sg_desc) {
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });

    init_shared();
    init_offscreen();
    init_background();
    init_display();
}

void gfx_update(void)
{
    // Render the scene and background to an offscreen image
    sg_begin_pass(&gfx.offscreen.pass);
    {
        frame_offscreen();
        frame_background();
    }
    sg_end_pass();

    // Display the offscreen image to a fullscreen quad and render the UI
    sg_begin_pass(&(sg_pass) {
        .action = gfx.default_pass_action,
        .swapchain = sglue_swapchain(),
        .label = "swapchain-pass",
    });
    {
        frame_display();
        ui_update();
    }
    sg_end_pass();

    sg_commit();
}

void gfx_shutdown(void)
{
    sg_destroy_pipeline(gfx.offscreen.pipeline);
    sg_destroy_pipeline(gfx.background.pipeline);
    sg_destroy_pipeline(gfx.display.pipeline);

    sg_destroy_image(gfx.color_image);
    sg_destroy_image(gfx.depth_image);

    sg_destroy_sampler(gfx.default_sampler);

    sg_shutdown();
}

static void init_shared(void)
{
    gfx.color_image = sg_make_image(&(sg_image_desc) {
        .render_target = true,
        .width = GFX_OFFSCREEN_WIDTH,
        .height = GFX_OFFSCREEN_HEIGHT,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .label = "color-image",
    });

    gfx.depth_image = sg_make_image(&(sg_image_desc) {
        .render_target = true,
        .width = GFX_OFFSCREEN_WIDTH,
        .height = GFX_OFFSCREEN_HEIGHT,
        .pixel_format = SG_PIXELFORMAT_DEPTH,
        .label = "depth-image",
    });

    gfx.default_sampler = sg_make_sampler(&(sg_sampler_desc) {
        .min_filter = SG_FILTER_NEAREST,
        .mag_filter = SG_FILTER_NEAREST,
        .wrap_u = SG_WRAP_REPEAT,
        .wrap_v = SG_WRAP_REPEAT,
    });

    gfx.default_pass_action = (sg_pass_action) {
        .colors[0] = (sg_color_attachment_action) {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = { 0.0f, 0.0f, 0.0f, 1.0f },
        },
    };
}

static void init_offscreen(void)
{
    gfx.offscreen.pass = (sg_pass) {
        .attachments = sg_make_attachments(&(sg_attachments_desc) {
            .colors[0].image = gfx.color_image,
            .depth_stencil.image = gfx.depth_image,
            .label = "offscreen-attachments",
        }),
        .action = gfx.default_pass_action,
        .label = "offscreen-pass",
    };

    gfx.offscreen.pipeline = sg_make_pipeline(&(sg_pipeline_desc) {
        .layout = {
            .attrs = {
                [ATTR_standard_vs_a_position].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_standard_vs_a_normal].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_standard_vs_a_uv].format = SG_VERTEXFORMAT_FLOAT2,
                [ATTR_standard_vs_a_palette_index].format = SG_VERTEXFORMAT_FLOAT,
                [ATTR_standard_vs_a_is_textured].format = SG_VERTEXFORMAT_FLOAT,
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

static void init_background(void)
{
    gfx.background.pipeline = sg_make_pipeline(&(sg_pipeline_desc) {

        .layout = {
            .buffers[0].stride = sizeof(vertex_t),
            .attrs = {
                [ATTR_background_vs_a_position].format = SG_VERTEXFORMAT_FLOAT3,
            },
        },
        .shader = sg_make_shader(background_shader_desc(sg_query_backend())),
        .index_type = SG_INDEXTYPE_UINT16,
        .cull_mode = SG_CULLMODE_NONE,
        .depth = {
            .pixel_format = SG_PIXELFORMAT_DEPTH,
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true,
        },
        .colors[0].pixel_format = SG_PIXELFORMAT_RGBA8,
        .label = "background-pipeline",
    });

    // Fullscreen quad vertices, moved back a bit
    // clang-format off
    vertex_t quad_vertices[] = {
        { .position = {{ -1.0f, -1.0f, 0.5f }}, .uv = {{ 0.0f, 1.0f }} }, // bottom-left
        { .position = {{  1.0f, -1.0f, 0.5f }}, .uv = {{ 1.0f, 1.0f }} }, // bottom-right
        { .position = {{ -1.0f,  1.0f, 0.5f }}, .uv = {{ 0.0f, 0.0f }} }, // top-left
        { .position = {{  1.0f,  1.0f, 0.5f }}, .uv = {{ 1.0f, 0.0f }} }  // top-right
    };

    uint16_t quad_indices[] = { 0, 1, 2, 1, 3, 2 };
    // clang-format on

    sg_buffer quad_vbuf = sg_make_buffer(&(sg_buffer_desc) {
        .data = SG_RANGE(quad_vertices),
        .label = "background-vertices",
    });

    sg_buffer quad_ibuf = sg_make_buffer(&(sg_buffer_desc) {
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = SG_RANGE(quad_indices),
        .label = "background-indices",
    });

    gfx.background.bindings = (sg_bindings) {
        .vertex_buffers[0] = quad_vbuf,
        .index_buffer = quad_ibuf,
    };
}

static void init_display(void)
{
    gfx.display.pipeline = sg_make_pipeline(&(sg_pipeline_desc) {
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

    sg_buffer quad_vbuf = sg_make_buffer(&(sg_buffer_desc) {
        .data = SG_RANGE(quad_vertices),
        .label = "quad-vertices",
    });

    sg_buffer quad_ibuf = sg_make_buffer(&(sg_buffer_desc) {
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = SG_RANGE(quad_indices),
        .label = "quad-indices",
    });

    gfx.display.bindings = (sg_bindings) {
        .vertex_buffers[0] = quad_vbuf,
        .index_buffer = quad_ibuf,
        .fs = {
            .images[SLOT_tex] = gfx.color_image,
            .samplers[SLOT_smp] = gfx.default_sampler,
        }
    };
}

static void frame_offscreen(void)
{
    model_t* model = &game.scene.model;

    mat4s proj = camera_get_proj();
    mat4s view = camera_get_view();

    vs_standard_params_t vs_params = {
        .u_proj = proj,
        .u_view = view,
        .u_model = model->model_matrix,
    };

    fs_standard_params_t fs_params;
    fs_params.u_ambient_color = model->mesh.lighting.ambient_color;
    fs_params.u_ambient_strength = model->mesh.lighting.ambient_strength;

    int light_count = 0;
    for (int i = 0; i < MESH_MAX_LIGHTS; i++) {

        light_t light = model->mesh.lighting.lights[i];
        if (!light.valid) {
            continue;
        }

        fs_params.u_light_colors[light_count] = light.color;
        fs_params.u_light_directions[light_count] = glms_vec4(light.direction, 1.0f);
        light_count++;
    }
    fs_params.u_light_count = light_count;

    sg_apply_pipeline(gfx.offscreen.pipeline);
    sg_apply_bindings(&model->bindings);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_standard_params, &SG_RANGE(vs_params));
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_standard_params, &SG_RANGE(fs_params));
    sg_draw(0, model->mesh.geometry.count, 1);
}

static void frame_background(void)
{

    sg_apply_pipeline(gfx.background.pipeline);

    fs_background_params_t fs_params;
    fs_params.u_top_color = game.scene.model.mesh.lighting.bg_top;
    fs_params.u_bottom_color = game.scene.model.mesh.lighting.bg_bottom;

    sg_apply_pipeline(gfx.background.pipeline);
    sg_apply_bindings(&gfx.background.bindings);
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_background_params, &SG_RANGE(fs_params));
    sg_draw(0, 6, 1);
}

static void frame_display(void)
{
    sg_apply_pipeline(gfx.display.pipeline);
    sg_apply_bindings(&gfx.display.bindings);
    sg_draw(0, 6, 1);
}
