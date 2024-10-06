#ifndef BIN_H_
#define BIN_H_

#include <stdint.h>
#include <stdio.h>

#include "model.h"

#define BIN_FILE_MAX_SIZE (131072)
#define BIN_GNS_FILE_MAX_SIZE (2388)
#define BIN_GNS_RECORD_MAX_NUM (100)
#define BIN_GNS_RECORD_SIZE (20)
#define BIN_SECTOR_HEADER_SIZE (24)
#define BIN_SECTOR_SIZE (2048)
#define BIN_SECTOR_SIZE_RAW (2352)

typedef struct {
    uint8_t data[BIN_FILE_MAX_SIZE];
    size_t size;
} bytes_t;

typedef struct {
    uint8_t data[BIN_SECTOR_SIZE];
} sector_t;

typedef struct {
    uint8_t data[BIN_FILE_MAX_SIZE];
    size_t offset;
    size_t size;
} file_t;

typedef enum {
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
    int arrangement;

    uint8_t data[BIN_GNS_RECORD_SIZE];
} record_t;

typedef struct {
    record_t records[BIN_GNS_RECORD_MAX_NUM];
    int count;
} records_t;

typedef struct {
    uint8_t id;
    uint16_t sector;
    bool valid;
    const char* name;
} map_t;

extern map_t map_list[];

model_t bin_map(FILE* bin, int num);

#endif // BIN_H_
