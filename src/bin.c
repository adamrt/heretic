#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "bin.h"
#include "model.h"

static uint8_t bin_u8(file_t* f)
{
    uint8_t value;
    memcpy(&value, &f->data[f->offset], sizeof(uint8_t));
    f->offset += sizeof(uint8_t);
    return value;
}

static uint16_t bin_u16(file_t* f)
{
    uint16_t value;
    memcpy(&value, &f->data[f->offset], sizeof(uint16_t));
    f->offset += sizeof(uint16_t);
    return value;
}

static uint32_t bin_u32(file_t* f)
{
    uint32_t value;
    memcpy(&value, &f->data[f->offset], sizeof(uint32_t));
    f->offset += sizeof(uint32_t);
    return value;
}

static int8_t bin_i8(file_t* f)
{
    int8_t value;
    memcpy(&value, &f->data[f->offset], sizeof(int8_t));
    f->offset += sizeof(int8_t);
    return value;
}

static int16_t bin_i16(file_t* f)
{
    int16_t value;
    memcpy(&value, &f->data[f->offset], sizeof(int16_t));
    f->offset += sizeof(int16_t);
    return value;
}

static int32_t bin_i32(file_t* f)
{
    int32_t value;
    memcpy(&value, &f->data[f->offset], sizeof(int32_t));
    f->offset += sizeof(int32_t);
    return value;
}

bytes_t bin_bytes(file_t* f, int size)
{
    bytes_t bytes = { .size = size };
    for (int i = 0; i < size; i++) {
        bytes.data[i] = bin_u8(f);
    }
    return bytes;
}

// bin_read_sector reads a sector to `out_bytes`.
static sector_t bin_sector(FILE* file, int32_t sector_num)
{
    int32_t seek_to = (sector_num * BIN_SECTOR_SIZE_RAW) + BIN_SECTOR_HEADER_SIZE;
    if (fseek(file, seek_to, SEEK_SET) != 0) {
        assert(false);
    }

    sector_t sector;
    size_t n = fread(sector.data, sizeof(uint8_t), BIN_SECTOR_SIZE, file);
    assert(n == BIN_SECTOR_SIZE);
    return sector;
}

static file_t bin_file(FILE* bin, int sector_num, int size)
{
    file_t file = { .size = size };

    int offset = 0;
    uint32_t occupied_sectors = ceil((float)size / (float)BIN_SECTOR_SIZE);

    for (uint32_t i = 0; i < occupied_sectors; i++) {
        sector_t sector_data = bin_sector(bin, sector_num + i);

        int remaining_size = size - offset;
        int bytes_to_copy = (remaining_size < BIN_SECTOR_SIZE) ? remaining_size : BIN_SECTOR_SIZE;

        memcpy(file.data + offset, sector_data.data, bytes_to_copy);
        offset += bytes_to_copy;
    }

    return file;
}

static float bin_f1x3x12(file_t* f)
{
    float value = bin_i16(f);
    return value / 4096.0f;
}

static vec3s bin_position(file_t* f)
{
    int16_t x = bin_i16(f);
    int16_t y = bin_i16(f);
    int16_t z = bin_i16(f);

    y = -y;
    z = -z;

    return (vec3s) { { x, y, z } };
}

static vec3s bin_normal(file_t* f)
{
    float x = bin_f1x3x12(f);
    float y = bin_f1x3x12(f);
    float z = bin_f1x3x12(f);

    y = -y;
    z = -z;

    return (vec3s) { { x, y, z } };
}

static vec3s bin_rgb8(file_t* f)
{
    uint8_t r = bin_u8(f);
    uint8_t g = bin_u8(f);
    uint8_t b = bin_u8(f);
    return (vec3s) { { r, g, b } };
}

static vec4s bin_rgb15(file_t* f)
{
    uint16_t val = bin_u16(f);
    uint8_t a = val == 0 ? 0x00 : 0xFF;
    uint8_t b = (val & 0x7C00) >> 7; // 0b0111110000000000
    uint8_t g = (val & 0x03E0) >> 2; // 0b0000001111100000
    uint8_t r = (val & 0x001F) << 3; // 0b0000000000011111
    return (vec4s) { { r, g, b, a } };
}

