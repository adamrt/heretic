// https://ffhacktics.com/wiki/EVENT/FRAME.BIN
#include <string.h>

#include "cglm/types-struct.h"

#include "defines.h"
#include "filesystem.h"
#include "memory.h"
#include "span.h"
#include "sprite.h"
#include "texture.h"

static struct {
    // There is some wasted space because space on the palette_idx and cache
    // because only certain files have sprites, but this is easier to manage and
    // reason about. they are only u8 and i32 respectively.

    // current_palette_idx is the current palette index for each file.
    u8 current_palette_idx[F_FILE_COUNT];

    // cache the gpu texture for each file, they will be requested per frame.
    sg_image cache[F_FILE_COUNT];

    // These are the same as above but they are for sprites that have multiple
    // rows, each with thier own palette. We must track the palette index for
    // each row and cache the texture for each row. Currently only used for
    // EVTFACE.BIN, but most likely will be expanded for other files when we
    // find them.
    u8 current_palette_idx_evtface[8][8];
    sg_image cache_evtface[8];
} _state;

typedef struct {
    int tex_width;
    int tex_height;
    int tex_offset;

    int pal_count;
    int pal_offset;
    int pal_default; // Not used yet
} paletted_image_4bpp_desc_t;

const paletted_image_4bpp_desc_t paletted_image_desc_list[] = {
    [F_EVENT__FRAME_BIN] = { 256, 288, 0, 22, 36864, 5 },
    [F_EVENT__ITEM_BIN] = { 256, 256, 0, 16, 32768, 0 },
    [F_EVENT__UNIT_BIN] = { 256, 480, 0, 128, 61440, 0 },
};

static texture_t _read_palette(span_t*, int, usize);
static texture_t _read_sprite_with_palette(span_t*, int, int, int, texture_t, usize);
static texture_t _read_paletted_image_4bpp(span_t*, paletted_image_4bpp_desc_t, int);

void sprite_init(void) {
    // Initialize the palette index to -1 to make them currently invalid
    for (usize i = 0; i < F_FILE_COUNT; i++) {
        _state.current_palette_idx[i] = UINT8_MAX;
    }

    for (usize row = 0; row < 8; row++) {
        for (usize col = 0; col < 8; col++) {
            _state.current_palette_idx_evtface[row][col] = UINT8_MAX;
        }
    }
}

void sprite_shutdown(void) {
    for (usize i = 0; i < F_FILE_COUNT; i++) {
        if (_state.cache[i].id != SG_INVALID_ID) {
            sg_destroy_image(_state.cache[i]);
        }
    }
    for (usize row = 0; row < 8; row++) {
        if (_state.cache_evtface[row].id != SG_INVALID_ID) {
            sg_destroy_image(_state.cache_evtface[row]);
        }
    }
}

sg_image sprite_get_paletted_image(file_entry_e entry, int palette_idx) {
    if (_state.current_palette_idx[entry] == palette_idx) {
        return _state.cache[entry];
    } else {
        _state.current_palette_idx[entry] = palette_idx;
    }

    if (_state.cache[entry].id != SG_INVALID_ID) {
        sg_destroy_image(_state.cache[entry]);
    }

    span_t span = filesystem_read_file(entry);
    texture_t texture = _read_paletted_image_4bpp(&span, paletted_image_desc_list[entry], palette_idx);
    _state.cache[entry] = texture_to_sg_image(texture);
    texture_destroy(texture);

    return _state.cache[entry];
}

//
// EVTFACE.BIN
//

