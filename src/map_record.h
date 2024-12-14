#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "defines.h"
#include "span.h"

#define MAP_FILE_MAX_SIZE  (2388)
#define MAP_RECORD_MAX_NUM (100)
#define MAP_RECORD_SIZE    (20)

typedef enum {
    FILETYPE_NONE = 0x0000,
    FILETYPE_TEXTURE = 0x1701,
    FILETYPE_MESH_PRIMARY = 0x2E01,
    FILETYPE_MESH_OVERRIDE = 0x2F01,
    FILETYPE_MESH_ALT = 0x3001,
    FILETYPE_END = 0x3101,
} filetype_e;

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

// Each resource is a file that contains a single type of data (mesh, texture).
// Each resource is related to a specific time, weather, and layout.
typedef struct {
    time_e time;
    weather_e weather;
    int layout;
} map_state_t;

// A map record is the information for a specific resource.
// It can be for a mesh (multiple types), texture, or end of file.
//
// These are also called GNS records.
typedef struct {
    filetype_e type;
    usize sector;
    usize length;

    map_state_t state;

    u8 data[MAP_RECORD_SIZE];
} map_record_t;

static map_state_t default_map_state = (map_state_t) {
    .time = TIME_DAY,
    .weather = WEATHER_NONE,
    .layout = 0,
};

map_record_t read_map_record(span_t*);
int read_map_records(span_t*, map_record_t*);

bool map_state_eq(map_state_t, map_state_t);
bool map_state_default(map_state_t);
bool map_record_state_unique(map_record_t*, int, map_record_t);

const char* time_str(time_e);
const char* weather_str(weather_e);
const char* filetype_str(filetype_e);
