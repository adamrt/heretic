#pragma once

#include <stdbool.h>

#include "sokol_gfx.h"

void sprite_init(void);
void sprite_shutdown(void);

sg_image sprite_get_frame_image(int palette_id);
sg_image sprite_get_item_image(int palette_id);
sg_image sprite_get_evtface_image(void);
sg_image sprite_get_unit_image(int palette_id);
