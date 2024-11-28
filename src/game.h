#ifndef GAME_H_
#define GAME_H_

#include "sokol_app.h"

void game_init(void);
void game_shutdown(void);
void game_update(void);
void game_input(const sapp_event* event);

#endif // GAME_H_
