#pragma once

#include "sokol_app.h"

void gui_init(void);
void gui_shutdown(void);
void gui_update(void);
bool gui_input(const sapp_event* event);
