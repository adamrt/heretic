#pragma once

#include "sokol_gfx.h"

#include "defines.h"

constexpr int FONT_BYTES_PER_CHAR = 35;
constexpr int FONT_BYTES_PER_PIXEL = 4;
constexpr int FONT_CHAR_COUNT = 2200;
constexpr int FONT_CHAR_WIDTH = 10;
constexpr int FONT_CHAR_HEIGHT = 14;
constexpr int FONT_ATLAS_COLS = 56;
constexpr int FONT_ATLAS_ROWS = 40;
constexpr int FONT_ATLAS_WIDTH = FONT_ATLAS_COLS * FONT_CHAR_WIDTH;
constexpr int FONT_ATLAS_HEIGHT = FONT_ATLAS_ROWS * FONT_CHAR_HEIGHT;
constexpr int FONT_ATLAS_BYTE_COUNT = FONT_ATLAS_WIDTH * FONT_ATLAS_HEIGHT * FONT_BYTES_PER_PIXEL;

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
