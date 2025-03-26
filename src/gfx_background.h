#pragma once

#include "cglm/types-struct.h"

void gfx_background_init(void);
void gfx_background_shutdown(void);
void gfx_background_set(vec4s, vec4s);
void gfx_background_render(void);
