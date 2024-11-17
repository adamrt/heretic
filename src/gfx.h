#ifndef GFX_H_
#define GFX_H_

#include "sokol_gfx.h"

#define GFX_DISPLAY_WIDTH  (1440)
#define GFX_DISPLAY_HEIGHT (960)

void gfx_init(void);
void gfx_update(void);
void gfx_scale_change(void);
void gfx_shutdown(void);

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
