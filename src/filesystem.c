#include <math.h>
#include <string.h>

#include "filesystem.h"
#include "memory.h"
#include "util.h"

static struct {
    FILE* file;

    // This is a cache of the files that have been read from the filesystem.
    // This is useful for lazy loading files and not having to read the same
    // file multiple times.
    u8* cache[F_FILE_COUNT];
    usize cached_count;
    usize cached_size;
} _state;

usize filesystem_cached_count(void) { return _state.cached_count; }
usize filesystem_cached_size(void) { return _state.cached_size; }

// This is a list of description for all files in the filesystem.
//
// Examples:
//   [F_BATTLE_BIN] =      { .sector = 1000,  .size = 1397096, .name = "BATTLE.BIN"},
//   [F_EVENT__TEST_EVT] = { .sector = 3707,  .size = 4096000, .name = "EVENT/TEST.EVT"},
//   [F_MAP__MAP001_GNS] = { .sector = 11304, .size = 2388,    .name = "MAP/MAP000.GNS"},
//
const file_desc_t file_list[F_FILE_COUNT] = {
#define X(oname, osector, osize, opath) [oname] = { .sector = osector, .size = osize, .name = opath },
    FILESYSTEM_INDEX
#undef X
};

// Forward declarations
static void _read_file(file_entry_e, u8*);

void filesystem_init(void) {
    _state.file = fopen("fft.bin", "rb");
    ASSERT(_state.file != NULL, "Failed to open fft.bin");
}

void filesystem_shutdown(void) {
    for (usize i = 0; i < F_FILE_COUNT; i++) {
        if (_state.cache[i] != NULL) {
            memory_free(_state.cache[i]);
        }
    }
    fclose(_state.file);
}

span_t filesystem_read_file(file_entry_e file) {
    if (_state.cache[file] == NULL) {
        u8* bytes = memory_allocate(file_list[file].size);
        _read_file(file, bytes);
        _state.cache[file] = bytes;

        _state.cached_count++;
        _state.cached_size += file_list[file].size;
    }

    span_t span = (span_t) {
        .data = _state.cache[file],
        .size = file_list[file].size,
    };

    return span;
}

file_entry_e filesystem_entry_by_sector(u32 sector) {
    for (usize i = 0; i < F_FILE_COUNT; i++) {
        if (file_list[i].sector == sector) {
            return (file_entry_e)i;
        }
    }
    ASSERT(false, "Failed to find file by sector %d", sector);
}

static void _read_file(file_entry_e file, u8* out_bytes) {
    constexpr int sector_header_size = 24;
    constexpr int sector_size = 2048;
    constexpr int sector_size_raw = 2352;

    file_desc_t desc = file_list[file];

    usize offset = 0;
    usize occupied_sectors = ceil(desc.size / (f64)sector_size);

    for (usize i = 0; i < occupied_sectors; i++) {
        usize seek_to = ((desc.sector + i) * sector_size_raw) + sector_header_size;
        usize sn = fseek(_state.file, seek_to, SEEK_SET);
        ASSERT(sn == 0, "Failed to seek to sector");

        u8 sector[sector_size];
        usize rn = fread(sector, sizeof(u8), sector_size, _state.file);
        ASSERT(rn == sector_size, "Failed to read correct number of bytes from sector");

        usize remaining_size = desc.size - offset;
        usize bytes_to_copy = (remaining_size < sector_size) ? remaining_size : sector_size;

        memcpy(out_bytes + offset, sector, bytes_to_copy);
        offset += bytes_to_copy;
    }

    return;
}
