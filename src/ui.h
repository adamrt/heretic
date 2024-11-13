#ifndef UI_H_
#define UI_H_

#include "nuklear.h"

void ui_init(void);
void ui_frame(void);
void ui_draw(struct nk_context* ctx);

#endif // UI_H_
