#ifndef GUI_H_
#define GUI_H_

#include "sokol_app.h"

void gui_init(void);
void gui_update(void);
bool gui_input(const sapp_event* event);
void gui_shutdown(void);

#endif // GUI_H_
