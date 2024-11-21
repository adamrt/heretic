#include <string.h>

#include "bin.h"

#define SECTOR_HEADER_SIZE (24)
#define SECTOR_SIZE        (2048)
#define SECTOR_SIZE_RAW    (2352)

#define FILE_SIZE_MAX (131072)

// Forward declarations
static void read_sector(FILE* f, int32_t sector_num, uint8_t out_sector[static SECTOR_SIZE]);

file_t read_file(FILE* f, int sector_num, int size)
{
    file_t file = { .size = size };
    file.data = calloc(1, size);

    int offset = 0;
    uint32_t occupied_sectors = ceil((double)size / (double)SECTOR_SIZE);

    for (uint32_t i = 0; i < occupied_sectors; i++) {
        uint8_t sector[SECTOR_SIZE];
        read_sector(f, sector_num + i, sector);

        int remaining_size = size - offset;
        int bytes_to_copy = (remaining_size < SECTOR_SIZE) ? remaining_size : SECTOR_SIZE;

        memcpy(file.data + offset, sector, bytes_to_copy);
        offset += bytes_to_copy;
    }

    return file;
}

void read_bytes(file_t* f, int size, uint8_t* out_bytes)
{
    assert(size < FILE_SIZE_MAX);
    for (int i = 0; i < size; i++) {
        out_bytes[i] = read_u8(f);
    }
    return;
}

uint8_t read_u8(file_t* f)
{
    uint8_t value;
    memcpy(&value, &f->data[f->offset], sizeof(uint8_t));
    f->offset += sizeof(uint8_t);
    return value;
}

uint16_t read_u16(file_t* f)
{
    uint16_t value;
    memcpy(&value, &f->data[f->offset], sizeof(uint16_t));
    f->offset += sizeof(uint16_t);
    return value;
}

uint32_t read_u32(file_t* f)
{
    uint32_t value;
    memcpy(&value, &f->data[f->offset], sizeof(uint32_t));
    f->offset += sizeof(uint32_t);
    return value;
}

int8_t read_i8(file_t* f)
{
    int8_t value;
    memcpy(&value, &f->data[f->offset], sizeof(int8_t));
    f->offset += sizeof(int8_t);
    return value;
}

int16_t read_i16(file_t* f)
{
    int16_t value;
    memcpy(&value, &f->data[f->offset], sizeof(int16_t));
    f->offset += sizeof(int16_t);
    return value;
}

int32_t read_i32(file_t* f)
{
    int32_t value;
    memcpy(&value, &f->data[f->offset], sizeof(int32_t));
    f->offset += sizeof(int32_t);
    return value;
}

float read_f1x3x12(file_t* f)
{
    float value = read_i16(f);
    return value / 4096.0f;
}

static void read_sector(FILE* f, int32_t sector_num, uint8_t out_sector[static SECTOR_SIZE])
{
    int32_t seek_to = (sector_num * SECTOR_SIZE_RAW) + SECTOR_HEADER_SIZE;
    if (fseek(f, seek_to, SEEK_SET) != 0) {
        assert(false);
    }

    size_t n = fread(out_sector, sizeof(uint8_t), SECTOR_SIZE, f);
    assert(n == SECTOR_SIZE);
    return;
}
