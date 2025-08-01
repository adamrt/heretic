#pragma once

#include "sokol_gfx.h"

enum {
    GFX_WINDOW_WIDTH = 1920,
    GFX_WINDOW_HEIGHT = 1280,
    
    GFX_RENDER_WIDTH = 256,
    GFX_RENDER_HEIGHT = 240,
    GFX_RENDER_SCALE = 3
};

void gfx_init(void);
void gfx_shutdown(void);
void gfx_render_begin(void);
void gfx_render_end(void);
void gfx_scale_change(void);

sg_image gfx_get_color_image(void);
sg_sampler gfx_get_sampler(void);
sg_buffer gfx_get_quad_vbuf(void);

sg_face_winding gfx_get_face_winding(void);

int gfx_get_scale_divisor(void);
void gfx_set_scale_divisor(int);
bool* gfx_get_dither(void);

typedef struct {
    bool dither;

    sg_sampler sampler;
    sg_buffer quad_vbuf;
    sg_pipeline pipeline;
    sg_image color_image;
    sg_image depth_image;
    sg_attachments attachments;
} gfx_t;
