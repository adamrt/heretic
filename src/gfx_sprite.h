#pragma once

#include <stdbool.h>

#include "cglm/types-struct.h"

#include "filesystem.h"
#include "sokol_gfx.h"
#include "texture.h"
#include "transform.h"

typedef struct {
    texture_t texture;
    vec2s uv_min;
    vec2s uv_max;
    transform_t transform;
} sprite2d_t;

typedef struct {
    texture_t texture;
    vec2s uv_min;
    vec2s uv_max;
    transform_t transform;
} sprite3d_t;

void gfx_sprite_init(void);
void gfx_sprite_shutdown(void);
void gfx_sprite_reset(void);
void gfx_sprite_render(void);
sprite2d_t gfx_sprite2d_create(texture_t, vec2s, vec2s, f32, f32, f32);
sprite3d_t gfx_sprite3d_create(texture_t, vec2s, vec2s, transform_t);

sprite3d_t* gfx_sprite3d_get_internals(void);
sprite2d_t* gfx_sprite2d_get_internals(void);
texture_t sprite_get_paletted_texture(file_entry_e, int);
