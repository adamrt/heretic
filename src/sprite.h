#pragma once

#include <stdbool.h>

#include "filesystem.h"
#include "sokol_gfx.h"

void sprite_init(void);
void sprite_shutdown(void);

sg_image sprite_get_paletted_image(file_entry_e, int);
sg_image sprite_get_evtface_bin(void);
