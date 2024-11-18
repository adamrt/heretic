#ifndef BIN_H_
#define BIN_H_

#include <stdint.h>

#include "gfx.h"
#include "model.h"

#define EVENT_COUNT           (500)
#define SCENARIO_TOTAL_COUNT  (488)
#define SCENARIO_USABLE_COUNT (302)

#define GNS_RECORD_MAX_NUM (100)
#define GNS_RECORD_SIZE    (20)

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
} map_t;

typedef struct {
    map_t* map;
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

map_t* read_map(int, map_state_t);

void time_str(time_e, char[static 8]);
void weather_str(weather_e, char[static 12]);
void file_type_str(file_type_e, char[static 12]);

//
// Public variables
//

extern scenario_desc_t scenario_list[];
extern map_desc_t map_list[];

#endif // BIN_H_
