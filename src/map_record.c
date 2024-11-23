#include "map_record.h"

#include <string.h>

record_t read_record(file_t* f)
{
    uint8_t bytes[GNS_RECORD_SIZE];
    read_bytes(f, GNS_RECORD_SIZE, bytes);
    int sector = bytes[8] | bytes[9] << 8;
    uint64_t length = (uint32_t)(bytes[12]) | ((uint32_t)(bytes[13]) << 8) | ((uint32_t)(bytes[14]) << 16) | ((uint32_t)(bytes[15]) << 24);
    filetype_e type = (bytes[4] | (bytes[5] << 8));
    time_e time = (bytes[3] >> 7) & 0x1;
    weather_e weather = (bytes[3] >> 4) & 0x7;
    int layout = bytes[2];

    record_t record = {
        .sector = sector,
        .length = length,
        .type = type,
        .state = {
            .time = time,
            .weather = weather,
            .layout = layout,
        },
    };
    memcpy(record.data, bytes, GNS_RECORD_SIZE);
    return record;
}

int read_records(file_t* f, record_t* out_records)
{
    int count = 0;
    while (true) {
        record_t record = read_record(f);
        if (record.type == FILETYPE_END) {
            break;
        }
        out_records[count] = record;
        count++;
    }
    return count;
}

void time_str(time_e value, char out[static 8])
{
    switch (value) {
    case TIME_DAY:
        strcpy(out, "Day");
        break;
    case TIME_NIGHT:
        strcpy(out, "Night");
        break;
    default:
        strcpy(out, "Unknown");
        break;
    }
}

void weather_str(weather_e value, char out[static 12])
{
    switch (value) {
    case WEATHER_NONE:
        strcpy(out, "None");
        break;
    case WEATHER_NONE_ALT:
        strcpy(out, "NoneAlt");
        break;
    case WEATHER_NORMAL:
        strcpy(out, "Normal");
        break;
    case WEATHER_STRONG:
        strcpy(out, "Strong");
        break;
    case WEATHER_VERY_STRONG:
        strcpy(out, "VeryStrong");
        break;
    default:
        strcpy(out, "Unknown");
        break;
    }
}

void filetype_str(filetype_e value, char* out)
{
    switch (value) {
    case FILETYPE_MESH_PRIMARY:
        strcpy(out, "Primary");
        break;
    case FILETYPE_MESH_ALT:
        strcpy(out, "Alt");
        break;
    case FILETYPE_MESH_OVERRIDE:
        strcpy(out, "Override");
        break;
    case FILETYPE_TEXTURE:
        strcpy(out, "Texture");
        break;
    case FILETYPE_END:
        strcpy(out, "End");
        break;
    default:
        strcpy(out, "Unknown");
        break;
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

bool record_map_state_unique(record_t* unique_records, int unique_record_count, record_t record)
{
    for (int i = 0; i < unique_record_count; i++) {
        if (map_state_eq(unique_records[i].state, record.state)) {
            return false;
        }
    }
    return true;
}
