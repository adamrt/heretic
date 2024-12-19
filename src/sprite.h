#pragma once

#include <stdbool.h>

#include "sokol_gfx.h"

#define SPRITE_FRAME_WIDTH  (256)
#define SPRITE_FRAME_HEIGHT (228)

void sprite_init(void);
void sprite_shutdown(void);
sg_image sprite_get_frame_image(void);
void sprite_set_frame_palette(int index);
