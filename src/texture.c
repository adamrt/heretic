#include "cglm/types-struct.h"

#include "defines.h"
#include "span.h"
#include "texture.h"

texture_t read_texture(span_t* span) {
    // Each pixel stored as 1/2 a byte
    const int TEXTURE_ON_DISK_SIZE = (TEXTURE_SIZE / 2);

    texture_t texture = { 0 };

    u8 bytes[TEXTURE_ON_DISK_SIZE];
    span_read_bytes(span, sizeof(bytes), bytes);

    usize write_idx = 0;
    for (int i = 0; i < TEXTURE_ON_DISK_SIZE; i++) {
        u8 raw_pixel = bytes[i];
        u8 right = raw_pixel & 0x0F;
        u8 left = (raw_pixel & 0xF0) >> 4;

        // Expand left/right nibbles into four entries for RBGA
        for (int j = 0; j < 4; j++) {
            texture.data[write_idx++] = right;
        }
        for (int j = 0; j < 4; j++) {
            texture.data[write_idx++] = left;
        }
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

vec4s read_rgb15(span_t* span) {
    u16 val = span_read_u16(span);

    vec4s color = { 0 };
    color.r = (val & 0x001F) << 3; // 0b0000000000011111
    color.g = (val & 0x03E0) >> 2; // 0b0000001111100000
    color.b = (val & 0x7C00) >> 7; // 0b0111110000000000
    color.a = val == 0 ? 0x00 : 0xFF;
    return color;
}
