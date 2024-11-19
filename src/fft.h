#ifndef FFT_H_
#define FFT_H_

#include <stdint.h>

#include "cglm/struct.h"
#include "sokol_gfx.h"

#include "gfx.h"

#define GNS_RECORD_MAX_NUM (100)
#define GNS_RECORD_SIZE    (20)

#define SCENARIO_USABLE_COUNT (302)

#define MESH_MAX_TEX_TRIS    (512)
#define MESH_MAX_TEX_QUADS   (768)
#define MESH_MAX_UNTEX_TRIS  (64)
#define MESH_MAX_UNTEX_QUADS (256)
#define MESH_MAX_VERTICES    (7620)
#define MESH_MAX_LIGHTS      (3)

#define TEXTURE_WIDTH     (256)
#define TEXTURE_HEIGHT    (1024)
#define TEXTURE_SIZE      (TEXTURE_WIDTH * TEXTURE_HEIGHT) // 262144
#define TEXTURE_BYTE_SIZE (TEXTURE_SIZE * 4)

#define PALETTE_WIDTH     (256)
#define PALETTE_HEIGHT    (1)
#define PALETTE_SIZE      (PALETTE_WIDTH * PALETTE_HEIGHT)
#define PALETTE_BYTE_SIZE (PALETTE_SIZE * 4)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

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

typedef enum {
    FILE_TYPE_NONE = 0x0000,
    FILE_TYPE_TEXTURE = 0x1701,
    FILE_TYPE_MESH_PRIMARY = 0x2E01,
    FILE_TYPE_MESH_OVERRIDE = 0x2F01,
    FILE_TYPE_MESH_ALT = 0x3001,
    FILE_TYPE_END = 0x3101,
} file_type_e;

// Each resource is a file that contains a single type of data (mesh, texture).
// Each resource is related to a specific time, weather, and layout.
typedef struct {
    time_e time;
    weather_e weather;
    int layout;
} map_state_t;

typedef struct {
    vec3s position;
    vec3s normal;
    vec2s uv;
    float palette_index;
    float is_textured;
} vertex_t;

typedef struct {
    vertex_t vertices[MESH_MAX_VERTICES];
    int count;

    struct {
        vec3s vmin;
        vec3s vmax;
        float vcount;
        float tri_count;
        float quad_count;
    } meta;

    bool valid;
} geometry_t;

typedef struct {
    uint8_t data[TEXTURE_BYTE_SIZE];
    bool valid;
} texture_t;

typedef struct {
    uint8_t data[PALETTE_BYTE_SIZE];
    bool valid;
} palette_t;

typedef struct {
    vec3s direction;
    vec4s color;

    bool valid;
} light_t;

// Lighting is a collection of lights, ambient color, and background colors.
// This is because of hose the data is stored on the PSX bin.
typedef struct {
    light_t lights[MESH_MAX_LIGHTS];

    vec4s ambient_color;
    float ambient_strength;

    vec4s bg_top;
    vec4s bg_bottom;

    bool valid;
} lighting_t;

static map_state_t default_map_state = (map_state_t) {
    .time = TIME_DAY,
    .weather = WEATHER_NONE,
    .layout = 0,
};

typedef struct {
    geometry_t geometry;
    texture_t texture;
    palette_t palette;
    lighting_t lighting;

    bool valid;
} mesh_t;

typedef struct {
    vec3s translation;
    vec3s rotation;
    vec3s scale;
} transform_t;

typedef struct {
    int id;
    int map_id;
    weather_e weather;
    time_e time;

    int entd_id;
    int next_scenario_id;
} scenario_t;

typedef struct {
    int text_count;
    int code_count;

    uint8_t text[8192];
    uint8_t code[8192];
    bool valid;
} event_t;

typedef struct {
    file_type_e type;
    size_t sector;
    size_t length;

    map_state_t state;

    uint8_t data[GNS_RECORD_SIZE];
} record_t;

typedef struct {
    record_t records[GNS_RECORD_MAX_NUM];
    int count;
} records_t;

typedef struct {
    mesh_t mesh;
    transform_t transform;
    renderable_t renderable;

    struct {
        records_t records;
        texture_t textures[16];
    } meta;

    // computed properties
    mat4s model_matrix;
    vec3s centered_translation;
} map_data_t;

typedef struct {
    map_data_t* map;
    bool center_model;
    int current_scenario;
    int current_map;
    map_state_t map_state;
} scene_t;

// Scenario and map descriptors
typedef struct {
    uint16_t id;
    const char* name;
} scenario_desc_t;

typedef struct {
    uint8_t id;
    uint16_t sector;
    bool valid;
    const char* name;
} map_desc_t;

//
// Public functions
//

void bin_load_global_data(void);
void bin_free_global_data(void);

map_data_t* read_map(int, map_state_t);

void time_str(time_e, char[static 8]);
void weather_str(weather_e, char[static 12]);
void file_type_str(file_type_e, char[static 12]);

//
// Public variables
//

extern scenario_desc_t scenario_list[];
extern map_desc_t map_list[];

#endif // FFT_H_
