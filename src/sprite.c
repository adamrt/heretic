// https://ffhacktics.com/wiki/EVENT/FRAME.BIN
#include "sprite.h"
#include "cglm/types-struct.h"
#include "cimgui.h"
#include "filesystem.h"
#include "memory.h"
#include "span.h"
#include "texture.h"

#include "defines.h"

static struct {
    // There is some wasted space because space on the palette_idx and cache
    // because only certain files have sprites, but this is easier to manage and
    // reason about. they are only u8 and i32 respectively.

    // current_palette_idx is the current palette index for each file.
    u8 current_palette_idx[F_FILE_COUNT];

    // cache the gpu texture for each file, they will be requested per frame.
    sg_image cache[F_FILE_COUNT];
} _state;

typedef struct {
    int tex_width;
    int tex_height;
    int pal_width;
    int pal_height;
    int pal_offset;
} paletted_image_4bpp_desc_t;

const paletted_image_4bpp_desc_t paletted_image_desc_list[] = {
    [F_EVENT__FRAME_BIN] = { 256, 288, 16, 22, 36864 },
    [F_EVENT__ITEM_BIN] = { 256, 256, 16, 16, 32768 },
    [F_EVENT__UNIT_BIN] = { 256, 480, 16, 128, 61440 },
};

static texture_t _read_paletted_image_4bpp(span_t*, paletted_image_4bpp_desc_t, int);

void sprite_init(void) {
    // Initialize the palette index to -1 to make them currently invalid
    for (usize i = 0; i < F_FILE_COUNT; i++) {
        _state.current_palette_idx[i] = UINT8_MAX;
    }
}

void sprite_shutdown(void) {
    for (usize i = 0; i < F_FILE_COUNT; i++) {
        if (_state.cache[i].id != SG_INVALID_ID) {
            sg_destroy_image(_state.cache[i]);
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

static texture_t _read_texture_evtface_bin(span_t* span) {
    constexpr int width = 256;
    constexpr int height = 384;
    constexpr int dims = width * height;
    constexpr int size = dims * 4;

    // Basic dimensions
    constexpr int rows_of_portraits = 8;
    constexpr int cols_of_portraits = 8;
    constexpr int portrait_width = 32;
    constexpr int portrait_height = 48;
    constexpr int bytes_per_row = 8192;
    constexpr int palette_offset = 6144; // per row

    u8* data = memory_allocate(size);

    for (int portrait_row = 0; portrait_row < rows_of_portraits; portrait_row++) {

        // Jump to the palette offset of the new row and read it
        span->offset = portrait_row * bytes_per_row + palette_offset;
        vec4s palette[16];
        for (int i = 0; i < 16; i++) {
            palette[i] = read_rgb15(span);
        }

        for (int portrait_col = 0; portrait_col < cols_of_portraits; portrait_col++) {
            span->offset = portrait_row * bytes_per_row + portrait_col * 768; // Jump to the portrait offset of the new row

            // The top-left of this portrait
            int base_x = portrait_col * portrait_width;
            int base_y = portrait_row * portrait_height;

            for (int py = 0; py < portrait_height; py++) {
                for (int px = 0; px < (portrait_width / 2); px++) {
                    u8 raw_byte = span_read_u8(span);
                    u8 right = (raw_byte & 0x0F);
                    u8 left = (raw_byte & 0xF0) >> 4;

                    // Actual X positions in the image
                    int absolute_x1 = base_x + (px * 2);
                    int absolute_x2 = absolute_x1 + 1;
                    int absolute_y = base_y + py;

                    // Lookup color from the local palette
                    vec4s right_color = palette[right];
                    vec4s left_color = palette[left];

                    int right_idx = (absolute_y * width + absolute_x1) * 4;
                    data[right_idx + 0] = right_color.r;
                    data[right_idx + 1] = right_color.g;
                    data[right_idx + 2] = right_color.b;
                    data[right_idx + 3] = right_color.a;

                    int left_idx = (absolute_y * width + absolute_x2) * 4;
                    data[left_idx + 0] = left_color.r;
                    data[left_idx + 1] = left_color.g;
                    data[left_idx + 2] = left_color.b;
                    data[left_idx + 3] = left_color.a;
                }
            }
        }
    }

    texture_t evtface = {
        .width = width,
        .height = height,
        .data = data,
        .size = size,
        .valid = true
    };
    return evtface;
}

sg_image sprite_get_evtface_bin(void) {
    file_entry_e entry = F_EVENT__EVTFACE_BIN;

    if (_state.cache[entry].id != SG_INVALID_ID) {
        return _state.cache[entry];
    }

    span_t span = filesystem_read_file(entry);
    texture_t texture = _read_texture_evtface_bin(&span);
    _state.cache[entry] = texture_to_sg_image(texture);

    return _state.cache[entry];
}

//
// Shared functions
//

static texture_t _read_palette(span_t* span, const int width, const int height, const usize offset) {
    const int dims = width * height;
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

static texture_t _read_sprite_with_palette(span_t* span, const int width, const int height, const texture_t palette, const usize palette_idx) {
    const int dims = width * height;
    const int size = dims * 4;
    const int size_on_disk = dims / 2;

    span->offset = 0;
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
    texture_t palette = _read_palette(span, desc.pal_width, desc.pal_height, desc.pal_offset);
    texture_t texture = _read_sprite_with_palette(span, desc.tex_width, desc.tex_height, palette, pindex);
    texture_destroy(palette);
    return texture;
}
