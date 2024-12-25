#pragma once

#include "defines.h"
#include "span.h"

void io_init(void);
void io_shutdown(void);
void io_read_file(usize, usize, u8*);
bytes_t io_read_file_bytes(usize, usize);

// Return preloaded files.
span_t io_file_test_evt(void);
span_t io_file_attack_out(void);
span_t io_file_font_bin(void);
span_t io_file_frame_bin(void);
span_t io_file_item_bin(void);
span_t io_file_evtface_bin(void);
