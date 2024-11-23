#include "texture.h"

static vec4s read_rgb15(file_t*);

texture_t read_texture(file_t* f)
{
    const int TEXTURE_ON_DISK_SIZE = (TEXTURE_SIZE / 2); // Each pixel stored as 1/2 a byte

    texture_t texture = { 0 };

    for (int i = 0; i < TEXTURE_ON_DISK_SIZE * 8; i += 8) {
        uint8_t raw_pixel = read_u8(f);
        uint8_t right = ((raw_pixel & 0x0F));
        uint8_t left = ((raw_pixel & 0xF0) >> 4);
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

palette_t read_palette(file_t* f)
{
    palette_t palette = { 0 };

    f->offset = 0x44;
    uint32_t intra_file_ptr = read_u32(f);
    if (intra_file_ptr == 0) {
        return palette;
    }

    f->offset = intra_file_ptr;

    for (int i = 0; i < 16 * 16 * 4; i = i + 4) {
        vec4s c = read_rgb15(f);
        palette.data[i + 0] = c.x;
        palette.data[i + 1] = c.y;
        palette.data[i + 2] = c.z;
        palette.data[i + 3] = c.w;
    }

    palette.valid = true;
    return palette;
}

static vec4s read_rgb15(file_t* f)
{
    uint16_t val = read_u16(f);
    uint8_t a = val == 0 ? 0x00 : 0xFF;
    uint8_t blue = (val & 0x7C00) >> 7;  // 0b0111110000000000
    uint8_t green = (val & 0x03E0) >> 2; // 0b0000001111100000
    uint8_t red = (val & 0x001F) << 3;   // 0b0000000000011111
    return (vec4s) { { red, green, blue, a } };
}
