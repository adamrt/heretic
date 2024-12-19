#include <math.h>
#include <string.h>

#include "event.h"
#include "io.h"
#include "memory.h"
#include "scenario.h"
#include "util.h"

#define SECTOR_HEADER_SIZE (24)
#define SECTOR_SIZE        (2048)
#define SECTOR_SIZE_RAW    (2352)

// Individual file lengths
#define ATTACK_OUT_LEN (125956)
#define TEST_EVT_LEN   (4096000)
#define FONT_BIN_LEN   (77000)
#define FRAME_BIN_LEN  (37568)

static struct {
    FILE* file;

    u8 test_evt[TEST_EVT_LEN];
    u8 attack_out[ATTACK_OUT_LEN];
    u8 font_bin[FONT_BIN_LEN];
    u8 frame_bin[FRAME_BIN_LEN];
} _state;

void io_init(void) {
    _state.file = fopen("../fft.bin", "rb");
    ASSERT(_state.file != NULL, "Failed to open fft.bin");

    // Read all files
    io_read_file(2448, ATTACK_OUT_LEN, _state.attack_out);
    io_read_file(3707, TEST_EVT_LEN, _state.test_evt);
    io_read_file(3650, FONT_BIN_LEN, _state.font_bin);
    io_read_file(3688, FRAME_BIN_LEN, _state.frame_bin);
}

void io_shutdown(void) {
    fclose(_state.file);
}

void io_read_file(usize sector_num, usize size, u8* out_bytes) {
    usize offset = 0;
    usize occupied_sectors = ceil(size / (f64)SECTOR_SIZE);

    for (usize i = 0; i < occupied_sectors; i++) {
        usize seek_to = ((sector_num + i) * SECTOR_SIZE_RAW) + SECTOR_HEADER_SIZE;
        usize sn = fseek(_state.file, seek_to, SEEK_SET);
        ASSERT(sn == 0, "Failed to seek to sector");

        u8 sector[SECTOR_SIZE];
        usize rn = fread(sector, sizeof(u8), SECTOR_SIZE, _state.file);
        ASSERT(rn == SECTOR_SIZE, "Failed to read correct number of bytes from sector");

        usize remaining_size = size - offset;
        usize bytes_to_copy = (remaining_size < SECTOR_SIZE) ? remaining_size : SECTOR_SIZE;

        memcpy(out_bytes + offset, sector, bytes_to_copy);
        offset += bytes_to_copy;
    }

    return;
}

// Return preloaded files
span_t io_file_test_evt(void) { return (span_t) { .data = _state.test_evt, .size = TEST_EVT_LEN }; }
span_t io_file_attack_out(void) { return (span_t) { .data = _state.attack_out, .size = ATTACK_OUT_LEN }; }
span_t io_file_font_bin(void) { return (span_t) { .data = _state.font_bin, .size = FONT_BIN_LEN }; }
span_t io_file_frame_bin(void) { return (span_t) { .data = _state.frame_bin, .size = FRAME_BIN_LEN }; }
