#include <math.h>
#include <string.h>

#include "io.h"
#include "util.h"

#define SECTOR_HEADER_SIZE (24)
#define SECTOR_SIZE        (2048)
#define SECTOR_SIZE_RAW    (2352)

static struct {
    FILE* bin;

    buffer_t test_evt;
    buffer_t attack_out;
} _state;

void io_init(void)
{
    _state.bin = fopen("../fft.bin", "rb");
    ASSERT(_state.bin != NULL, "Failed to open fft.bin");

    // Cache the files immediately.
    _state.test_evt = read_file_test_evt();
    _state.attack_out = read_file_attack_out();
}

void io_shutdown(void)
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

buffer_t io_read_file(usize sector_num, usize size)
{
    buffer_t file = { .size = size };
    file.data = calloc(1, size);

    usize offset = 0;
    usize occupied_sectors = ceil(size / (f64)SECTOR_SIZE);

    for (usize i = 0; i < occupied_sectors; i++) {
        usize seek_to = ((sector_num + i) * SECTOR_SIZE_RAW) + SECTOR_HEADER_SIZE;
        usize sn = fseek(_state.bin, seek_to, SEEK_SET);
        ASSERT(sn == 0, "Failed to seek to sector");

        u8 sector[SECTOR_SIZE];
        usize rn = fread(sector, sizeof(u8), SECTOR_SIZE, _state.bin);
        ASSERT(rn == SECTOR_SIZE, "Failed to read correct number of bytes from sector");

        usize remaining_size = size - offset;
        usize bytes_to_copy = (remaining_size < SECTOR_SIZE) ? remaining_size : SECTOR_SIZE;

        memcpy(file.data + offset, sector, bytes_to_copy);
        offset += bytes_to_copy;
    }

    return file;
}

// Read the EVENT/TEST.EVT file.
buffer_t read_file_test_evt(void)
{
    const int test_evt_sector = 3707;
    const int test_evt_size = 4096000;

    if (_state.test_evt.data == NULL) {
        _state.test_evt = io_read_file(test_evt_sector, test_evt_size);
    }
    return _state.test_evt;
}

// Read the ATTACK.OUT file.
buffer_t read_file_attack_out(void)
{
    const int attack_out_sector = 2448;
    const int attack_out_size = 125956;

    if (_state.attack_out.data == NULL) {
        _state.attack_out = io_read_file(attack_out_sector, attack_out_size);
    }
    return _state.attack_out;
}
