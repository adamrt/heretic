// https://ffhacktics.com/wiki/EVENT/FRAME.BIN
#include "sprite.h"
#include "cglm/types-struct.h"
#include "io.h"
#include "span.h"
#include "texture.h"

#include "defines.h"

#define SPRITE_FRAME_SIZE       (SPRITE_FRAME_WIDTH * SPRITE_FRAME_HEIGHT) // 58,368
#define SPRITE_FRAME_BYTE_COUNT (SPRITE_FRAME_SIZE * 4)                    // 233,472

#define SPRITE_FRAME_PALETTE_WIDTH      (16) // 16 colors
#define SPRITE_FRAME_PALETTE_HEIGHT     (22) // 22 palettes
#define SPRITE_FRAME_PALETTE_SIZE       (SPRITE_FRAME_PALETTE_WIDTH * SPRITE_FRAME_PALETTE_HEIGHT)
#define SPRITE_FRAME_PALETTE_BYTE_COUNT (SPRITE_FRAME_PALETTE_SIZE * 4)

typedef struct {
    u8 data[SPRITE_FRAME_BYTE_COUNT];
    bool valid;
} sprite_frame_t;

typedef struct {
    u8 data[SPRITE_FRAME_PALETTE_BYTE_COUNT];
    bool valid;
} sprite_frame_palette_t;

static struct {
    sg_image sprite_frame_image;
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

static sprite_frame_t read_sprite_frame(span_t* span, int frame_index) {
    sprite_frame_t frame = { 0 };

    span->offset = 0;

    const int ON_DISK_SIZE = (SPRITE_FRAME_SIZE / 2); // Each pixel stored as 1/2 a byte

    u8* palette = &_state.frame_palette.data[16 * frame_index];

    for (int i = 0; i < ON_DISK_SIZE * 8; i += 8) {
        u8 raw_pixel = span_read_u8(span);
        u8 right = ((raw_pixel & 0x0F));
        u8 left = ((raw_pixel & 0xF0) >> 4);

        frame.data[i + 0] = palette[right + 0];
        frame.data[i + 1] = palette[right + 1];
        frame.data[i + 2] = palette[right + 2];
        frame.data[i + 3] = palette[right + 3];

        frame.data[i + 4] = palette[left + 0];
        frame.data[i + 5] = palette[left + 1];
        frame.data[i + 6] = palette[left + 2];
        frame.data[i + 7] = palette[left + 3];
    }

    frame.valid = true;
    return frame;
}

sprite_frame_palette_t read_sprite_frame_palette(span_t* span) {
    sprite_frame_palette_t palette = { 0 };

    span->offset = 0x9000;

    for (int i = 0; i < SPRITE_FRAME_PALETTE_BYTE_COUNT; i = i + 4) {
        vec4s c = read_rgb15(span);
        palette.data[i + 0] = c.r;
        palette.data[i + 1] = c.g;
        palette.data[i + 2] = c.b;
        palette.data[i + 3] = c.z;
    }

    palette.valid = true;
    return palette;
}

void sprite_set_frame_palette(int index) {
    sg_destroy_image(_state.sprite_frame_image);

    span_t span = io_file_frame_bin();
    sprite_frame_t frame = read_sprite_frame(&span, index);

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
