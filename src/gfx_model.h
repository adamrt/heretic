#pragma once

#include "cglm/types-struct.h"
#include "sokol_gfx.h"

#include "lighting.h"
#include "texture.h"
#include "transform.h"

// model_t represents a renderable model
typedef struct {
    sg_buffer vbuf;
    sg_buffer ibuf;

    texture_t texture;
    texture_t palette;

    lighting_t lighting;

    transform3d_t transform;
    int vertex_count;
    vec3s center;
} model_t;

void gfx_model_init(void);
void gfx_model_shutdown(void);
void gfx_model_render(const model_t*);
void gfx_model_destroy(model_t);
