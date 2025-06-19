#include "map_record.h"

#include <string.h>

u32 parse_u32(u8* bytes) {
    return (u32)(bytes[0]) | ((u32)(bytes[1]) << 8) | ((u32)(bytes[2]) << 16) | ((u32)(bytes[3]) << 24);
}

u16 parse_u16(u8* bytes) {
    return (u16)(bytes[0]) | ((u16)(bytes[1]) << 8);
}

// read_map_record reads a map record from the span. Records are 20
// bytes long and contain information about a specific resource.
//
// Format: AAAA BBCC DDDD xxxx EEEE xxxx FFFF FFFF xxxx xxxx
//
// xxxx: padding
// AAAA: 2 bytes, always either 0x22, 0x30 or 0x70, but its purpose is unknown.
// BB: 1 bytes, room arrangement/layout
// CC: 1 byte, time is highest bit (0=day, 1=night) and weather is next 3 bits (values can be 0-4).
// DDDD: 2 bytes, file type (0x1701=texture, 0x2E01=primary mesh, 0x2F01=override mesh, 0x3001=alt mesh, 0x3101=end)
// EEEE: 2 bytes, sector of resource in the file system
// FFFF: 4 bytes, length of the resource in bytes
map_record_t read_map_record(span_t* span) {
    u8 bytes[MAP_RECORD_SIZE];
    span_read_bytes(span, MAP_RECORD_SIZE, bytes);

    int layout = bytes[2];
    time_e time = (time_e)((bytes[3] >> 7) & 0x1);
    weather_e weather = (weather_e)((bytes[3] >> 4) & 0x7);
    filetype_e type = (filetype_e)parse_u16(&bytes[4]);
    usize sector = parse_u32(&bytes[8]);
    usize length = parse_u32(&bytes[12]);

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

int read_map_records(span_t* span, map_record_t* out_records) {
    int count = 0;
    while (span->offset + 20 < span->size) {
        map_record_t record = read_map_record(span);
        if (record.type == FILETYPE_END) {
            // End of records, stop reading.
            break;
        }
        out_records[count++] = record;
    }
    return count;
}

const char* time_str(time_e value) {
    switch (value) {
    case TIME_DAY:
        return "Day";
    case TIME_NIGHT:
        return "Night";
    default:
        return "Unknown";
    }
}

const char* weather_str(weather_e value) {
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

const char* filetype_str(filetype_e value) {
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

bool map_state_default(map_state_t a) {
    return a.time == TIME_DAY && a.weather == WEATHER_NONE && a.layout == 0;
}

bool map_state_eq(map_state_t a, map_state_t b) {
    return a.time == b.time && a.weather == b.weather && a.layout == b.layout;
}

bool map_record_state_unique(map_record_t* unique_records, int unique_record_count, map_record_t record) {
    for (int i = 0; i < unique_record_count; i++) {
        if (map_state_eq(unique_records[i].state, record.state)) {
            return false;
        }
    }
    return true;
}
