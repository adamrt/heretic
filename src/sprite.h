#pragma once

#include <stdbool.h>

#include "filesystem.h"
#include "sokol_gfx.h"
#include "texture.h"

void sprite_init(void);
void sprite_shutdown(void);

texture_t sprite_get_paletted_texture(file_entry_e, int);
texture_t sprite_get_evtface_bin_texture(int, int);
