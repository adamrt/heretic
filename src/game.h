#ifndef GAME_H_
#define GAME_H_

#include "sokol_app.h"

void game_init(void);
void game_update(void);
void game_input(const sapp_event* event);
void game_shutdown(void);

typedef enum {
    MODE_MAP,
    MODE_SCENARIO,
} mode_e;

typedef struct {
    mode_e mode;
} game_t;

extern game_t gstate;

#endif // GAME_H_
