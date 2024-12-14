#include "cglm/types-struct.h"

#include "defines.h"
#include "texture.h"

static vec4s read_rgb15(span_t*);

texture_t read_texture(span_t* span) {
    const int TEXTURE_ON_DISK_SIZE = (TEXTURE_SIZE / 2); // Each pixel stored as 1/2 a byte

    texture_t texture = { 0 };

    for (int i = 0; i < TEXTURE_ON_DISK_SIZE * 8; i += 8) {
        u8 raw_pixel = span_read_u8(span);
        u8 right = ((raw_pixel & 0x0F));
        u8 left = ((raw_pixel & 0xF0) >> 4);
        texture.data[i + 0] = right;
        texture.data[i + 1] = right;
        texture.data[i + 2] = right;
        texture.data[i + 3] = right;
        texture.data[i + 4] = left;
        texture.data[i + 5] = left;
        texture.data[i + 6] = left;
        texture.data[i + 7] = left;
    }

    texture.valid = true;
    return texture;
}

palette_t read_palette(span_t* span) {
    palette_t palette = { 0 };

    u32 intra_file_ptr = span_readat_u32(span, 0x44);
    if (intra_file_ptr == 0) {
        return palette;
    }

    span->offset = intra_file_ptr;

    for (int i = 0; i < 16 * 16 * 4; i = i + 4) {
        vec4s c = read_rgb15(span);
        palette.data[i + 0] = c.x;
        palette.data[i + 1] = c.y;
        palette.data[i + 2] = c.z;
        palette.data[i + 3] = c.w;
    }

    palette.valid = true;
    return palette;
}

static vec4s read_rgb15(span_t* span) {
    u16 val = span_read_u16(span);

    vec4s color = { 0 };
    color.r = (val & 0x001F) << 3; // 0b0000000000011111
    color.g = (val & 0x03E0) >> 2; // 0b0000001111100000
    color.b = (val & 0x7C00) >> 7; // 0b0111110000000000
    color.a = val == 0 ? 0x00 : 0xFF;
    return color;
}