static record_t bin_record(file_t* f)
{
    bytes_t bytes = bin_bytes(f, BIN_GNS_RECORD_SIZE);
    int sector = bytes.data[8] | bytes.data[9] << 8;
    uint64_t length = (uint32_t)(bytes.data[12]) | ((uint32_t)(bytes.data[13]) << 8) | ((uint32_t)(bytes.data[14]) << 16) | ((uint32_t)(bytes.data[15]) << 24);
    file_type_e type = (bytes.data[4] | (bytes.data[5] << 8));
    time_e time = (bytes.data[3] >> 7) & 0x1;
    weather_e weather = (bytes.data[3] >> 4) & 0x7;
    int arrangement = bytes.data[2];

    record_t record = {
        .sector = sector,
        .length = length,
        .type = type,
        .time = time,
        .weather = weather,
        .arrangement = arrangement,
    };
    memcpy(record.data, bytes.data, BIN_GNS_RECORD_SIZE);
    return record;
}

static records_t bin_records(file_t* f)
{
    records_t records = { .count = 0 };
    while (true) {
        record_t record = bin_record(f);
        if (record.type == FILE_TYPE_END) {
            break;
        }
        records.records[records.count] = record;
        records.count++;
    }
    return records;
}

// 16 palettes of 16 colors of 4 bytes
// process_tex_coords has two functions:
//
// 1. Update the v coordinate to the specific page of the texture. FFT
//    Textures have 4 pages (256x1024) and the original V specifies
//    the pixel on one of the 4 pages. Multiply the page by the height
//    of a single page (256).
// 2. Normalize the coordinates that can be U:0-255 and V:0-1023. Just
//    divide them by their max to get a 0.0-1.0 value.
static vec2s process_tex_coords(float u, float v, uint8_t page)
{
    u = u / 255.0f;
    v = (v + (page * 256)) / 1023.0f;
    return (vec2s) { { u, v } };
}

