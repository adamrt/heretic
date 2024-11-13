#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "bin.h"
#include "model.h"

#define SECTOR_HEADER_SIZE (24)
#define SECTOR_SIZE (2048)
#define SECTOR_SIZE_RAW (2352)

#define FILE_MAX_SIZE (131072)

#define GNS_FILE_MAX_SIZE (2388)
#define GNS_RECORD_MAX_NUM (100)
#define GNS_RECORD_SIZE (20)

typedef struct {
    uint8_t data[FILE_MAX_SIZE];
    size_t size;
} bytes_t;

typedef struct {
    uint8_t data[SECTOR_SIZE];
} sector_t;

typedef struct {
    uint8_t data[FILE_MAX_SIZE];
    size_t offset;
    size_t size;
} file_t;

typedef enum {
    FILE_TYPE_NONE = 0x0000,
    FILE_TYPE_TEXTURE = 0x1701,
    FILE_TYPE_MESH_PRIMARY = 0x2E01,
    FILE_TYPE_MESH_OVERRIDE = 0x2F01,
    FILE_TYPE_MESH_ALT = 0x3001,
    FILE_TYPE_END = 0x3101,
} file_type_e;

typedef enum {
    TIME_DAY = 0x0,
    TIME_NIGHT = 0x1,
} time_e;

typedef enum {
    WEATHER_NONE = 0x0,
    WEATHER_NONE_ALT = 0x1,
    WEATHER_NORMAL = 0x2,
    WEATHER_STRONG = 0x3,
    WEATHER_VERY_STRONG = 0x4,
} weather_e;

typedef struct {
    file_type_e type;
    size_t sector;
    size_t length;

    time_e time;
    weather_e weather;
    int layout;

    uint8_t data[GNS_RECORD_SIZE];
} record_t;

typedef struct {
    record_t records[GNS_RECORD_MAX_NUM];
    int count;
} records_t;

// Each resource is a file that contains a single type of data (mesh, texture,
// etc). Each resource is related to a specific time, weather, and layout.
typedef struct {
    time_e time;
    weather_e weather;
    int layout;
} resource_key_t;

typedef struct {
    resource_key_t key;
    file_type_e type;
    void* resource_data;
    bool valid;
} resource_t;

static resource_key_t fallback_key = (resource_key_t) {
    .time = TIME_DAY,
    .weather = WEATHER_NONE,
    .layout = 0,
};

// Forward declarations
static void add_resource(resource_t*, int, file_type_e, resource_key_t, void*);
static void* find_resource(resource_t*, int, file_type_e, resource_key_t);

static uint8_t read_u8(file_t*);
static uint16_t read_u16(file_t*);
static uint32_t read_u32(file_t*);
static int8_t read_i8(file_t*);
static int16_t read_i16(file_t*);
static int32_t read_i32(file_t*);

static bytes_t read_bytes(file_t*, int);
static sector_t read_sector(FILE*, int32_t);
static file_t read_file(FILE*, int, int);

static float read_f1x3x12(file_t*);
static vec3s read_position(file_t*);
static vec3s read_normal(file_t*);
static vec4s read_rgb8(file_t*);
static vec4s read_rgb15(file_t*);
static float read_light_color(file_t*);

static record_t read_record(file_t*);
static records_t read_records(file_t*);

static geometry_t read_geometry(file_t*);
static texture_t read_texture(file_t*);
static palette_t read_palette(file_t*);
static lighting_t read_lighting(file_t*);

static mesh_t read_mesh(file_t*);

// Utility functions
static void merge_meshes(mesh_t*, mesh_t*);
static vec2s process_tex_coords(float u, float v, uint8_t page);

