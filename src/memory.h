#pragma once

#include "defines.h"

typedef struct {
    usize usage_peak;
    usize usage_total;
    usize usage_current;
    usize allocations_total;
    usize allocations_current;
} memory_stats_t;

extern memory_stats_t memory_state;

void memory_init(void);
void memory_shutdown(void);
void* memory_allocate(usize size);
void memory_free(void* ptr);
