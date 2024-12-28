// https://ffhacktics.com/wiki/EVENT/FRAME.BIN
#include "sprite.h"
#include "cglm/types-struct.h"
#include "io.h"
#include "memory.h"
#include "span.h"
#include "texture.h"

#include "defines.h"

static struct {
    sg_image sprite_frame;
    sg_image sprite_item;
    sg_image sprite_unit;
    sg_image sprite_evtface;

    int frame_palette_idx;
    int item_palette_idx;
    int unit_palette_idx;
} _state;

// Reusable functions for reading sprites and their palettes.
static texture_t _read_palette(span_t*, const int, const int, const usize);
static texture_t _read_sprite_with_palette(span_t*, const int, const int, const texture_t, const usize);

void sprite_init(void) {
    // Set to invalid palettes so we can load them on first request
    _state.frame_palette_idx = 999;
    _state.item_palette_idx = 999;
    _state.unit_palette_idx = 999;
}

void sprite_shutdown(void) {
    sg_destroy_image(_state.sprite_frame);
    sg_destroy_image(_state.sprite_item);
    sg_destroy_image(_state.sprite_unit);
    sg_destroy_image(_state.sprite_evtface);
}

//
// FRAME.BIN
//

static texture_t _read_frame_bin(span_t* span, int palette_idx) {
    constexpr int pal_width = 16;
    constexpr int pal_height = 22;
    constexpr int pal_offset = 36864;
    texture_t palette = _read_palette(span, pal_width, pal_height, pal_offset);

    constexpr int width = 256;
    constexpr int height = 288;
    texture_t texture = _read_sprite_with_palette(span, width, height, palette, palette_idx);

    texture_destroy(palette);
    return texture;
}

sg_image sprite_get_frame_bin(int palette_idx) {
    if (_state.frame_palette_idx == palette_idx) {
        return _state.sprite_frame;
    }

    if (_state.sprite_frame.id != SG_INVALID_ID) {
        sg_destroy_image(_state.sprite_frame);
    }

    span_t span = io_file_frame_bin();
    texture_t texture = _read_frame_bin(&span, palette_idx);
    _state.sprite_frame = texture_to_sg_image(texture);
    texture_destroy(texture);

    _state.frame_palette_idx = palette_idx;

    return _state.sprite_frame;
}

//
// ITEM.BIN
//

static texture_t _read_texture_item_bin(span_t* span, int palette_idx) {
    constexpr int pal_width = 16;
    constexpr int pal_height = 16;
    constexpr int pal_offset = 32768;
    texture_t palette = _read_palette(span, pal_width, pal_height, pal_offset);

    constexpr int tex_width = 256;
    constexpr int tex_height = 256;
    texture_t texture = _read_sprite_with_palette(span, tex_width, tex_height, palette, palette_idx);

    texture_destroy(palette);
    return texture;
}

sg_image sprite_get_item_bin(int palette_idx) {
    if (_state.item_palette_idx == palette_idx) {
        return _state.sprite_item;
    }

    if (_state.sprite_item.id != SG_INVALID_ID) {
        sg_destroy_image(_state.sprite_item);
    }

    span_t span = io_file_item_bin();
    texture_t texture = _read_texture_item_bin(&span, palette_idx);
    _state.sprite_item = texture_to_sg_image(texture);
    texture_destroy(texture);

    _state.item_palette_idx = palette_idx;

    return _state.sprite_item;
}

//
// UNIT.BIN
//

static texture_t _read_texture_unit_bin(span_t* span, int palette_idx) {
    constexpr int pal_width = 16;
    constexpr int pal_height = 128;
    constexpr int pal_offset = 61440;
    texture_t palette = _read_palette(span, pal_width, pal_height, pal_offset);

    constexpr int tex_width = 256;
    constexpr int tex_height = 480;
    texture_t texture = _read_sprite_with_palette(span, tex_width, tex_height, palette, palette_idx);

    texture_destroy(palette);
    return texture;
}

sg_image sprite_get_unit_bin(int palette_idx) {
    if (_state.unit_palette_idx == palette_idx) {
        return _state.sprite_unit;
    }

    if (_state.sprite_unit.id != SG_INVALID_ID) {
        sg_destroy_image(_state.sprite_unit);
    }

    span_t span = io_file_unit_bin();
    texture_t texture = _read_texture_unit_bin(&span, palette_idx);
    _state.sprite_unit = texture_to_sg_image(texture);
    texture_destroy(texture);

    _state.unit_palette_idx = palette_idx;

    return _state.sprite_unit;
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
    if (_state.sprite_evtface.id != SG_INVALID_ID) {
        return _state.sprite_evtface;
    }

    span_t span = io_file_evtface_bin();
    texture_t texture = _read_texture_evtface_bin(&span);
    _state.sprite_evtface = texture_to_sg_image(texture);

    return _state.sprite_evtface;
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
