#pragma once

#include "lighting.h"
#include "sokol_gfx.h"

#include "cglm/types-struct.h"
#include "texture.h"

typedef struct {
    vec3s translation;
    vec3s rotation;
    vec3s scale;
} transform_t;

// model_t represents a renderable model
typedef struct {
    sg_buffer vbuf;
    sg_buffer ibuf;

    texture_t texture;
    texture_t palette;

    lighting_t lighting;

    transform_t transform;
    int vertex_count;
    vec3s center;
} model_t;

void model_destroy(model_t);
mat4s model_matrix(transform_t);
