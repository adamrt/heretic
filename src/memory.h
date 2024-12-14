#pragma once

#include "defines.h"

void memory_init(void);
void memory_shutdown(void);

void* memory_allocate(usize size);
void memory_free(void* ptr);
