// https://ffhacktics.com/wiki/EVENT/FRAME.BIN
#include "sprite.h"
#include "cglm/types-struct.h"
#include "io.h"
#include "span.h"
#include "texture.h"

#include "defines.h"

#define FRAME_SIZE        (SPRITE_FRAME_WIDTH * SPRITE_FRAME_HEIGHT) // 58,368
#define FRAME_NBYTES_RGBA (FRAME_SIZE * 4)                           // 233,472
#define FRAME_NBYTES_RAW  (FRAME_SIZE / 2)                           // 29,184

#define FRAME_PALETTE_SIZE        (SPRITE_FRAME_PALETTE_WIDTH * SPRITE_FRAME_PALETTE_HEIGHT) // 352
#define FRAME_PALETTE_NBYTES_RGBA (FRAME_PALETTE_SIZE * 4)                                   // 1,408
#define FRAME_PALETTE_NBYTES_RAW  (FRAME_PALETTE_SIZE / 2)                                   // 176

#define ITEM_SIZE        (SPRITE_ITEM_WIDTH * SPRITE_ITEM_HEIGHT) // 58,368
#define ITEM_NBYTES_RGBA (ITEM_SIZE * 4)                          // 233,472
#define ITEM_NBYTES_RAW  (ITEM_SIZE / 2)                          // 29,184

#define ITEM_PALETTE_SIZE        (SPRITE_ITEM_PALETTE_WIDTH * SPRITE_ITEM_PALETTE_HEIGHT) //
#define ITEM_PALETTE_NBYTES_RGBA (ITEM_PALETTE_SIZE * 4)                                  //
#define ITEM_PALETTE_NBYTES_RAW  (ITEM_PALETTE_SIZE / 2)                                  //

#define EVTFACE_SIZE        (SPRITE_EVTFACE_WIDTH * SPRITE_EVTFACE_HEIGHT) // 256*384 = 98,304
#define EVTFACE_NBYTES_RGBA (EVTFACE_SIZE * 4)                             // 393,216
#define EVTFACE_NBYTES_RAW  (EVTFACE_SIZE / 2)                             // 29,184

typedef struct {
    u8 data[FRAME_NBYTES_RGBA];
    bool valid;
} sprite_frame_t;

typedef struct {
    u8 data[FRAME_PALETTE_NBYTES_RGBA];
    bool valid;
} sprite_frame_palette_t;

typedef struct {
    u8 data[ITEM_NBYTES_RGBA];
    bool valid;
} sprite_item_t;

typedef struct {
    u8 data[ITEM_PALETTE_NBYTES_RGBA];
    bool valid;
} sprite_item_palette_t;

typedef struct {
    u8 data[EVTFACE_NBYTES_RGBA];
    bool valid;
} sprite_evtface_image_t;

static struct {
    sg_image sprite_frame_image;
    sg_image sprite_frame_palette_image;
    sprite_frame_palette_t frame_palette;

    sg_image sprite_item_image;
    sg_image sprite_item_palette_image;
    sprite_item_palette_t item_palette;

    sg_image sprite_evtface_image;
} _state;

static sprite_frame_t read_sprite_frame(span_t*, int);
static sprite_frame_palette_t read_sprite_frame_palette(span_t*);
static sprite_item_t read_sprite_item(span_t*, int);
static sprite_item_palette_t read_sprite_item_palette(span_t*);
static sprite_evtface_image_t read_sprite_evtface(span_t*);

void sprite_init(void) {
    span_t span = io_file_frame_bin();
    _state.frame_palette = read_sprite_frame_palette(&span);
    sprite_set_frame_palette(0);

    span = io_file_item_bin();
    _state.item_palette = read_sprite_item_palette(&span);
    sprite_set_item_palette(0);

    span = io_file_evtface_bin();
    sprite_evtface_image_t evtface = read_sprite_evtface(&span);

    _state.sprite_evtface_image = sg_make_image(&(sg_image_desc) {
        .width = SPRITE_EVTFACE_WIDTH,
        .height = SPRITE_EVTFACE_HEIGHT,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = SG_RANGE(evtface.data),
        .label = "evtface-image",
    });
}

void sprite_shutdown(void) {
    sg_destroy_image(_state.sprite_frame_image);
    sg_destroy_image(_state.sprite_frame_palette_image);
    sg_destroy_image(_state.sprite_item_image);
    sg_destroy_image(_state.sprite_item_palette_image);
    sg_destroy_image(_state.sprite_evtface_image);
}

static sprite_frame_t read_sprite_frame(span_t* span, int palette_idx) {
    sprite_frame_t frame = { 0 };

    span->offset = 0;

    u8* palette = _state.frame_palette.data;
    usize palette_offset = (16 * 4 * palette_idx); // 16 colors * 4 bytes per color * frame_index

    usize write_idx = 0;
    for (int i = 0; i < FRAME_NBYTES_RAW; i++) {
        u8 raw_pixel = span_read_u8(span);
        u8 right = ((raw_pixel & 0x0F));
        u8 left = ((raw_pixel & 0xF0) >> 4);

        for (int j = 0; j < 4; j++) {
            frame.data[write_idx++] = palette[right * 4 + palette_offset + j];
        }
        for (int j = 0; j < 4; j++) {
            frame.data[write_idx++] = palette[left * 4 + palette_offset + j];
        }
    }

    frame.valid = true;
    return frame;
}

