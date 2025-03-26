#pragma once

#include "sokol_gfx.h"

#include "cglm/types-struct.h"

void gfx_line_init(void);
void gfx_line_shutdown(void);
void gfx_line_render(sg_buffer, vec4s);
void gfx_line_render_axis(void);
