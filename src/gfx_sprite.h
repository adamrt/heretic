#pragma once

#include <stdbool.h>

#include "cglm/types-struct.h"

#include "filesystem.h"
#include "sokol_gfx.h"
#include "texture.h"
#include "transform.h"

typedef enum {
    SPRITE_2D,
    SPRITE_3D
} sprite_type_e;

typedef struct {
    sprite_type_e type;
    texture_t texture;
    vec2s uv_min;
    vec2s uv_max;
    transform_t transform;
} sprite_t;

void gfx_sprite_init(void);
void gfx_sprite_shutdown(void);
void gfx_sprite_reset(void);
void gfx_sprite_render(void);
sprite_t gfx_sprite_create(sprite_type_e, texture_t, vec2s, vec2s, transform_t);

sprite_t* gfx_sprite_get_internals(void);
texture_t sprite_get_paletted_texture(file_entry_e, int);
