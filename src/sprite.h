#pragma once

#include <stdbool.h>

#include "sokol_gfx.h"

void sprite_init(void);
void sprite_shutdown(void);

// Paletted
sg_image sprite_get_frame_bin(int);
sg_image sprite_get_item_bin(int);
sg_image sprite_get_unit_bin(int);

// Non-paletted
sg_image sprite_get_evtface_bin(void);
