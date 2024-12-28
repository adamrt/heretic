#pragma once

#include "defines.h"
#include "filesystem_index.h"
#include "span.h"

typedef struct {
    u32 sector;
    u32 size;
    char* name;
} file_desc_t;

// This is an enum of all files in the filesystem. This is useful for
// referencing files in the filesystem and allowing indexing into file_list
//
// Examples:
//   F_BATTLE_BIN,
//   F_EVENT__TEST_EVT,
//   F_MAP__MAP000_GNS,
//
typedef enum {
#define X(name, sector, size, path) name,
    FILESYSTEM_INDEX
#undef X
        F_FILE_COUNT // Automatically represents the count of files
} file_entry_e;

void filesystem_init(void);
void filesystem_shutdown(void);
span_t filesystem_read_file(file_entry_e);
file_entry_e filesystem_entry_by_sector(u32);

extern const file_desc_t file_list[F_FILE_COUNT];
