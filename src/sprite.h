#pragma once

#include <stdbool.h>

#include "cglm/types-struct.h"
#include "filesystem.h"
#include "model.h"
#include "sokol_gfx.h"
#include "texture.h"

typedef struct {
    texture_t texture;
    vec2s uv_min;
    vec2s uv_max;
    transform_t transform;
} sprite_t;

void sprite_init(void);
void sprite_shutdown(void);
sprite_t sprite_create(texture_t, vec2s, vec2s, transform_t);
void sprite_render(const sprite_t*);

// FIXME: These should be moved to another module.
// resource.h? sprite_resource.h? sprite_loader.h?
texture_t sprite_get_paletted_texture(file_entry_e, int);
texture_t sprite_get_evtface_bin_texture(int, int);
