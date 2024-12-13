#include <math.h>
#include <string.h>

#include "io.h"
#include "util.h"

#define SECTOR_HEADER_SIZE (24)
#define SECTOR_SIZE        (2048)
#define SECTOR_SIZE_RAW    (2352)

// Individual file lengths
#define ATTACK_OUT_LEN (125956)
#define TEST_EVT_LEN   (4096000)

static struct {
    FILE* bin;

    u8* test_evt;
    u8* attack_out;
} _state;

void io_init(void)
{
    _state.bin = fopen("../fft.bin", "rb");
    ASSERT(_state.bin != NULL, "Failed to open fft.bin");

    // Read files into memory.
    _state.test_evt = calloc(1, TEST_EVT_LEN);
    _state.attack_out = calloc(1, ATTACK_OUT_LEN);
    io_read_file(2448, ATTACK_OUT_LEN, _state.attack_out);
    io_read_file(3707, TEST_EVT_LEN, _state.test_evt);
}

void io_shutdown(void)
{
    fclose(_state.bin);
    free(_state.test_evt);
    free(_state.attack_out);
}

void io_read_file(usize sector_num, usize size, u8* out_bytes)
{
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

        memcpy(out_bytes + offset, sector, bytes_to_copy);
        offset += bytes_to_copy;
    }

    return;
}

file_t io_file_test_evt(void) { return (file_t) { _state.test_evt, TEST_EVT_LEN }; }
file_t io_file_attack_out(void) { return (file_t) { _state.attack_out, ATTACK_OUT_LEN }; }
