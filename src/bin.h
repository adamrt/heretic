#ifndef BIN_H_
#define BIN_H_

#include <stdint.h>
#include <stdio.h>

#include "model.h"

model_t read_map(FILE* bin, int num);

typedef struct {
    uint8_t id;
    uint16_t sector;
    bool valid;
    const char* name;
} map_desc_t;

extern map_desc_t map_list[];

#endif // BIN_H_
