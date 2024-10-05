// This file lets us compile sokol as a shared obj
#define SOKOL_IMPL
#if defined(_MSC_VER)
#define SOKOL_D3D11
#elif defined(__EMSCRIPTEN__)
#define SOKOL_GLES3
#elif defined(__APPLE__)
// NOTE: on macOS, sokol.c is compiled explicitly as ObjC
#define SOKOL_METAL
// #define SOKOL_GLCORE
#else
#define SOKOL_GLCORE
#endif
#include "sokol_app.h"
#include "sokol_audio.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#define NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_STANDARD_VARARGS
#include "nuklear/nuklear.h"
#define SOKOL_NUKLEAR_IMPL
#include "sokol_nuklear.h"
