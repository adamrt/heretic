#include "cglm/types-struct.h"

#include "defines.h"
#include "memory.h"
#include "model.h"
#include "span.h"
#include "texture.h"

texture_t read_texture(span_t* span) {
    // Each pixel stored as 1/2 a byte
    const int width = 256;
    const int height = 1024;
    const int dims = width * height;
    const int size = dims * 4;
    const int size_on_disk = (dims / 2);

    u8* data = memory_allocate(size);

    u8 bytes[size_on_disk];
    span_read_bytes(span, sizeof(bytes), bytes);

    usize write_idx = 0;
    for (int i = 0; i < size_on_disk; i++) {
        u8 raw_pixel = bytes[i];
        u8 right = raw_pixel & 0x0F;
        u8 left = (raw_pixel & 0xF0) >> 4;

        // Expand left/right nibbles into four entries for RBGA
        for (int j = 0; j < 4; j++) {
            data[write_idx++] = right;
        }
        for (int j = 0; j < 4; j++) {
            data[write_idx++] = left;
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

texture_t read_palette(span_t* span) {
    const int width = 256;
    const int height = 1;
    const int dims = width * height;
    const int size = dims * 4;

    u32 intra_file_ptr = span_readat_u32(span, 0x44);
    if (intra_file_ptr == 0) {
        return (texture_t) { 0 };
    }
    span->offset = intra_file_ptr;

    u8* data = memory_allocate(size);

    for (int i = 0; i < 16 * 16 * 4; i = i + 4) {
        vec4s c = read_rgb15(span);
        data[i + 0] = c.r;
        data[i + 1] = c.g;
        data[i + 2] = c.b;
        data[i + 3] = c.a;
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

vec4s read_rgb15(span_t* span) {
    u16 val = span_read_u16(span);

    vec4s color = { 0 };
    color.r = (val & 0x001F) << 3; // 0b0000000000011111
    color.g = (val & 0x03E0) >> 2; // 0b0000001111100000
    color.b = (val & 0x7C00) >> 7; // 0b0111110000000000
    color.a = val == 0 ? 0x00 : 0xFF;
    return color;
}

void texture_destroy(texture_t texture) {
    if (texture.data != NULL) {
        memory_free(texture.data);
    }
}

sg_image texture_to_sg_image(texture_t texture) {
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
