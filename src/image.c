#include "cglm/types-struct.h"

#include "defines.h"
#include "image.h"
#include "memory.h"
#include "span.h"

static vec4s _read_rgb15(span_t* span);

image_t image_read_rgb15(span_t* span, int width, int height) {
    const int dims = width * height;
    const int size = dims * 4;
    const int size_on_disk = dims / 2;

    u8* data = memory_allocate(size);

    usize write_idx = 0;
    for (int i = 0; i < size_on_disk * 2; i++) {
        vec4s c = _read_rgb15(span); // reading 2 bytes at a time
        data[write_idx++] = c.r;
        data[write_idx++] = c.g;
        data[write_idx++] = c.b;
        data[write_idx++] = c.a;
    }

    image_t image = {};
    image.width = width;
    image.height = height;
    image.data = data;
    image.size = size;
    image.valid = true;

    return image;
}

image_t image_read_4bpp(span_t* span, int width, int height) {
    const int dims = width * height;
    const int size = dims * 4;
    const int size_on_disk = dims / 2;

    u8* data = memory_allocate(size);

    usize write_idx = 0;
    for (int i = 0; i < size_on_disk; i++) {
        u8 raw_pixel = span_read_u8(span);
        u8 right = ((raw_pixel & 0x0F));
        u8 left = ((raw_pixel & 0xF0) >> 4);

        for (int j = 0; j < 4; j++) {
            data[write_idx++] = right;
        }
        for (int j = 0; j < 4; j++) {
            data[write_idx++] = left;
        }
    }

    image_t image = {
        .width = width,
        .height = height,
        .data = data,
        .size = size,
        .valid = true
    };
    return image;
}

void image_destroy(image_t image) {
    if (image.data != NULL) {
        memory_free(image.data);
    }
}

vec4s _read_rgb15(span_t* span) {
    u16 val = span_read_u16(span);

    vec4s color = {};
    color.r = (val & 0b0000000000011111) << 3;
    color.g = (val & 0b0000001111100000) >> 2;
    color.b = (val & 0b0111110000000000) >> 7;
    color.a = (val == 0) ? 0x00 : 0xFF;
    return color;
}
