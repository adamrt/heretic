#ifndef UI_H_
#define UI_H_

#include "sokol_app.h"

void ui_init(void);
void ui_update(void);
bool ui_input(const sapp_event* event);
void ui_shutdown(void);

#endif // UI_H_
