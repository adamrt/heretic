#pragma once

#include <stdbool.h>

#include "sokol_gfx.h"

#include "defines.h"

#define SPRITE_FRAME_WIDTH      (256)
#define SPRITE_FRAME_HEIGHT     (228)
#define SPRITE_FRAME_BYTE_COUNT (SPRITE_FRAME_WIDTH * SPRITE_FRAME_HEIGHT * 4)

typedef struct {
    u8 data[SPRITE_FRAME_BYTE_COUNT];
    bool valid;
} sprite_frame_t;

void sprite_init(void);
void sprite_shutdown(void);
sg_image sprite_get_frame_image(void);
