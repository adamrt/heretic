#ifndef GFX_H_
#define GFX_H_

#include "cglm/types-struct.h"
#include "sokol_gfx.h"

#define GFX_DISPLAY_WIDTH  (256 * 4)
#define GFX_DISPLAY_HEIGHT (240 * 4)

void gfx_init(void);
void gfx_shutdown(void);
void gfx_update(void);
void gfx_scale_change(void);

sg_sampler gfx_get_sampler(void);
int gfx_get_scale_divisor(void);
void gfx_set_scale_divisor(int);

typedef struct {
    vec3s translation;
    vec3s rotation;
    vec3s scale;
} transform_t;

// model_t represents a renderable model
typedef struct {
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

#endif // GFX_H_
