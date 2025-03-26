#include "memory.h"
#include "util.h"

memory_stats_t memory_state;

typedef struct allocation_header {
    usize size;
    int line;
    const char* file;
    struct allocation_header* next;
} allocation_header_t;

static allocation_header_t* allocations_head = NULL;

void memory_init(void) {
    memory_state.usage_peak = 0;
    memory_state.usage_total = 0;
    memory_state.usage_current = 0;
    memory_state.allocations_total = 0;
    memory_state.allocations_current = 0;
}

void memory_shutdown(void) {
    if (memory_state.allocations_current != 0) {
        printf("Memory leak detected: %zu allocations remaining\n", memory_state.allocations_current);
        allocation_header_t* current = allocations_head;
        while (current) {
            printf("Leaked %zu bytes allocated from %s:%d\n", current->size, current->file, current->line);
            current = current->next;
        }
    }
    if (memory_state.usage_current != 0) {
        printf("Memory leak detected: %zu bytes remaining\n", memory_state.usage_current);
    }
    printf("Memory usage peak: %0.2fMB\n", BYTES_TO_MB(memory_state.usage_peak));
    printf("Memory usage total: %0.2fMB\n", BYTES_TO_MB(memory_state.usage_total));
    printf("Memory allocations: %zu\n", memory_state.allocations_total);
}

void* memory_allocate_impl(usize size, const char* file, int line) {
    allocation_header_t* header = calloc(1, sizeof(allocation_header_t) + size);
    ASSERT(header != NULL, "Failed to allocate memory");

    header->size = size;
    header->file = file;
    header->line = line;

    header->next = allocations_head;
    allocations_head = header;

    memory_state.usage_current += size;
    memory_state.usage_peak = MAX(memory_state.usage_peak, memory_state.usage_current);
    memory_state.usage_total += size;
    memory_state.allocations_total++;
    memory_state.allocations_current++;

    return (void*)(header + 1);
}

void memory_free(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    allocation_header_t* header = ((allocation_header_t*)ptr) - 1;

    // Remove from the linked list
    allocation_header_t** current = &allocations_head;
    while (*current) {
        if (*current == header) {
            *current = header->next;
            break;
        }
        current = &((*current)->next);
    }

    memory_state.allocations_current--;
    memory_state.usage_current -= header->size;

    free(header);
}
