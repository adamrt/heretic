#include "gfx_model.h"
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#include "gfx.h"
#include "gfx_background.h"
#include "gfx_line.h"
#include "gfx_sprite.h"
#include "gui.h"
#include "shape.h"

// Global gfx state
static gfx_t _state;

// There are two passes so we can render the offscreen image to a fullscreen
// quad. The offscreen is rendered in a lower resolution and then upscaled to
// the window size to keep the pixelated look.
void gfx_init(void) {
    _state.dither = true;

    sg_setup(&(sg_desc) {
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });

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

    gfx_model_init();
    gfx_sprite_init();
    gfx_background_init();
    gfx_line_init();
}

void gfx_render_begin(void) {
    sg_begin_pass(&(sg_pass) {
        .attachments = _state.attachments,
        .action = {
            .colors[0] = (sg_color_attachment_action) {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = { 0.0f, 0.0f, 0.0f, 1.0f },
            },
            .depth = {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = 0.0f, // Clear depth to 0.0 because Z+ into screen
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

void gfx_shutdown(void) {
    gfx_model_shutdown();
    gfx_sprite_shutdown();
    gfx_background_shutdown();
    gfx_line_shutdown();

    sg_destroy_attachments(_state.attachments);
    sg_destroy_image(_state.color_image);
    sg_destroy_image(_state.depth_image);
    sg_destroy_sampler(_state.sampler);
    sg_destroy_buffer(_state.quad_vbuf);
    sg_shutdown();
}

sg_image gfx_get_color_image(void) { return _state.color_image; }
sg_sampler gfx_get_sampler(void) { return _state.sampler; }
sg_buffer gfx_get_quad_vbuf(void) { return _state.quad_vbuf; }

// API specific winding order. This is required because Metal's coordinate system
// has flipped Y compared to OpenGL, and when we use @msl_options flip_vert_y
// to normalize this, it reverses the triangle winding order from CCW to CW.
sg_face_winding gfx_get_face_winding(void) {
    sg_backend backend = sg_query_backend();
    // Return the appropriate face winding order based on the backend
    switch (backend) {
    case SG_BACKEND_METAL_MACOS:
    case SG_BACKEND_METAL_IOS:
        return SG_FACEWINDING_CW;
    case SG_BACKEND_GLCORE:
    case SG_BACKEND_GLES3:
    default:
        return SG_FACEWINDING_CCW;
    }
}

bool* gfx_get_dither(void) {
    return &_state.dither;
}
