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

#define memory_allocate(size) memory_allocate_impl(size, __FILE__, __LINE__)

void memory_init(void);
void memory_shutdown(void);
void* memory_allocate_impl(usize size, const char* file, int line);
void memory_free(void* ptr);
