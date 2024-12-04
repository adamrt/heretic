#ifndef BIN_H_
#define BIN_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Simple buffer struct to allow incremental reading of different types.
// Typically useful for reading files from the original PSX bin.
typedef struct {
    uint8_t* data;
    size_t offset;
    size_t size;
} buffer_t;

void bin_init(void);
void bin_shutdown(void);
bool bin_is_loaded(void);

buffer_t read_file(int, int);

void read_bytes(buffer_t*, int, uint8_t*);
uint8_t read_u8(buffer_t*);
uint16_t read_u16(buffer_t*);
uint32_t read_u32(buffer_t*);
int8_t read_i8(buffer_t*);
int16_t read_i16(buffer_t*);
int32_t read_i32(buffer_t*);
float read_f1x3x12(buffer_t*);

buffer_t read_file_test_evt(void);
buffer_t read_file_attack_out(void);

#endif // BIN_H_
