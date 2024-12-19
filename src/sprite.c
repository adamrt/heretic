#include "sprite.h"
#include "io.h"
#include "span.h"

static struct {
    sg_image sprite_frame_image;
} _state;

#define SPRITE_FRAME_SIZE      (SPRITE_FRAME_WIDTH * SPRITE_FRAME_HEIGHT) // 58368
#define SPRITE_FRAME_BYTE_SIZE (SPRITE_FRAME_SIZE * 4)

static sprite_frame_t read_sprite_frame(span_t*);

void sprite_init(void) {
    span_t span = io_file_frame_bin();
    sprite_frame_t frame = read_sprite_frame(&span);

    _state.sprite_frame_image = sg_make_image(&(sg_image_desc) {
        .width = SPRITE_FRAME_WIDTH,
        .height = SPRITE_FRAME_HEIGHT,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = SG_RANGE(frame.data),
        .label = "font-atlas",
    });
}

void sprite_shutdown(void) {
    sg_destroy_image(_state.sprite_frame_image);
}

static sprite_frame_t read_sprite_frame(span_t* span) {
    sprite_frame_t frame = { 0 };

    const int ON_DISK_SIZE = (SPRITE_FRAME_SIZE / 2); // Each pixel stored as 1/2 a byte

    for (int i = 0; i < ON_DISK_SIZE * 8; i += 8) {
        u8 raw_pixel = span_read_u8(span);
        u8 right = ((raw_pixel & 0x0F));
        u8 left = ((raw_pixel & 0xF0) >> 4);
        frame.data[i + 0] = right;
        frame.data[i + 1] = right;
        frame.data[i + 2] = right;
        frame.data[i + 3] = right;
        frame.data[i + 4] = left;
        frame.data[i + 5] = left;
        frame.data[i + 6] = left;
        frame.data[i + 7] = left;
    }

    frame.valid = true;
    return frame;
}

sg_image sprite_get_frame_image(void) {
    return _state.sprite_frame_image;
}
