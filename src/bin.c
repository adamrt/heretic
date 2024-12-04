#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bin.h"
#include "util.h"

static struct {
    FILE* bin; // The FFT bin file.

    buffer_t test_evt;   // The EVENT/TEST.EVT file.
    buffer_t attack_out; // The ATTACK.OUT file.
} _state;

#define SECTOR_HEADER_SIZE (24)
#define SECTOR_SIZE        (2048)
#define SECTOR_SIZE_RAW    (2352)

#define FILE_SIZE_MAX (131072)

void bin_init(void)
{
    _state.bin = fopen("../fft.bin", "rb");
    ASSERT(_state.bin != NULL, "Failed to open fft.bin");

    // Cache the files immediately.
    _state.test_evt = read_file_test_evt();
    _state.attack_out = read_file_attack_out();
}

bool bin_is_loaded(void)
{
    return _state.bin != NULL;
}

void bin_shutdown(void)
{
    fclose(_state.bin);
    _state.bin = NULL;

    if (_state.test_evt.data != NULL) {
        free(_state.test_evt.data);
    }
    if (_state.attack_out.data != NULL) {
        free(_state.attack_out.data);
    }
}

// Forward declarations
static void _read_sector(int32_t sector_num, uint8_t out_sector[static SECTOR_SIZE]);

buffer_t read_file(int sector_num, int size)
{
    buffer_t file = { .size = size };
    file.data = calloc(1, size);

    int offset = 0;
    uint32_t occupied_sectors = ceil((double)size / (double)SECTOR_SIZE);

    for (uint32_t i = 0; i < occupied_sectors; i++) {
        uint8_t sector[SECTOR_SIZE];
        _read_sector(sector_num + i, sector);

        int remaining_size = size - offset;
        int bytes_to_copy = (remaining_size < SECTOR_SIZE) ? remaining_size : SECTOR_SIZE;

        memcpy(file.data + offset, sector, bytes_to_copy);
        offset += bytes_to_copy;
    }

    return file;
}

void read_bytes(buffer_t* f, int size, uint8_t* out_bytes)
{
    ASSERT(size < FILE_SIZE_MAX, "File size too large");
    for (int i = 0; i < size; i++) {
        out_bytes[i] = read_u8(f);
    }
    return;
}

uint8_t read_u8(buffer_t* f)
{
    uint8_t value;
    memcpy(&value, &f->data[f->offset], sizeof(uint8_t));
    f->offset += sizeof(uint8_t);
    return value;
}

uint16_t read_u16(buffer_t* f)
{
    uint16_t value;
    memcpy(&value, &f->data[f->offset], sizeof(uint16_t));
    f->offset += sizeof(uint16_t);
    return value;
}

uint32_t read_u32(buffer_t* f)
{
    uint32_t value;
    memcpy(&value, &f->data[f->offset], sizeof(uint32_t));
    f->offset += sizeof(uint32_t);
    return value;
}

int8_t read_i8(buffer_t* f)
{
    int8_t value;
    memcpy(&value, &f->data[f->offset], sizeof(int8_t));
    f->offset += sizeof(int8_t);
    return value;
}

int16_t read_i16(buffer_t* f)
{
    int16_t value;
    memcpy(&value, &f->data[f->offset], sizeof(int16_t));
    f->offset += sizeof(int16_t);
    return value;
}

int32_t read_i32(buffer_t* f)
{
    int32_t value;
    memcpy(&value, &f->data[f->offset], sizeof(int32_t));
    f->offset += sizeof(int32_t);
    return value;
}

float read_f1x3x12(buffer_t* f)
{
    float value = read_i16(f);
    return value / 4096.0f;
}

// Read the EVENT/TEST.EVT file.
buffer_t read_file_test_evt(void)
{
    const int test_evt_sector = 3707;
    const int test_evt_size = 4096000;

    if (_state.test_evt.data == NULL) {
        _state.test_evt = read_file(test_evt_sector, test_evt_size);
    }
    return _state.test_evt;
}

// Read the ATTACK.OUT file.
buffer_t read_file_attack_out(void)
{
    const int attack_out_sector = 2448;
    const int attack_out_size = 125956;

    if (_state.attack_out.data == NULL) {
        _state.attack_out = read_file(attack_out_sector, attack_out_size);
    }
    return _state.attack_out;
}

static void _read_sector(int32_t sector_num, uint8_t out_sector[static SECTOR_SIZE])
{
    int32_t seek_to = (sector_num * SECTOR_SIZE_RAW) + SECTOR_HEADER_SIZE;
    size_t n = fseek(_state.bin, seek_to, SEEK_SET);
    ASSERT(n == 0, "Failed to seek to sector");

    n = fread(out_sector, sizeof(uint8_t), SECTOR_SIZE, _state.bin);
    ASSERT(n == SECTOR_SIZE, "Failed to read correct number of bytes from sector");
}
