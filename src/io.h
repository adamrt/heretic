#pragma once

#include "defines.h"
#include "event.h"
#include "scenario.h"
#include "span.h"

void io_init(void);
void io_shutdown(void);
void io_read_file(usize, usize, u8*);

event_t io_get_event(int);
scenario_t io_get_scenario(int);

// Return preloaded files.
span_t io_file_test_evt(void);
span_t io_file_attack_out(void);
span_t io_file_font_bin(void);
