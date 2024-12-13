#pragma once

#include "defines.h"

// file_t just wraps data. This is only useful so we can pass back the size with
// the data.
typedef struct {
    u8* data;
    usize size;
} file_t;

void io_init(void);
void io_shutdown(void);
void io_read_file(usize, usize, u8*);

// Return preloaded files.
file_t io_file_test_evt(void);
file_t io_file_attack_out(void);
