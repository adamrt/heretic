// This file lets us compile nuklear as a shared obj
#if defined(_MSC_VER)
#    define SOKOL_D3D11
#elif defined(__EMSCRIPTEN__)
#    define SOKOL_GLES3
#elif defined(__APPLE__)
// NOTE: on macOS, sokol.c is compiled explicitly as ObjC
#    define SOKOL_METAL
#else
#    define SOKOL_GLCORE
#endif

#define SOKOL_IMGUI_IMPL

#include "dcimgui/src-docking/cimgui.h"

#include "sokol/sokol_app.h"
#include "sokol/sokol_audio.h"
#include "sokol/sokol_gfx.h"
#include "sokol/util/sokol_imgui.h"