static geometry_t bin_geometry(file_t* f)
{
    geometry_t geometry = { 0 };

    // 0x40 is always the location of the primary mesh pointer.
    // 0xC4 is always the primary mesh pointer.
    f->offset = 0x40;
    f->offset = bin_u32(f);

    // The number of each type of polygon.
    int N = bin_u16(f); // Textured triangles
    int P = bin_u16(f); // Textured quads
    int Q = bin_u16(f); // Untextured triangles
    int R = bin_u16(f); // Untextured quads

    // Validate maximum values
    assert(N < 512 && P < 768 && Q < 64 && R < 256);

    // Textured triangle
    for (int i = 0; i < N; i++) {
        geometry.vertices[geometry.count++].position = bin_position(f);
        geometry.vertices[geometry.count++].position = bin_position(f);
        geometry.vertices[geometry.count++].position = bin_position(f);
    }

    // Textured quads
    for (int i = 0; i < P; i++) {
        vec3s a = bin_position(f);
        vec3s b = bin_position(f);
        vec3s c = bin_position(f);
        vec3s d = bin_position(f);

        // Tri A
        geometry.vertices[geometry.count++].position = a;
        geometry.vertices[geometry.count++].position = b;
        geometry.vertices[geometry.count++].position = c;

        // Tri B
        geometry.vertices[geometry.count++].position = b;
        geometry.vertices[geometry.count++].position = d;
        geometry.vertices[geometry.count++].position = c;
    }

    // Untextured triangle
    for (int i = 0; i < Q; i++) {
        geometry.vertices[geometry.count++].position = bin_position(f);
        geometry.vertices[geometry.count++].position = bin_position(f);
        geometry.vertices[geometry.count++].position = bin_position(f);
    }

    // Untextured quads
    for (int i = 0; i < R; i++) {
        vec3s a = bin_position(f);
        vec3s b = bin_position(f);
        vec3s c = bin_position(f);
        vec3s d = bin_position(f);

        // Tri A
        geometry.vertices[geometry.count++].position = a;
        geometry.vertices[geometry.count++].position = b;
        geometry.vertices[geometry.count++].position = c;

        // Tri B
        geometry.vertices[geometry.count++].position = b;
        geometry.vertices[geometry.count++].position = d;
        geometry.vertices[geometry.count++].position = c;
    }

    // Triangle normals
    for (int i = 0; i < N * 3; i = i + 3) {
        geometry.vertices[i + 0].normal = bin_normal(f);
        geometry.vertices[i + 1].normal = bin_normal(f);
        geometry.vertices[i + 2].normal = bin_normal(f);
    };

    // Quad normals
    for (int i = N * 3; i < N * 3 + (P * 3 * 2); i = i + 6) {
        vec3s a = bin_normal(f);
        vec3s b = bin_normal(f);
        vec3s c = bin_normal(f);
        vec3s d = bin_normal(f);

        // Tri A
        geometry.vertices[i + 0].normal = a;
        geometry.vertices[i + 1].normal = b;
        geometry.vertices[i + 2].normal = c;

        // Tri B
        geometry.vertices[i + 3].normal = b;
        geometry.vertices[i + 4].normal = d;
        geometry.vertices[i + 5].normal = c;
    };

    // Triangle UV
    for (int i = 0; i < N * 3; i = i + 3) {
        float au = bin_u8(f);
        float av = bin_u8(f);
        float palette = bin_u8(f);
        (void)bin_u8(f); // padding
        float bu = bin_u8(f);
        float bv = bin_u8(f);
        float page = (bin_u8(f) & 0x03); // 0b00000011
        (void)bin_u8(f);                 // padding
        float cu = bin_u8(f);
        float cv = bin_u8(f);

        vec2s a = process_tex_coords(au, av, page);
        vec2s b = process_tex_coords(bu, bv, page);
        vec2s c = process_tex_coords(cu, cv, page);

        geometry.vertices[i + 0].uv = a;
        geometry.vertices[i + 0].palette_index = palette;
        geometry.vertices[i + 1].uv = b;
        geometry.vertices[i + 1].palette_index = palette;
        geometry.vertices[i + 2].uv = c;
        geometry.vertices[i + 2].palette_index = palette;
    }

    // Quad UV. Split into 2 triangles.
    for (int i = N * 3; i < N * 3 + (P * 3 * 2); i = i + 6) {
        float au = bin_u8(f);
        float av = bin_u8(f);
        float palette = bin_u8(f);
        (void)bin_u8(f); // padding
        float bu = bin_u8(f);
        float bv = bin_u8(f);
        float page = (bin_u8(f) & 0x03); // 0b00000011
        (void)bin_u8(f);                 // padding
        float cu = bin_u8(f);
        float cv = bin_u8(f);
        float du = bin_u8(f);
        float dv = bin_u8(f);

        vec2s a = process_tex_coords(au, av, page);
        vec2s b = process_tex_coords(bu, bv, page);
        vec2s c = process_tex_coords(cu, cv, page);
        vec2s d = process_tex_coords(du, dv, page);

        // Triangle A
        geometry.vertices[i + 0].uv = a;
        geometry.vertices[i + 0].palette_index = palette;
        geometry.vertices[i + 1].uv = b;
        geometry.vertices[i + 1].palette_index = palette;
        geometry.vertices[i + 2].uv = c;
        geometry.vertices[i + 2].palette_index = palette;

        // Triangle B
        geometry.vertices[i + 3].uv = b;
        geometry.vertices[i + 3].palette_index = palette;
        geometry.vertices[i + 4].uv = d;
        geometry.vertices[i + 4].palette_index = palette;
        geometry.vertices[i + 5].uv = c;
        geometry.vertices[i + 5].palette_index = palette;
    }

    return geometry;
}

