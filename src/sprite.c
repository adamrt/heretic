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

typedef struct {
    u8 data[FRAME_NBYTES_RGBA];
    bool valid;
} sprite_frame_t;

typedef struct {
    u8 data[FRAME_PALETTE_NBYTES_RGBA];
    bool valid;
} sprite_frame_palette_t;

static struct {
    sg_image sprite_frame_image;
    sg_image sprite_frame_palette_image;
    sprite_frame_palette_t frame_palette;
} _state;

static sprite_frame_t read_sprite_frame(span_t*, int);
static sprite_frame_palette_t read_sprite_frame_palette(span_t*);

void sprite_init(void) {
    span_t span = io_file_frame_bin();
    _state.frame_palette = read_sprite_frame_palette(&span);
    sprite_set_frame_palette(0);
}

void sprite_shutdown(void) {
    sg_destroy_image(_state.sprite_frame_image);
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
        .label = "font-palette",
    });

    _state.sprite_frame_image = sg_make_image(&(sg_image_desc) {
        .width = SPRITE_FRAME_WIDTH,
        .height = SPRITE_FRAME_HEIGHT,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = SG_RANGE(frame.data),
        .label = "font-atlas",
    });
}

sg_image sprite_get_frame_image(void) {
    return _state.sprite_frame_image;
}

sg_image sprite_get_frame_palette_image(void) {
    return _state.sprite_frame_palette_image;
}
