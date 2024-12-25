#include <math.h>
#include <string.h>

#include "io.h"
#include "util.h"

#define SECTOR_HEADER_SIZE (24)
#define SECTOR_SIZE        (2048)
#define SECTOR_SIZE_RAW    (2352)

// Individual file lengths
#define ATTACK_OUT_SECTOR  (2448)
#define ATTACK_OUT_LEN     (125956)
#define TEST_EVT_SECTOR    (3707)
#define TEST_EVT_LEN       (4096000)
#define FONT_BIN_SECTOR    (3650)
#define FONT_BIN_LEN       (77000)
#define FRAME_BIN_LEN      (37568)
#define FRAME_BIN_SECTOR   (3688)
#define ITEM_BIN_SECTOR    (6297)
#define ITEM_BIN_LEN       (33280)
#define EVTFACE_BIN_SECTOR (5707)
#define EVTFACE_BIN_LEN    (65536)

static struct {
    FILE* file;

    u8 test_evt[TEST_EVT_LEN];
    u8 attack_out[ATTACK_OUT_LEN];
    u8 font_bin[FONT_BIN_LEN];
    u8 frame_bin[FRAME_BIN_LEN];
    u8 item_bin[ITEM_BIN_LEN];
    u8 evtface_bin[EVTFACE_BIN_LEN];
} _state;

void io_init(void) {
    _state.file = fopen("fft.bin", "rb");
    ASSERT(_state.file != NULL, "Failed to open fft.bin");

    // Read all files
    io_read_file(ATTACK_OUT_SECTOR, ATTACK_OUT_LEN, _state.attack_out);
    io_read_file(TEST_EVT_SECTOR, TEST_EVT_LEN, _state.test_evt);
    io_read_file(FONT_BIN_SECTOR, FONT_BIN_LEN, _state.font_bin);
    io_read_file(FRAME_BIN_SECTOR, FRAME_BIN_LEN, _state.frame_bin);
    io_read_file(ITEM_BIN_SECTOR, ITEM_BIN_LEN, _state.item_bin);
    io_read_file(EVTFACE_BIN_SECTOR, EVTFACE_BIN_LEN, _state.evtface_bin);
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

bytes_t io_read_file_bytes(usize sector_num, usize size) {
    ASSERT(size <= BYTES_MAX, "Size is too large for bytes_t");
    usize offset = 0;
    usize occupied_sectors = ceil(size / (f64)SECTOR_SIZE);

    bytes_t bytes = { .size = size };
    for (usize i = 0; i < occupied_sectors; i++) {
        usize seek_to = ((sector_num + i) * SECTOR_SIZE_RAW) + SECTOR_HEADER_SIZE;
        usize sn = fseek(_state.file, seek_to, SEEK_SET);
        ASSERT(sn == 0, "Failed to seek to sector");

        u8 sector[SECTOR_SIZE];
        usize rn = fread(sector, sizeof(u8), SECTOR_SIZE, _state.file);
        ASSERT(rn == SECTOR_SIZE, "Failed to read correct number of bytes from sector");

        usize remaining_size = size - offset;
        usize bytes_to_copy = (remaining_size < SECTOR_SIZE) ? remaining_size : SECTOR_SIZE;

        memcpy(bytes.data + offset, sector, bytes_to_copy);
        offset += bytes_to_copy;
    }
    return bytes;
}

// Return preloaded files
span_t io_file_test_evt(void) { return (span_t) { .data = _state.test_evt, .size = TEST_EVT_LEN }; }
span_t io_file_attack_out(void) { return (span_t) { .data = _state.attack_out, .size = ATTACK_OUT_LEN }; }
span_t io_file_font_bin(void) { return (span_t) { .data = _state.font_bin, .size = FONT_BIN_LEN }; }
span_t io_file_frame_bin(void) { return (span_t) { .data = _state.frame_bin, .size = FRAME_BIN_LEN }; }
span_t io_file_item_bin(void) { return (span_t) { .data = _state.item_bin, .size = ITEM_BIN_LEN }; }
span_t io_file_evtface_bin(void) { return (span_t) { .data = _state.evtface_bin, .size = EVTFACE_BIN_LEN }; }
