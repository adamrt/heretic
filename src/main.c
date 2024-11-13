#include "sokol_app.h"
#include "sokol_log.h"

#include "game.h"
#include "gfx.h"

sapp_desc sokol_main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    return (sapp_desc) {
        .init_cb = game_init,
        .event_cb = game_input,
        .frame_cb = game_update,
        .cleanup_cb = game_shutdown,
        .width = GFX_DISPLAY_WIDTH,
        .height = GFX_DISPLAY_HEIGHT,
        .window_title = "Heretic",
        .icon.sokol_default = true,
        .logger.func = slog_func,
    };
}