model_t read_map(FILE* bin, int num)
{
    resource_t resources[GNS_RECORD_MAX_NUM];
    int resource_count = 0;

    file_t gns_file = read_file(bin, map_list[num].sector, GNS_FILE_MAX_SIZE);
    records_t records = read_records(&gns_file);
    resource_key_t requested_key = { .time = TIME_DAY, .weather = WEATHER_NONE, .layout = 0 };

    model_t model = { 0 };

    for (int i = 0; i < records.count; i++) {
        record_t record = records.records[i];
        resource_key_t record_key = { record.time, record.weather, record.layout };

        file_t file = read_file(bin, record.sector, record.length);

        switch (record.type) {
        case FILE_TYPE_MESH_PRIMARY:
            model.mesh = read_mesh(&file);
            if (!model.mesh.valid) {
                assert(false);
            }
            break;

        case FILE_TYPE_TEXTURE: {
            // There can be duplicate textures for the same time/weather. Use the first one.
            void* found = find_resource(resources, resource_count, record.type, record_key);
            if (found == NULL) {
                texture_t texture = read_texture(&file);
                add_resource(resources, resource_count, record.type, record_key, &texture);
                resource_count++;
            }
            break;
        }

        case FILE_TYPE_MESH_ALT: {
            mesh_t mesh = read_mesh(&file);
            if (!mesh.geometry.valid) {
                break;
            }
            add_resource(resources, resource_count, record.type, record_key, &mesh);
            resource_count++;
            break;
        }

        case FILE_TYPE_MESH_OVERRIDE: {
            mesh_t mesh = read_mesh(&file);
            if (!mesh.geometry.valid) {
                break;
            }

            add_resource(resources, resource_count, record.type, record_key, &mesh);
            resource_count++;
            break;
        }

        default:
            break;
        }
    }

    // FIXME: We might not want to use fallback. Maybe use fallback only if there is no primary mesh.

    mesh_t* override_mesh = find_resource(resources, resource_count, FILE_TYPE_MESH_OVERRIDE, requested_key);
    if (!model.mesh.valid) {
        if (override_mesh == NULL) {
            override_mesh = find_resource(resources, resource_count, FILE_TYPE_MESH_OVERRIDE, fallback_key);
            assert(override_mesh != NULL);
        }
    }

    if (override_mesh != NULL) {
        merge_meshes(&model.mesh, override_mesh);
    }

    mesh_t* alt_mesh = find_resource(resources, resource_count, FILE_TYPE_MESH_ALT, requested_key);
    if (alt_mesh != NULL) {
        merge_meshes(&model.mesh, alt_mesh);
    }

    // Select texture
    texture_t* texture = find_resource(resources, resource_count, FILE_TYPE_TEXTURE, requested_key);
    if (texture == NULL) {
        texture = find_resource(resources, resource_count, FILE_TYPE_TEXTURE, fallback_key);
    }
    assert(texture != NULL);

    model.mesh.texture = *texture;
    model.transform = (transform_t) { .scale = (vec3s) { { 1.0f, 1.0f, 1.0f } } };
    model.centered_translation = geometry_centered_translation(&model.mesh.geometry);

    return model;
}

