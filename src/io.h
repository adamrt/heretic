#pragma once

#include "defines.h"
#include "span.h"

void io_init(void);
void io_shutdown(void);
void io_read_file(usize, usize, u8*);

// Return preloaded files.
span_t io_file_test_evt(void);
span_t io_file_attack_out(void);
span_t io_file_font_bin(void);
span_t io_file_frame_bin(void);
