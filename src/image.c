#include "cglm/types-struct.h"

#include "defines.h"
#include "image.h"
#include "memory.h"
#include "model.h"
#include "span.h"

static vec4s _read_rgb15(span_t* span);

image_t image_read_rgb15_image(span_t* span, int width, int height, int offset) {
    const int dims = width * height;
    const int size = dims * 4;
    const int size_on_disk = dims / 2;

    span->offset = offset;

    u8* data = memory_allocate(size);

    usize write_idx = 0;
    for (int i = 0; i < size_on_disk * 2; i++) {
        vec4s c = _read_rgb15(span); // reading 2 at a time
        data[write_idx++] = c.r;
        data[write_idx++] = c.g;
        data[write_idx++] = c.b;
        data[write_idx++] = c.a;
    }

    image_t palette = {
        .width = width,
        .height = height,
        .data = data,
        .size = size,
        .valid = true
    };

    return palette;
}

image_t image_read_4bpp_image(span_t* span, int width, int height, int offset) {
    const int dims = width * height;
    const int size = dims * 4;
    const int size_on_disk = dims / 2;

    span->offset = offset;
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
image_t image_read_map_texture(span_t* span) {
    // Each pixel stored as 1/2 a byte
    constexpr int width = 256;
    constexpr int height = 1024;

    return image_read_4bpp_image(span, width, height, span->offset);
}

image_t image_read_map_palette(span_t* span) {
    constexpr int width = 256;
    constexpr int height = 1;

    u32 intra_file_ptr = span_readat_u32(span, 0x44);
    if (intra_file_ptr == 0) {
        return (image_t) {};
    }

    return image_read_rgb15_image(span, width, height, intra_file_ptr);
}

sg_image image_to_texture(image_t texture) {
    return sg_make_image(&(sg_image_desc) {
        .width = texture.width,
        .height = texture.height,
        .data.subimage[0][0] = {
            .size = texture.size,
            .ptr = texture.data,
        },
        .pixel_format = SG_PIXELFORMAT_RGBA8,
    });
}

void image_destroy(image_t texture) {
    if (texture.data != NULL) {
        memory_free(texture.data);
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