static geometry_t read_geometry(file_t* f)
{
    geometry_t geometry = { 0 };

    // 0x40 is always the location of the primary mesh pointer.
    // 0xC4 is always the primary mesh pointer.
    f->offset = 0x40;
    f->offset = read_u32(f);
    if (f->offset == 0) {
        return geometry;
    }

    // The number of each type of polygon.
    int N = read_u16(f); // Textured triangles
    int P = read_u16(f); // Textured quads
    int Q = read_u16(f); // Untextured triangles
    int R = read_u16(f); // Untextured quads

    // Validate maximum values
    assert(N < 512 && P < 768 && Q < 64 && R < 256);

    // Textured triangle
    for (int i = 0; i < N; i++) {
        geometry.vertices[geometry.count++].position = read_position(f);
        geometry.vertices[geometry.count].is_textured = 1.0f;
        geometry.vertices[geometry.count++].position = read_position(f);
        geometry.vertices[geometry.count].is_textured = 1.0f;
        geometry.vertices[geometry.count++].position = read_position(f);
        geometry.vertices[geometry.count].is_textured = 1.0f;
    }

    // Textured quads
    for (int i = 0; i < P; i++) {
        vec3s a = read_position(f);
        vec3s b = read_position(f);
        vec3s c = read_position(f);
        vec3s d = read_position(f);

        // Tri A
        geometry.vertices[geometry.count++].position = a;
        geometry.vertices[geometry.count].is_textured = 1.0f;
        geometry.vertices[geometry.count++].position = b;
        geometry.vertices[geometry.count].is_textured = 1.0f;
        geometry.vertices[geometry.count++].position = c;
        geometry.vertices[geometry.count].is_textured = 1.0f;

        // Tri B
        geometry.vertices[geometry.count++].position = b;
        geometry.vertices[geometry.count].is_textured = 1.0f;
        geometry.vertices[geometry.count++].position = d;
        geometry.vertices[geometry.count].is_textured = 1.0f;
        geometry.vertices[geometry.count++].position = c;
        geometry.vertices[geometry.count].is_textured = 1.0f;
    }

    // Untextured triangle
    for (int i = 0; i < Q; i++) {
        geometry.vertices[geometry.count++].position = read_position(f);
        geometry.vertices[geometry.count++].position = read_position(f);
        geometry.vertices[geometry.count++].position = read_position(f);
    }

    // Untextured quads
    for (int i = 0; i < R; i++) {
        vec3s a = read_position(f);
        vec3s b = read_position(f);
        vec3s c = read_position(f);
        vec3s d = read_position(f);

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
        geometry.vertices[i + 0].normal = read_normal(f);
        geometry.vertices[i + 1].normal = read_normal(f);
        geometry.vertices[i + 2].normal = read_normal(f);
    };

    // Quad normals
    for (int i = N * 3; i < N * 3 + (P * 3 * 2); i = i + 6) {
        vec3s a = read_normal(f);
        vec3s b = read_normal(f);
        vec3s c = read_normal(f);
        vec3s d = read_normal(f);

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
        float au = read_u8(f);
        float av = read_u8(f);
        float palette = read_u8(f);
        (void)read_u8(f); // padding
        float bu = read_u8(f);
        float bv = read_u8(f);
        float page = (read_u8(f) & 0x03); // 0b00000011
        (void)read_u8(f);                 // padding
        float cu = read_u8(f);
        float cv = read_u8(f);

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
        float au = read_u8(f);
        float av = read_u8(f);
        float palette = read_u8(f);
        (void)read_u8(f); // padding
        float bu = read_u8(f);
        float bv = read_u8(f);
        float page = (read_u8(f) & 0x03); // 0b00000011
        (void)read_u8(f);                 // padding
        float cu = read_u8(f);
        float cv = read_u8(f);
        float du = read_u8(f);
        float dv = read_u8(f);

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

    geometry.valid = true;
    return geometry;
}

static texture_t read_texture(file_t* f)
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

    return texture;
}

