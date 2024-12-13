#pragma once

#include "bin.h"
#include "defines.h"

void io_init(void);
void io_shutdown(void);
buffer_t io_read_file(usize, usize);

buffer_t read_file_test_evt(void);
buffer_t read_file_attack_out(void);
