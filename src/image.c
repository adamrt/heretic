#include <string.h>

#include "defines.h"
#include "image.h"
#include "memory.h"
#include "span.h"

constexpr int pal_col_count = 16;
constexpr int pal_row_size = pal_col_count * 4; // 4 bytes per color

image_t image_read_palette(span_t* span, int rows) {
    return image_read_16bpp(span, pal_col_count, rows);
}

image_t image_read_4bpp(span_t* span, int width, int height) {
    const int dims = width * height;
    const int size = dims * 4;
    const int size_on_disk = dims / 2; // two pixels per byte

    u8* data = memory_allocate(size);

    usize write_idx = 0;
    for (int i = 0; i < size_on_disk; i++) {
        u8 raw_pixel = span_read_u8(span);

        u8 right = (raw_pixel & 0b00001111);
        u8 left = (raw_pixel & 0b11110000) >> 4;

        for (int j = 0; j < 4; j++) {
            data[write_idx++] = right;
        }

        for (int j = 0; j < 4; j++) {
            data[write_idx++] = left;
        }
    }

    return (image_t) {
        .width = width,
        .height = height,
        .data = data,
        .size = size,
        .valid = true
    };
}

image_t image_read_16bpp(span_t* span, int width, int height) {
    const int dims = width * height;
    const int size = dims * 4;

    u8* data = memory_allocate(size);

    usize write_idx = 0;
    for (int i = 0; i < dims; i++) {
        u16 val = span_read_u16(span);
        data[write_idx++] = (val & 0b0000000000011111) << 3;
        data[write_idx++] = (val & 0b0000001111100000) >> 2;
        data[write_idx++] = (val & 0b0111110000000000) >> 7;
        data[write_idx++] = (val == 0) ? 0x00 : 0xFF;
    }

    return (image_t) {
        .width = width,
        .height = height,
        .data = data,
        .size = size,
        .valid = true,
    };
}

// Read and return an image using the palette provided.
// The image's rgba values are the index of the palette.
image_t image_read_4bpp_pal(span_t* span, int width, int height, image_t palette, usize pal_idx) {
    const int dims = width * height;
    const int pal_offset = (pal_row_size * pal_idx);

    image_t image = image_read_4bpp(span, width, height);

    for (int i = 0; i < dims * 4; i = i + 4) {
        u8 pixel = image.data[i];
        memcpy(&image.data[i], &palette.data[pal_offset + (pixel * 4)], 4);
    }

    return image;
}

void image_destroy(image_t image) {
    if (image.data != NULL) {
        memory_free(image.data);
    }
}
