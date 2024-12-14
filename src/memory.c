#include "memory.h"
#include "util.h"

memory_stats_t memory_state;

typedef struct {
    usize size;
    usize padding; // Ensure 16-byte alignment
} allocation_header_t;

void memory_init(void)
{
    memory_state.usage_peak = 0;
    memory_state.usage_total = 0;
    memory_state.usage_current = 0;
    memory_state.allocations_total = 0;
    memory_state.allocations_current = 0;
}

void memory_shutdown(void)
{
    if (memory_state.allocations_current != 0) {
        printf("Memory leak detected: %zu allocations remaining\n", memory_state.allocations_current);
    }
    if (memory_state.usage_current != 0) {
        printf("Memory leak detected: %zu bytes remaining\n", memory_state.usage_current);
    }
    printf("Memory usage peak: %0.2fMB\n", BYTES_TO_MB(memory_state.usage_peak));
    printf("Memory usage total: %0.2fMB\n", BYTES_TO_MB(memory_state.usage_total));
    printf("Memory allocations: %zu\n", memory_state.allocations_total);
}

void* memory_allocate(usize size)
{
    allocation_header_t* header = calloc(1, sizeof(allocation_header_t) + size);
    ASSERT(header != NULL, "Failed to allocate memory");

    header->size = size;

    memory_state.usage_current += size;
    memory_state.usage_peak = MAX(memory_state.usage_peak, memory_state.usage_current);
    memory_state.usage_total += size;
    memory_state.allocations_total++;
    memory_state.allocations_current++;

    return (void*)(header + 1);
}
void memory_free(void* ptr)
{
    ASSERT(ptr != NULL, "Attempted to free NULL pointer");

    allocation_header_t* header = ((allocation_header_t*)ptr) - 1;

    memory_state.allocations_current--;
    memory_state.usage_current -= header->size;

    free(header);
}