sprite_frame_palette_t read_sprite_frame_palette(span_t* span) {
    sprite_frame_palette_t palette = { 0 };

    span->offset = 0x9000;

    usize write_idx = 0;
    for (int i = 0; i < FRAME_PALETTE_NBYTES_RAW * 2; i++) {
        vec4s c = read_rgb15(span); // reading 2 at a time
        palette.data[write_idx++] = c.r;
        palette.data[write_idx++] = c.g;
        palette.data[write_idx++] = c.b;
        palette.data[write_idx++] = c.a;
    }

    palette.valid = true;
    return palette;
}

void sprite_set_frame_palette(int index) {
    sg_destroy_image(_state.sprite_frame_image);

    span_t span = io_file_frame_bin();
    sprite_frame_t frame = read_sprite_frame(&span, index);

    _state.sprite_frame_palette_image = sg_make_image(&(sg_image_desc) {
        .width = SPRITE_FRAME_PALETTE_WIDTH,
        .height = SPRITE_FRAME_PALETTE_HEIGHT,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = SG_RANGE(_state.frame_palette.data),
        .label = "frame-palette",
    });

    _state.sprite_frame_image = sg_make_image(&(sg_image_desc) {
        .width = SPRITE_FRAME_WIDTH,
        .height = SPRITE_FRAME_HEIGHT,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = SG_RANGE(frame.data),
        .label = "frame-image",
    });
}

sg_image sprite_get_frame_image(void) {
    return _state.sprite_frame_image;
}

sg_image sprite_get_frame_palette_image(void) {
    return _state.sprite_frame_palette_image;
}

static sprite_item_t read_sprite_item(span_t* span, int palette_idx) {
    sprite_item_t item = { 0 };

    span->offset = 0;

    u8* palette = _state.item_palette.data;
    usize palette_offset = (16 * 4 * palette_idx); // 16 colors * 4 bytes per color * item_index

    usize write_idx = 0;
    for (int i = 0; i < ITEM_NBYTES_RAW; i++) {
        u8 raw_pixel = span_read_u8(span);
        u8 right = ((raw_pixel & 0x0F));
        u8 left = ((raw_pixel & 0xF0) >> 4);

        for (int j = 0; j < 4; j++) {
            item.data[write_idx++] = palette[right * 4 + palette_offset + j];
        }
        for (int j = 0; j < 4; j++) {
            item.data[write_idx++] = palette[left * 4 + palette_offset + j];
        }
    }

    item.valid = true;
    return item;
}

sprite_item_palette_t read_sprite_item_palette(span_t* span) {
    sprite_item_palette_t palette = { 0 };

    span->offset = 32768;

    usize write_idx = 0;
    for (int i = 0; i < ITEM_PALETTE_NBYTES_RAW * 2; i++) {
        vec4s c = read_rgb15(span); // reading 2 at a time
        palette.data[write_idx++] = c.r;
        palette.data[write_idx++] = c.g;
        palette.data[write_idx++] = c.b;
        palette.data[write_idx++] = c.a;
    }

    palette.valid = true;
    return palette;
}

void sprite_set_item_palette(int index) {
    sg_destroy_image(_state.sprite_item_image);

    span_t span = io_file_item_bin();
    sprite_item_t item = read_sprite_item(&span, index);

    _state.sprite_item_palette_image = sg_make_image(&(sg_image_desc) {
        .width = SPRITE_ITEM_PALETTE_WIDTH,
        .height = SPRITE_ITEM_PALETTE_HEIGHT,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = SG_RANGE(_state.item_palette.data),
        .label = "item-palette",
    });

    _state.sprite_item_image = sg_make_image(&(sg_image_desc) {
        .width = SPRITE_ITEM_WIDTH,
        .height = SPRITE_ITEM_HEIGHT,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = SG_RANGE(item.data),
        .label = "item-image",
    });
}

sg_image sprite_get_item_image(void) {
    return _state.sprite_item_image;
}

sg_image sprite_get_item_palette_image(void) {
    return _state.sprite_item_palette_image;
}
static sprite_evtface_image_t read_sprite_evtface(span_t* span) {
    sprite_evtface_image_t evtface = { 0 };

    // Basic dimensions
    const int rows_of_portraits = 8;
    const int cols_of_portraits = 8;
    const int portrait_width = 32;
    const int portrait_height = 48;
    const int image_width = 256;
    const int bytes_per_row = 8192;
    const int palette_offset = 6144; // per row

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

                    int right_idx = (absolute_y * image_width + absolute_x1) * 4;
                    evtface.data[right_idx + 0] = right_color.r;
                    evtface.data[right_idx + 1] = right_color.g;
                    evtface.data[right_idx + 2] = right_color.b;
                    evtface.data[right_idx + 3] = right_color.a;

                    int left_idx = (absolute_y * image_width + absolute_x2) * 4;
                    evtface.data[left_idx + 0] = left_color.r;
                    evtface.data[left_idx + 1] = left_color.g;
                    evtface.data[left_idx + 2] = left_color.b;
                    evtface.data[left_idx + 3] = left_color.a;
                }
            }
        }
    }

    evtface.valid = true;
    return evtface;
}

sg_image sprite_get_evtface_image(void) {
    return _state.sprite_evtface_image;
}
