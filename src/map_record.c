#include "map_record.h"

#include <string.h>

map_record_t read_map_record(buffer_t* f)
{
    u8 bytes[MAP_RECORD_SIZE];
    read_bytes(f, MAP_RECORD_SIZE, bytes);
    usize sector = bytes[8] | bytes[9] << 8;
    usize length = (u32)(bytes[12]) | ((u32)(bytes[13]) << 8) | ((u32)(bytes[14]) << 16) | ((u32)(bytes[15]) << 24);
    filetype_e type = (bytes[4] | (bytes[5] << 8));
    time_e time = (bytes[3] >> 7) & 0x1;
    weather_e weather = (bytes[3] >> 4) & 0x7;
    int layout = bytes[2];

    map_record_t record = {
        .sector = sector,
        .length = length,
        .type = type,
        .state = {
            .time = time,
            .weather = weather,
            .layout = layout,
        },
    };
    memcpy(record.data, bytes, MAP_RECORD_SIZE);
    return record;
}

int read_map_records(buffer_t* f, map_record_t* out_records)
{
    int count = 0;
    while (true) {
        map_record_t record = read_map_record(f);
        if (record.type == FILETYPE_END) {
            break;
        }
        out_records[count] = record;
        count++;
    }
    return count;
}

const char* time_str(time_e value)
{
    switch (value) {
    case TIME_DAY:
        return "Day";
    case TIME_NIGHT:
        return "Night";
    default:
        return "Unknown";
    }
}

const char* weather_str(weather_e value)
{
    switch (value) {
    case WEATHER_NONE:
        return "None";
    case WEATHER_NONE_ALT:
        return "NoneAlt";
    case WEATHER_NORMAL:
        return "Normal";
    case WEATHER_STRONG:
        return "Strong";
    case WEATHER_VERY_STRONG:
        return "VeryStrong";
    default:
        return "Unknown";
    }
}

const char* filetype_str(filetype_e value)
{
    switch (value) {
    case FILETYPE_MESH_PRIMARY:
        return "Primary";
    case FILETYPE_MESH_ALT:
        return "Alt";
    case FILETYPE_MESH_OVERRIDE:
        return "Override";
    case FILETYPE_TEXTURE:
        return "Texture";
    case FILETYPE_END:
        return "End";
    default:
        return "Unknown";
    }
}

bool map_state_default(map_state_t a)
{
    return a.time == TIME_DAY && a.weather == WEATHER_NONE && a.layout == 0;
}

bool map_state_eq(map_state_t a, map_state_t b)
{
    return a.time == b.time && a.weather == b.weather && a.layout == b.layout;
}

bool map_record_state_unique(map_record_t* unique_records, int unique_record_count, map_record_t record)
{
    for (int i = 0; i < unique_record_count; i++) {
        if (map_state_eq(unique_records[i].state, record.state)) {
            return false;
        }
    }
    return true;
}
