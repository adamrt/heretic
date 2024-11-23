// This file lets us compile sokol as a shared obj
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

#define SOKOL_IMPL
#include "sokol_app.h"
#include "sokol_audio.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"
#include "sokol_time.h"
