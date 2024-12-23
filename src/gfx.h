#pragma once

#include "cglm/types-struct.h"
#include "lighting.h"
#include "map.h"
#include "sokol_gfx.h"

#define GFX_WINDOW_WIDTH  (1920)
#define GFX_WINDOW_HEIGHT (1280)

#define GFX_RENDER_WIDTH  (256 * 2)
#define GFX_RENDER_HEIGHT (240 * 2)

typedef struct {
    vec3s translation;
    vec3s rotation;
    vec3s scale;
} transform_t;

// model_t represents a renderable model
typedef struct {
    sg_buffer vbuf;
    sg_buffer ibuf;
    sg_image texture;
    sg_image grayscale;
    sg_image palette;
    transform_t transform;
    int vertex_count;
    vec3s center;
} model_t;

void gfx_init(void);
void gfx_shutdown(void);
void gfx_render_begin(void);
void gfx_render_model(model_t*, lighting_t*);
void gfx_render_end(void);
void gfx_scale_change(void);
model_t gfx_map_to_model(map_t*);

sg_image gfx_get_color_image(void);

sg_sampler gfx_get_sampler(void);
sg_buffer gfx_get_quad_vbuf(void);
sg_buffer gfx_get_quad_ibuf(void);

int gfx_get_scale_divisor(void);
void gfx_set_scale_divisor(int);

typedef struct {
    sg_sampler sampler;
    sg_buffer quad_vbuf;
    sg_buffer quad_ibuf;
    sg_pipeline pipeline;
    sg_image color_image;
    sg_image depth_image;
    sg_attachments attachments;
} gfx_t;
