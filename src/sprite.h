#pragma once

#include <stdbool.h>

#include "sokol_gfx.h"

#define SPRITE_FRAME_WIDTH          (256)
#define SPRITE_FRAME_HEIGHT         (288)
#define SPRITE_FRAME_PALETTE_WIDTH  (16) // 16 colors
#define SPRITE_FRAME_PALETTE_HEIGHT (22) // 22 palettes

#define SPRITE_ITEM_WIDTH          (256)
#define SPRITE_ITEM_HEIGHT         (256)
#define SPRITE_ITEM_PALETTE_WIDTH  (16) // 16 colors
#define SPRITE_ITEM_PALETTE_HEIGHT (16) // 16 palettes

void sprite_init(void);
void sprite_shutdown(void);

void sprite_set_frame_palette(int);
sg_image sprite_get_frame_image(void);
sg_image sprite_get_frame_palette_image(void);

void sprite_set_item_palette(int);
sg_image sprite_get_item_image(void);
sg_image sprite_get_item_palette_image(void);
