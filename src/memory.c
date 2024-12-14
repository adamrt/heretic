#include "memory.h"
#include "util.h"

static struct {
    usize memory_usage_peak;
    usize memory_usage_total;
    usize memory_usage_current;
    usize memory_allocations_total;
    usize memory_allocations_current;
} _state;

typedef struct {
    usize size;
    usize padding; // Ensure 16-byte alignment
} allocation_header_t;

void memory_init(void)
{
    // Useless due to static allocation, but nothing else to go here :)
    _state.memory_usage_peak = 0;
    _state.memory_usage_total = 0;
    _state.memory_usage_current = 0;
    _state.memory_allocations_total = 0;
    _state.memory_allocations_current = 0;
}

void memory_shutdown(void)
{
    if (_state.memory_allocations_current != 0) {
        printf("Memory leak detected: %zu allocations remaining\n", _state.memory_allocations_current);
    }
    if (_state.memory_usage_current != 0) {
        printf("Memory leak detected: %zu bytes remaining\n", _state.memory_usage_current);
    }
    printf("Memory usage peak: %0.2fMB\n", BYTES_TO_MB(_state.memory_usage_peak));
    printf("Memory usage total: %0.2fMB\n", BYTES_TO_MB(_state.memory_usage_total));
    printf("Memory allocations: %zu\n", _state.memory_allocations_total);
}

void* memory_allocate(usize size)
{
    allocation_header_t* header = calloc(1, sizeof(allocation_header_t) + size);
    ASSERT(header != NULL, "Failed to allocate memory");

    header->size = size;

    _state.memory_usage_current += size;
    _state.memory_usage_peak = MAX(_state.memory_usage_peak, _state.memory_usage_current);
    _state.memory_usage_total += size;
    _state.memory_allocations_total++;
    _state.memory_allocations_current++;

    return (void*)(header + 1);
}
void memory_free(void* ptr)
{
    ASSERT(ptr != NULL, "Attempted to free NULL pointer");

    allocation_header_t* header = ((allocation_header_t*)ptr) - 1;

    _state.memory_allocations_current--;
    _state.memory_usage_current -= header->size;

    free(header);
}