static texture_t bin_texture(file_t* f)
{
    const int TEXTURE_ON_DISK_SIZE = (TEXTURE_SIZE / 2); // Each pixel stored as 1/2 a byte

    texture_t texture = { 0 };

    for (int i = 0; i < TEXTURE_ON_DISK_SIZE * 8; i += 8) {
        uint8_t raw_pixel = bin_u8(f);
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

    return texture;
}

static palette_t bin_palette(file_t* f)
{
    palette_t palette = { 0 };

    f->offset = 0x44;
    f->offset = bin_u32(f);

    for (int i = 0; i < 16 * 16 * 4; i = i + 4) {
        vec4s c = bin_rgb15(f);
        palette.data[i + 0] = c.x;
        palette.data[i + 1] = c.y;
        palette.data[i + 2] = c.z;
        palette.data[i + 3] = c.w;
    }

    return palette;
}

model_t bin_map(FILE* bin, int num)
{
    model_t model = {
        .transform = {
            .translation = (vec3s) { { 0.0f, 0.0f, 0.0f } },
            .rotation = (vec3s) { { 0.0f, 0.0f, 0.0f } },
            .scale = (vec3s) { { 1.0f, 1.0f, 1.0f } },
        },
    };

    file_t gns_file = bin_file(bin, map_list[num].sector, BIN_GNS_FILE_MAX_SIZE);
    records_t records = bin_records(&gns_file);

    for (int i = 0; i < records.count; i++) {
        record_t record = records.records[i];

        file_t file = bin_file(bin, record.sector, record.length);

        switch (record.type) {
        case FILE_TYPE_MESH_PRIMARY:
            model.geometry = bin_geometry(&file);
            model.texture = bin_texture(&file);
            model.palette = bin_palette(&file);
            model.centered_translation = geometry_centered_translation(&model.geometry);
            model.normalized_scale = geometry_normalized_scale(&model.geometry);
            break;
        case FILE_TYPE_TEXTURE:
            model.texture = bin_texture(&file);
            break;
        default:
            break;
        }
    }

    return model;
}

map_t map_list[128] = {
    { 0, 10026, false, "Unknown" }, // No texture
    { 1, 11304, true, "At Main Gate of Igros Castle" },
    { 2, 12656, true, "Back Gate of Lesalia Castle" },
    { 3, 12938, true, "Hall of St. Murond Temple" },
    { 4, 13570, true, "Office of Lesalia Castle" },
    { 5, 14239, true, "Roof of Riovanes Castle" },
    { 6, 14751, true, "At the Gate of Riovanes Castle" },
    { 7, 15030, true, "Inside of Riovanes Castle" },
    { 8, 15595, true, "Riovanes Castle" },
    { 9, 16262, true, "Citadel of Igros Castle" },
    { 10, 16347, true, "Inside of Igros Castle" },
    { 11, 16852, true, "Office of Igros Castle" },
    { 12, 17343, true, "At the Gate of Lionel Castle" },
    { 13, 17627, true, "Inside of Lionel Castle" },
    { 14, 18175, true, "Office of Lionel Castle" },
    { 15, 19510, true, "At the Gate of Limberry Castle (1)" },
    { 16, 20075, true, "Inside of Limberry Castle" },
    { 17, 20162, true, "Underground Cemetery of Limberry Castle" },
    { 18, 20745, true, "Office of Limberry Castle" },
    { 19, 21411, true, "At the Gate of Limberry Castle (2)" },
    { 20, 21692, true, "Inside of Zeltennia Castle" },
    { 21, 22270, true, "Zeltennia Castle" },
    { 22, 22938, true, "Magic City Gariland" },
    { 23, 23282, true, "Belouve Residence" },
    { 24, 23557, true, "Military Academy's Auditorium" },
    { 25, 23899, true, "Yardow Fort City" },
    { 26, 23988, true, "Weapon Storage of Yardow" },
    { 27, 24266, true, "Goland Coal City" },
    { 28, 24544, true, "Colliery Underground First Floor" },
    { 29, 24822, true, "Colliery Underground Second Floor" },
    { 30, 25099, true, "Colliery Underground Third Floor" },
    { 31, 25764, true, "Dorter Trade City" },
    { 32, 26042, true, "Slums in Dorter" },
    { 33, 26229, true, "Hospital in Slums" },
    { 34, 26362, true, "Cellar of Sand Mouse" },
    { 35, 27028, true, "Zaland Fort City" },
    { 36, 27643, true, "Church Outside of Town" },
    { 37, 27793, true, "Ruins Outside Zaland" },
    { 38, 28467, true, "Goug Machine City" },
    { 39, 28555, true, "Underground Passage in Goland" },
    { 40, 29165, true, "Slums in Goug" },
    { 41, 29311, true, "Besrodio's House" },
    { 42, 29653, true, "Warjilis Trade City" },
    { 43, 29807, true, "Port of Warjilis" },
    { 44, 30473, true, "Bervenia Free City" },
    { 45, 30622, true, "Ruins of Zeltennia Castle's Church" },
    { 46, 30966, true, "Cemetery of Heavenly Knight, Balbanes" },
    { 47, 31697, true, "Zarghidas Trade City" },
    { 48, 32365, true, "Slums of Zarghidas" },
    { 49, 33032, true, "Fort Zeakden" },
    { 50, 33701, true, "St. Murond Temple" },
    { 51, 34349, true, "St. Murond Temple" },
    { 52, 34440, true, "Chapel of St. Murond Temple" },
    { 53, 34566, true, "Entrance to Death City" },
    { 54, 34647, true, "Lost Sacred Precincts" },
    { 55, 34745, true, "Graveyard of Airships" },
    { 56, 35350, true, "Orbonne Monastery" },
    { 57, 35436, true, "Underground Book Storage First Floor" },
    { 58, 35519, true, "Underground Book Storage Second Floor" },
    { 59, 35603, true, "Underground Book Storage Third Floor" },
    { 60, 35683, true, "Underground Book Storage Fourth Floor" },
    { 61, 35765, true, "Underground Book Storage Fifth Floor" },
    { 62, 36052, true, "Chapel of Orbonne Monastery" },
    { 63, 36394, true, "Golgorand Execution Site" },
    { 64, 36530, true, "In Front of Bethla Garrison's Sluice" },
    { 65, 36612, true, "Granary of Bethla Garrison" },
    { 66, 37214, true, "South Wall of Bethla Garrison" },
    { 67, 37817, true, "North Wall of Bethla Garrison" },
    { 68, 38386, true, "Bethla Garrison" },
    { 69, 38473, true, "Murond Death City" },
    { 70, 38622, true, "Nelveska Temple" },
    { 71, 39288, true, "Dolbodar Swamp" },
    { 72, 39826, true, "Fovoham Plains" },
    { 73, 40120, true, "Inside of Windmill Shed" },
    { 74, 40724, true, "Sweegy Woods" },
    { 75, 41391, true, "Bervenia Volcano" },
    { 76, 41865, true, "Zeklaus Desert" },
    { 77, 42532, true, "Lenalia Plateau" },
    { 78, 43200, true, "Zigolis Swamp" },
    { 79, 43295, true, "Yuguo Woods" },
    { 80, 43901, true, "Araguay Woods" },
    { 81, 44569, true, "Grog Hill" },
    { 82, 45044, true, "Bed Desert" },
    { 83, 45164, true, "Zirekile Falls" },
    { 84, 45829, true, "Bariaus Hill" },
    { 85, 46498, true, "Mandalia Plains" },
    { 86, 47167, true, "Doguola Pass" },
    { 87, 47260, true, "Bariaus Valley" },
    { 88, 47928, true, "Finath River" },
    { 89, 48595, true, "Poeskas Lake" },
    { 90, 49260, true, "Germinas Peak" },
    { 91, 49538, true, "Thieves Fort" },
    { 92, 50108, true, "Igros-Belouve Residence" },
    { 93, 50387, true, "Broke Down Shed-Wooden Building" },
    { 94, 50554, true, "Broke Down Shed-Stone Building" },
    { 95, 51120, true, "Church" },
    { 96, 51416, true, "Pub" },
    { 97, 52082, true, "Inside Castle Gate in Lesalia" },
    { 98, 52749, true, "Outside Castle Gate in Lesalia" },
    { 99, 53414, true, "Main Street of Lesalia" },
    { 100, 53502, true, "Public Cemetery" },
    { 101, 53579, true, "Tutorial (1)" },
    { 102, 53659, true, "Tutorial (2)" },
    { 103, 54273, true, "Windmill Shed" },
    { 104, 54359, true, "Belouve Residence" },
    { 105, 54528, true, "TERMINATE" },
    { 106, 54621, true, "DELTA" },
    { 107, 54716, true, "NOGIAS" },
    { 108, 54812, true, "VOYAGE" },
    { 109, 54909, true, "BRIDGE" },
    { 110, 55004, true, "VALKYRIES" },
    { 111, 55097, true, "MLAPAN" },
    { 112, 55192, true, "TIGER" },
    { 113, 55286, true, "HORROR" },
    { 114, 55383, true, "END" },
    { 115, 56051, true, "Banished Fort" },
    { 116, 56123, true, "Arena" },
    { 117, 56201, true, "Unknown" },
    { 118, 56279, true, "Unknown" },
    { 119, 56356, true, "Unknown" },
    { 120, 0, false, "???" },
    { 121, 0, false, "???" },
    { 122, 0, false, "???" },
    { 123, 0, false, "???" },
    { 124, 0, false, "???" },
    { 125, 56435, true, "Unknown" },
    { 126, 0, false, "???" },
    { 127, 0, false, "???" },
};
