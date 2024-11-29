#ifndef BIN_H_
#define BIN_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t* data;
    size_t offset;
    size_t size;
} file_t;

void bin_init(void);
void bin_shutdown(void);
bool bin_is_loaded(void);

file_t read_file(int, int);

void read_bytes(file_t*, int, uint8_t*);
uint8_t read_u8(file_t*);
uint16_t read_u16(file_t*);
uint32_t read_u32(file_t*);
int8_t read_i8(file_t*);
int16_t read_i16(file_t*);
int32_t read_i32(file_t*);
float read_f1x3x12(file_t*);

file_t read_file_test_evt(void);
file_t read_file_attack_out(void);

#endif // BIN_H_
