#ifndef GAME_H_
#define GAME_H_

#include "sokol_app.h"

#include "model.h"

void game_init(void);
void game_update(void);
void game_input(const sapp_event* event);
void game_shutdown(void);

typedef struct {
    FILE* bin;
    scene_t scene;

    struct {
        bool mouse_left;
        bool mouse_right;
    } input;
} game_t;

extern game_t game;

#endif // GAME_H_
