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
    sg_image sprite_evtface;
    sg_image sprite_unit;

    int frame_palette_idx;
    int item_palette_idx;
    int unit_palette_idx;
} _state;

static texture_t sprite_read_palette(span_t*, const int, const int, const usize);
static texture_t sprite_read_sprite_with_palette(span_t*, const int, const int, const texture_t, const usize);
static texture_t read_sprite_frame(span_t*, int);
static texture_t read_sprite_item(span_t*, int);
static texture_t read_sprite_unit(span_t*, int);
static texture_t read_sprite_evtface(span_t*);

void sprite_init(void) {
    _state.frame_palette_idx = 0;
    _state.item_palette_idx = 0;
    _state.unit_palette_idx = 0;

    span_t span = io_file_evtface_bin();
    texture_t evtface = read_sprite_evtface(&span);
    _state.sprite_evtface = texture_to_sg_image(evtface);
    texture_destroy(evtface);

    span = io_file_frame_bin();
    texture_t frame = read_sprite_frame(&span, _state.frame_palette_idx);
    _state.sprite_frame = texture_to_sg_image(frame);
    texture_destroy(frame);

    span = io_file_item_bin();
    texture_t item = read_sprite_item(&span, _state.item_palette_idx);
    _state.sprite_item = texture_to_sg_image(item);
    texture_destroy(item);

    span = io_file_unit_bin();
    texture_t unit = read_sprite_unit(&span, _state.unit_palette_idx);
    _state.sprite_unit = texture_to_sg_image(unit);
    texture_destroy(unit);
}

void sprite_shutdown(void) {
    sg_destroy_image(_state.sprite_frame);
    sg_destroy_image(_state.sprite_item);
    sg_destroy_image(_state.sprite_unit);
    sg_destroy_image(_state.sprite_evtface);
}

texture_t read_sprite_item_palette(span_t* span) {
    const int width = 16;
    const int height = 16;
    const int offset = 32768;
    return sprite_read_palette(span, width, height, offset);
}

static texture_t read_sprite_item(span_t* span, int palette_idx) {
    const int width = 256;
    const int height = 256;
    texture_t palette = read_sprite_item_palette(span);
    texture_t texture = sprite_read_sprite_with_palette(span, width, height, palette, palette_idx);
    texture_destroy(palette);
    return texture;
}

texture_t read_sprite_frame_palette(span_t* span) {
    const int width = 16;
    const int height = 22;
    const int offset = 0x9000;
    return sprite_read_palette(span, width, height, offset);
}

texture_t read_sprite_frame(span_t* span, int palette_idx) {
    const int width = 256;
    const int height = 288;
    texture_t palette = read_sprite_frame_palette(span);
    texture_t texture = sprite_read_sprite_with_palette(span, width, height, palette, palette_idx);
    texture_destroy(palette);
    return texture;
}

texture_t read_sprite_unit_palette(span_t* span) {
    const int width = 16;
    const int height = 128;
    const int offset = 61440;
    return sprite_read_palette(span, width, height, offset);
}

texture_t read_sprite_unit(span_t* span, int palette_idx) {
    const int width = 256;
    const int height = 480;
    texture_t palette = read_sprite_unit_palette(span);
    texture_t texture = sprite_read_sprite_with_palette(span, width, height, palette, palette_idx);
    texture_destroy(palette);
    return texture;
}

sg_image sprite_get_frame_image(int palette_idx) {
    if (_state.frame_palette_idx != palette_idx) {
        if (_state.sprite_frame.id != SG_INVALID_ID) {
            sg_destroy_image(_state.sprite_frame);
        }
        span_t span = io_file_frame_bin();
        texture_t frame = read_sprite_frame(&span, palette_idx);
        _state.sprite_frame = texture_to_sg_image(frame);
        texture_destroy(frame);
        _state.frame_palette_idx = palette_idx;
    }
    return _state.sprite_frame;
}

sg_image sprite_get_item_image(int palette_idx) {
    if (_state.item_palette_idx != palette_idx) {
        if (_state.sprite_item.id != SG_INVALID_ID) {
            sg_destroy_image(_state.sprite_item);
        }
        span_t span = io_file_item_bin();
        texture_t item = read_sprite_item(&span, palette_idx);
        _state.sprite_item = texture_to_sg_image(item);
        texture_destroy(item);
        _state.item_palette_idx = palette_idx;
    }
    return _state.sprite_item;
}

sg_image sprite_get_unit_image(int palette_idx) {
    if (_state.unit_palette_idx != palette_idx) {
        if (_state.sprite_unit.id != SG_INVALID_ID) {
            sg_destroy_image(_state.sprite_unit);
        }
        span_t span = io_file_unit_bin();
        texture_t unit = read_sprite_unit(&span, palette_idx);
        _state.sprite_unit = texture_to_sg_image(unit);
        texture_destroy(unit);
        _state.unit_palette_idx = palette_idx;
    }
    return _state.sprite_unit;
}

static texture_t sprite_read_palette(span_t* span, const int width, const int height, const usize offset) {
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

texture_t sprite_read_sprite_with_palette(span_t* span, const int width, const int height, const texture_t palette, const usize palette_idx) {
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

static texture_t read_sprite_evtface(span_t* span) {
    const int width = 256;
    const int height = 384;
    const int dims = width * height;
    const int size = dims * 4;

    // Basic dimensions
    const int rows_of_portraits = 8;
    const int cols_of_portraits = 8;
    const int portrait_width = 32;
    const int portrait_height = 48;
    const int bytes_per_row = 8192;
    const int palette_offset = 6144; // per row

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

sg_image sprite_get_evtface_image(void) {
    return _state.sprite_evtface;
}
