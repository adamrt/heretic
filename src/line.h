#pragma once

#include "sokol_gfx.h"

#include "cglm/types-struct.h"

void line_init(void);
void line_shutdown(void);
void line_render(sg_buffer, vec4s);
void line_render_axis(void);
