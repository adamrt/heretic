#pragma once

#include <stdbool.h>

#include "cglm/types-struct.h"
#include "filesystem.h"
#include "model.h"
#include "sokol_gfx.h"
#include "texture.h"

typedef enum {
    SPRITE_TYPE_2D,
    SPRITE_TYPE_3D,
} sprite_type_e;

typedef struct {
    vec3s scale;
    vec2s screen_pos;
} transform_2d_t;

typedef struct {
    texture_t texture;
    vec2s uv_min;
    vec2s uv_max;
    transform_t transform_3d;
    transform_2d_t transform_2d;
    sprite_type_e type;
} sprite_t;

void sprite_init(void);
void sprite_shutdown(void);
sprite_t sprite_create_3d(texture_t, vec2s, vec2s, transform_t);
sprite_t sprite_create_2d(texture_t, vec2s, vec2s, f32, f32, f32);
void sprite_render_3d(const sprite_t*);
void sprite_render_2d(const sprite_t*);

// FIXME: These should be moved to another module.
// resource.h? sprite_resource.h? sprite_loader.h?
texture_t sprite_get_paletted_texture(file_entry_e, int);
texture_t sprite_get_evtface_bin_texture(int, int);
