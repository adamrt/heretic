#pragma once

#include "sokol_app.h"

void game_init(void);
void game_shutdown(void);
void game_update(void);
void game_input(const sapp_event* event);
