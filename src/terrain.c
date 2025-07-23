#include "terrain.h"
#include "util.h"
#include <string.h>

terrain_t read_terrain(span_t* span) {
    terrain_t terrain = { 0 };

    u32 intra_file_ptr = span_readat_u32(span, 0x68);
    if (intra_file_ptr == 0) {
        return terrain;
    }

    span->offset = intra_file_ptr;

    u8 x_count = span_read_u8(span);
    u8 z_count = span_read_u8(span);
    ASSERT(x_count <= TERRAIN_X_MAX, "Terrain X count exceeded");
    ASSERT(z_count <= TERRAIN_Z_MAX, "Terrain Z count exceeded");

    for (u8 level = 0; level < 2; level++) {
        for (u8 z = 0; z < z_count; z++) {
            for (u8 x = 0; x < x_count; x++) {
                // FIXME: This code needs to be validated that it is reading everything correctly.
                tile_t tile = { 0 };

                u8 raw_surface = span_read_u8(span);
                tile.surface = (surface_e)(raw_surface & 0x3F); // 0b00111111
                span_read_u8(span);
                tile.sloped_height_bottom = span_read_u8(span);
                u8 slope_top_and_depth = span_read_u8(span);
                tile.depth = (slope_top_and_depth >> 5) & 0x07;      // 0b11100000 -> 0b00000111
                tile.sloped_height_top = slope_top_and_depth & 0x1F; // 0b00011111
                tile.slope = (slope_e)span_read_u8(span);
                span_read_u8(span); // Padding

                if (tile.slope == SLOPE_FLAT) {
                    // Sloped height top should be 0 for flat tiles but some
                    // maps tiles set to 1. This should be researched further.
                    ASSERT(tile.sloped_height_top == 0 || tile.sloped_height_top == 1, "Flat tile has > 1 sloped height top");
                }

                // bits 3, 4, 5, are unused
                u8 misc = span_read_u8(span);
                tile.pass_through_only = misc & (1 << 0); // bit 0
                tile.shading = (misc >> 2) & 0x3;         // bit 1 & 2
                tile.cant_walk = misc & (1 << 6);         // bit 6
                tile.cant_select = misc & (1 << 7);       // bit 7
                tile.auto_cam_dir = span_read_u8(span);

                terrain.tiles[level][z * x_count + x] = tile;
            }
        }
    }

    terrain.x_count = x_count;
    terrain.z_count = z_count;
    terrain.valid = true;
    return terrain;
}

const char* terrain_surface_str(surface_e value) {
    switch (value) {
#define X(oname, ovalue, ostring) \
    case oname:                   \
        return ostring;
        SURFACE_INDEX
#undef X
    default:;
        static char buf[32];
        snprintf(buf, sizeof(buf), "Unknown 0x%02X", value);
        return buf;
    }
}

const char* terrain_slope_str(slope_e value) {
    switch (value) {
#define X(oname, ovalue, ostring) \
    case oname:                   \
        return ostring;
        SLOPE_INDEX
#undef X
    default:;
        static char buf[32];
        snprintf(buf, sizeof(buf), "Unknown 0x%02X", value);
        return buf;
    }
}

const char* terrain_shading_str(u8 value) {
    switch (value) {
    case 0:
        return "Normal";
    case 1:
        return "Dark";
    case 2:
        return "Darker";
    case 3:
        return "Darkest";
    default:
        return "Unknown";
    }
}

void terrain_camdir_str(u8 cam_dir, char out_str[static TERRAIN_STR_SIZE]) {
    out_str[0] = '\0';

    static const char* labels[8] = {
        "NWT", "SWT", "SET", "NET",
        "NWB", "SWB", "SEB", "NEB"
    };

    size_t offset = 0;
    int written = 0;
    bool first = true;

    for (int i = 0; i < 8; i++) {
        if (cam_dir & (1 << i)) {
            if (!first) {
                written = snprintf(out_str + offset, TERRAIN_STR_SIZE - offset, ", ");
                offset += (written > 0) ? written : 0;
            }

            written = snprintf(out_str + offset, TERRAIN_STR_SIZE - offset, "%s", labels[i]);
            offset += (written > 0) ? written : 0;

            first = false;
        }
    }

    if (offset == 0) {
        snprintf(out_str, TERRAIN_STR_SIZE, "(None)");
    }
}
