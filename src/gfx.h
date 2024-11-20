#ifndef GFX_H_
#define GFX_H_

#include "cglm/struct.h"
#include "sokol_gfx.h"

#define GFX_DISPLAY_WIDTH  (1440)
#define GFX_DISPLAY_HEIGHT (960)

void gfx_init(void);
void gfx_update(void);
void gfx_scale_change(void);
void gfx_shutdown(void);

typedef struct {
    vec3s translation;
    vec3s rotation;
    vec3s scale;
} transform_t;

// model_t represents a renderable model
typedef struct {
    sg_image texture;
    sg_image palette;
    sg_buffer vbuffer;
    sg_bindings bindings;

    transform_t transform;
} model_t;

typedef struct {
    sg_image color_image;
    sg_image depth_image;

    sg_sampler sampler;
    sg_pass_action pass_action;

    struct {
        int scale_divisor;

        sg_pipeline pipeline;
        sg_pass pass;
    } offscreen;

    struct {
        sg_pipeline pipeline;
        sg_bindings bindings;
    } background;

    struct {
        int width;
        int height;

        sg_pipeline pipeline;
        sg_bindings bindings;
    } display;
} gfx_t;

extern gfx_t gfx;

#endif // GFX_H_