// This reads a single row of portraits. Each row has it's own palette at the end.
// We read them as individual rows so we can apply the palette per row.
static texture_t _read_texture_row_evtface_bin(span_t* span, int row, int palette_idx) {
    constexpr int width = 256;
    constexpr int height = 48;
    constexpr int dims = width * height;
    constexpr int size = dims * 4;

    // Basic dimensions
    constexpr int cols = 8;
    constexpr int portrait_width = 32;
    constexpr int portrait_height = 48;
    constexpr int bytes_per_row = 8192;
    constexpr int bytes_per_portrait = 768;
    constexpr int palette_offset = 6144; // per row

    u8* data = memory_allocate(size);

    u32 pal_offset = row * bytes_per_row + palette_offset;
    texture_t palette = _read_palette(span, 16, pal_offset);

    for (int col = 0; col < cols; col++) {
        int tex_offset = row * bytes_per_row + col * bytes_per_portrait;
        texture_t portrait_texture = _read_sprite_with_palette(span, portrait_width, portrait_height, tex_offset, palette, palette_idx);
        int dest_x = col * portrait_width;

        for (int y = 0; y < portrait_height; y++) {
            int src_index = y * portrait_width * 4;
            int dest_index = y * width * 4 + dest_x * 4;
            memcpy(&data[dest_index], &portrait_texture.data[src_index], portrait_width * 4);
        }
        texture_destroy(portrait_texture);
    }

    texture_destroy(palette);

    texture_t row_texture = {
        .width = width,
        .height = height,
        .data = data,
        .size = size,
        .valid = true
    };
    return row_texture;
}

sg_image sprite_get_evtface_bin(int row_idx, int palette_idx) {
    file_entry_e entry = F_EVENT__EVTFACE_BIN;
    if (_state.current_palette_idx_evtface[row_idx][palette_idx] == palette_idx) {
        return _state.cache_evtface[row_idx];
    } else {
        _state.current_palette_idx_evtface[row_idx][palette_idx] = palette_idx;
    }

    if (_state.cache_evtface[row_idx].id != SG_INVALID_ID) {
        sg_destroy_image(_state.cache_evtface[row_idx]);
    }

    span_t span = filesystem_read_file(entry);
    texture_t texture = _read_texture_row_evtface_bin(&span, row_idx, palette_idx);
    _state.cache_evtface[row_idx] = texture_to_sg_image(texture);
    texture_destroy(texture);

    return _state.cache_evtface[row_idx];
}

//
// Shared functions
//

static texture_t _read_palette(span_t* span, int count, usize offset) {
    constexpr int palette_width = 16; // This is always the case

    const int width = palette_width;
    const int height = count;
    const int dims = palette_width * count;
    const int size = dims * 4;
    const int size_on_disk = dims / 2;

    span->offset = offset;

    u8* data = memory_allocate(size);

    usize write_idx = 0;
    for (int i = 0; i < size_on_disk * 2; i++) {
        vec4s c = read_rgb15(span); // reading 2 at a time
        data[write_idx++] = c.r;
        data[write_idx++] = c.g;
        data[write_idx++] = c.b;
        data[write_idx++] = c.a;
    }

    texture_t palette = {
        .width = width,
        .height = height,
        .data = data,
        .size = size,
        .valid = true
    };

    return palette;
}

static texture_t _read_sprite_with_palette(span_t* span, int width, int height, int offset, texture_t palette, usize palette_idx) {
    const int dims = width * height;
    const int size = dims * 4;
    const int size_on_disk = dims / 2;

    span->offset = offset;
    u8* data = memory_allocate(size);

    usize palette_offset = (16 * 4 * palette_idx); // 16 colors * 4 bytes per color * item_index

    usize write_idx = 0;
    for (int i = 0; i < size_on_disk; i++) {
        u8 raw_pixel = span_read_u8(span);
        u8 right = ((raw_pixel & 0x0F));
        u8 left = ((raw_pixel & 0xF0) >> 4);

        for (int j = 0; j < 4; j++) {
            data[write_idx++] = palette.data[right * 4 + palette_offset + j];
        }
        for (int j = 0; j < 4; j++) {
            data[write_idx++] = palette.data[left * 4 + palette_offset + j];
        }
    }

    texture_t texture = {
        .width = width,
        .height = height,
        .data = data,
        .size = size,
        .valid = true
    };
    return texture;
}

static texture_t _read_paletted_image_4bpp(span_t* span, paletted_image_4bpp_desc_t desc, int pindex) {
    texture_t palette = _read_palette(span, desc.pal_count, desc.pal_offset);
    texture_t texture = _read_sprite_with_palette(span, desc.tex_width, desc.tex_height, desc.tex_offset, palette, pindex);
    texture_destroy(palette);
    return texture;
}
