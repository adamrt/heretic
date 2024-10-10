#ifndef BIN_H_
#define BIN_H_

#include <stdint.h>
#include <stdio.h>

#include "model.h"

typedef struct {
    uint8_t id;
    uint16_t sector;
    bool valid;
    const char* name;
} map_t;

extern map_t map_list[];

model_t read_map(FILE* bin, int num);

#endif // BIN_H_
