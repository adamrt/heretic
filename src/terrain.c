#include "terrain.h"

terrain_t read_terrain(span_t* span) {
    terrain_t terrain = {};

    u32 intra_file_ptr = span_readat_u32(span, 0x68);
    if (intra_file_ptr == 0) {
        return terrain;
    }

    span->offset = intra_file_ptr;

    u8 x_count = span_read_u8(span);
    u8 z_count = span_read_u8(span);

    // Each map contains Z rows of X definitions.
    for (u8 z = 0; z < z_count; z++) {
        for (u8 x = 0; x < x_count; x++) {
            tile_t tile = {};

            tile.surface = (surface_e)span_read_u8(span);
            span_read_u8(span); // Padding
            tile.sloped_height_bottom = span_read_u8(span);
            u8 slope_top_and_depth = span_read_u8(span);
            tile.depth = (slope_top_and_depth >> 5) & 0b00000111;
            tile.sloped_height_top = slope_top_and_depth & 0b00011111;
            tile.slope = (slope_e)span_read_u8(span);
            span_read_u8(span); // Padding

            // Bits 4, 5 and 6 are unknown.
            u8 misc = span_read_u8(span);
            tile.shading = (misc & 0b00001100) >> 2;
            tile.is_selectable = (misc & 0b10000000) >> 7 != 0;
            tile.is_cursorable = (misc & 0b00000010) >> 1 != 0;
            tile.is_walkable = (misc & 0b00000001) >> 0 != 0;

            tile.auto_cam_dir = span_read_u8(span);

            terrain.tiles[z][x] = tile;
        }
    }

    terrain.valid = true;
    return terrain;
}

char* surface_str(surface_e value) {
    switch (value) {
#define X(oname, ovalue, ostring) \
    case oname:                   \
        return ostring;
        SURFACE_INDEX
#undef X
    default:
        return "Unknown";
    }
}
