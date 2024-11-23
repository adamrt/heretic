#ifndef TEXTURE_H_
#define TEXTURE_H_

#include <stdbool.h>
#include <stdint.h>

#include "bin.h"
#include "map_record.h"

#define TEXTURE_WIDTH     (256)
#define TEXTURE_HEIGHT    (1024)
#define TEXTURE_SIZE      (TEXTURE_WIDTH * TEXTURE_HEIGHT) // 262144
#define TEXTURE_BYTE_SIZE (TEXTURE_SIZE * 4)

#define PALETTE_WIDTH     (256)
#define PALETTE_HEIGHT    (1)
#define PALETTE_SIZE      (PALETTE_WIDTH * PALETTE_HEIGHT)
#define PALETTE_BYTE_SIZE (PALETTE_SIZE * 4)

typedef struct {
    map_state_t map_state;
    uint8_t data[TEXTURE_BYTE_SIZE];
    bool valid;
} texture_t;

typedef struct {
    uint8_t data[PALETTE_BYTE_SIZE];
    bool valid;
} palette_t;

texture_t read_texture(file_t*);
palette_t read_palette(file_t*);

#endif // TEXTURE_H_