static palette_t read_palette(file_t* f)
{
    palette_t palette = { 0 };

    f->offset = 0x44;
    f->offset = read_u32(f);

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

// read_light_color clamps the value between 0.0 and 1.0. These unclamped values
// are used to affect the lighting model but it isn't understood yet.
// https://ffhacktics.com/wiki/Maps/Mesh#Light_colors_and_positions.2C_background_gradient_colors
static lighting_t read_lighting(file_t* f)
{
    lighting_t lighting = { 0 };

    f->offset = 0x64;
    uint32_t intra_file_ptr = read_u32(f);
    if (intra_file_ptr == 0) {
        return lighting;
    }

    f->offset = intra_file_ptr;

    vec4s a_color = { .w = 1.0f };
    vec4s b_color = { .w = 1.0f };
    vec4s c_color = { .w = 1.0f };

    a_color.x = read_light_color(f);
    b_color.x = read_light_color(f);
    c_color.x = read_light_color(f);
    a_color.y = read_light_color(f);
    b_color.y = read_light_color(f);
    c_color.y = read_light_color(f);
    a_color.z = read_light_color(f);
    b_color.z = read_light_color(f);
    c_color.z = read_light_color(f);

    bool a_valid = a_color.r + a_color.g + a_color.b > 0.0f;
    bool b_valid = b_color.r + b_color.g + b_color.b > 0.0f;
    bool c_valid = c_color.r + c_color.g + c_color.b > 0.0f;

    vec3s a_pos = read_position(f);
    vec3s b_pos = read_position(f);
    vec3s c_pos = read_position(f);

    lighting.lights[0] = (light_t) { .color = a_color, .direction = a_pos, .valid = a_valid };
    lighting.lights[1] = (light_t) { .color = b_color, .direction = b_pos, .valid = b_valid };
    lighting.lights[2] = (light_t) { .color = c_color, .direction = c_pos, .valid = c_valid };

    lighting.ambient_color = read_rgb8(f);
    lighting.ambient_strength = 2.0f;

    lighting.bg_top = read_rgb8(f);
    lighting.bg_bottom = read_rgb8(f);

    lighting.valid = true;
    return lighting;
}

mesh_t read_mesh(file_t* f)
{
    mesh_t mesh = { 0 };

    mesh.geometry = read_geometry(f);
    mesh.palette = read_palette(f);
    mesh.lighting = read_lighting(f);

    bool is_valid = mesh.geometry.valid || mesh.palette.valid || mesh.lighting.valid;
    mesh.valid = is_valid;

    return mesh;
}

void add_resource(resource_t* resources, int count, file_type_e type, resource_key_t key, void* resource)
{
    assert(count < GNS_RECORD_MAX_NUM);
    resources[count].key = key;
    resources[count].valid = true;
    resources[count].resource_data = resource;
    resources[count].type = type;
}

void* find_resource(resource_t* resources, int count, file_type_e type, resource_key_t key)
{
    for (int i = 0; i < count; i++) {
        resource_key_t mk = resources[i].key;
        if (resources[i].valid && resources[i].type == type && mk.time == key.time && mk.weather == key.weather && mk.layout == key.layout) {
            return resources[i].resource_data;
        }
    }
    return NULL;
}

static uint8_t read_u8(file_t* f)
{
    uint8_t value;
    memcpy(&value, &f->data[f->offset], sizeof(uint8_t));
    f->offset += sizeof(uint8_t);
    return value;
}

static uint16_t read_u16(file_t* f)
{
    uint16_t value;
    memcpy(&value, &f->data[f->offset], sizeof(uint16_t));
    f->offset += sizeof(uint16_t);
    return value;
}

static uint32_t read_u32(file_t* f)
{
    uint32_t value;
    memcpy(&value, &f->data[f->offset], sizeof(uint32_t));
    f->offset += sizeof(uint32_t);
    return value;
}

static int8_t read_i8(file_t* f)
{
    int8_t value;
    memcpy(&value, &f->data[f->offset], sizeof(int8_t));
    f->offset += sizeof(int8_t);
    return value;
}

static int16_t read_i16(file_t* f)
{
    int16_t value;
    memcpy(&value, &f->data[f->offset], sizeof(int16_t));
    f->offset += sizeof(int16_t);
    return value;
}

static int32_t read_i32(file_t* f)
{
    int32_t value;
    memcpy(&value, &f->data[f->offset], sizeof(int32_t));
    f->offset += sizeof(int32_t);
    return value;
}

static bytes_t read_bytes(file_t* f, int size)
{
    bytes_t bytes = { .size = size };
    for (int i = 0; i < size; i++) {
        bytes.data[i] = read_u8(f);
    }
    return bytes;
}

// read_read_sector reads a sector to `out_bytes`.
static sector_t read_sector(FILE* file, int32_t sector_num)
{
    int32_t seek_to = (sector_num * SECTOR_SIZE_RAW) + SECTOR_HEADER_SIZE;
    if (fseek(file, seek_to, SEEK_SET) != 0) {
        assert(false);
    }

    sector_t sector;
    size_t n = fread(sector.data, sizeof(uint8_t), SECTOR_SIZE, file);
    assert(n == SECTOR_SIZE);
    return sector;
}

static file_t read_file(FILE* bin, int sector_num, int size)
{
    file_t file = { .size = size };

    int offset = 0;
    uint32_t occupied_sectors = ceil((float)size / (float)SECTOR_SIZE);

    for (uint32_t i = 0; i < occupied_sectors; i++) {
        sector_t sector_data = read_sector(bin, sector_num + i);

        int remaining_size = size - offset;
        int bytes_to_copy = (remaining_size < SECTOR_SIZE) ? remaining_size : SECTOR_SIZE;

        memcpy(file.data + offset, sector_data.data, bytes_to_copy);
        offset += bytes_to_copy;
    }

    return file;
}

static float read_f1x3x12(file_t* f)
{
    float value = read_i16(f);
    return value / 4096.0f;
}

static vec3s read_position(file_t* f)
{
    float x = read_i16(f) / GLOBAL_SCALE;
    float y = read_i16(f) / GLOBAL_SCALE;
    float z = read_i16(f) / GLOBAL_SCALE;

    y = -y;
    z = -z;

    return (vec3s) { { x, y, z } };
}

static vec3s read_normal(file_t* f)
{
    float x = read_f1x3x12(f);
    float y = read_f1x3x12(f);
    float z = read_f1x3x12(f);

    y = -y;
    z = -z;

    return (vec3s) { { x, y, z } };
}

static vec4s read_rgb8(file_t* f)
{
    float r = read_u8(f) / 255.0f;
    float g = read_u8(f) / 255.0f;
    float b = read_u8(f) / 255.0f;
    return (vec4s) { { r, g, b, 1.0f } };
}

static vec4s read_rgb15(file_t* f)
{
    uint16_t val = read_u16(f);
    uint8_t a = val == 0 ? 0x00 : 0xFF;
    uint8_t b = (val & 0x7C00) >> 7; // 0b0111110000000000
    uint8_t g = (val & 0x03E0) >> 2; // 0b0000001111100000
    uint8_t r = (val & 0x001F) << 3; // 0b0000000000011111
    return (vec4s) { { r, g, b, a } };
}

static float read_light_color(file_t* f)
{
    float val = read_f1x3x12(f);
    return MIN(MAX(0.0f, val), 1.0f);
}

static record_t read_record(file_t* f)
{
    bytes_t bytes = read_bytes(f, GNS_RECORD_SIZE);
    int sector = bytes.data[8] | bytes.data[9] << 8;
    uint64_t length = (uint32_t)(bytes.data[12]) | ((uint32_t)(bytes.data[13]) << 8) | ((uint32_t)(bytes.data[14]) << 16) | ((uint32_t)(bytes.data[15]) << 24);
    file_type_e type = (bytes.data[4] | (bytes.data[5] << 8));
    time_e time = (bytes.data[3] >> 7) & 0x1;
    weather_e weather = (bytes.data[3] >> 4) & 0x7;
    int layout = bytes.data[2];

    record_t record = {
        .sector = sector,
        .length = length,
        .type = type,
        .time = time,
        .weather = weather,
        .layout = layout,
    };
    memcpy(record.data, bytes.data, GNS_RECORD_SIZE);
    return record;
}

static records_t read_records(file_t* f)
{
    records_t records = { .count = 0 };
    while (true) {
        record_t record = read_record(f);
        if (record.type == FILE_TYPE_END) {
            break;
        }
        records.records[records.count] = record;
        records.count++;
    }
    return records;
}

void merge_meshes(mesh_t* dst, mesh_t* src)
{
    assert(dst != NULL);
    assert(src != NULL);

    if (src->geometry.valid) {
        for (int i = 0; i < src->geometry.count; i++) {
            dst->geometry.vertices[dst->geometry.count++] = src->geometry.vertices[i];
        }
    }

    if (src->palette.valid) {
        dst->palette = src->palette;
    }

    if (src->lighting.valid) {
        for (int i = 0; i < MESH_MAX_LIGHTS; i++) {
            if (src->lighting.lights[i].valid) {
                for (int j = 0; j < MESH_MAX_LIGHTS; j++) {
                    dst->lighting.lights[j] = src->lighting.lights[j];
                }
                break;
            }
        }

        dst->lighting.ambient_color = src->lighting.ambient_color;
        dst->lighting.bg_top = src->lighting.bg_top;
        dst->lighting.bg_bottom = src->lighting.bg_bottom;
    }
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
