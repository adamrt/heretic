#pragma once

#include <stdint.h>

#include "sokol_gfx.h"

#include "defines.h"

#define FONT_CHAR_COUNT       (2200)
#define FONT_CHAR_WIDTH       (10)
#define FONT_CHAR_HEIGHT      (14)
#define FONT_ATLAS_COLS       (56)
#define FONT_ATLAS_ROWS       (40)
#define FONT_ATLAS_WIDTH      (FONT_ATLAS_COLS * FONT_CHAR_WIDTH)                      // 56 * 10 = 560
#define FONT_ATLAS_HEIGHT     (FONT_ATLAS_ROWS * FONT_CHAR_HEIGHT)                     // 40 * 14 = 560
#define FONT_ATLAS_BYTE_COUNT (FONT_ATLAS_WIDTH * FONT_ATLAS_HEIGHT * BYTES_PER_PIXEL) // 560 * 560 * 4 = 1,574,400

#define BYTES_PER_CHAR  (35)
#define BYTES_PER_PIXEL (4)

typedef struct {
    u16 id;
    const char* data;
} font_char_t;

typedef struct {
    u8 data[FONT_ATLAS_BYTE_COUNT];
    bool valid;
} font_atlas_t;

void font_init(void);
void font_shutdown(void);
sg_image font_get_atlas_image(void);
const char* font_get_char(u16 id);
