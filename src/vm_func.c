#include "vm_func.h"

#include "camera.h"
#include "dialog.h"
#include "gfx.h"
#include "gfx_model.h"
#include "gfx_sprite.h"
#include "parse.h"
#include "scene.h"
#include "vm.h"
#include "vm_message.h"
#include "vm_transition.h"

void fn_display_message(const instruction_t* instr) {
    scene_t* scene = scene_get_internals();

    (void)instr->params[0].value.u8; // unused, typically 0x10
    dialog_t dialog = parse_dialog(instr->params[1].value.u8);
    u16 message_id = instr->params[2].value.u16;
    u8 unit_id = instr->params[3].value.u8;
    (void)instr->params[4].value.u8; // unused, always 0x00
    u8 portrait_row = instr->params[5].value.u8;
    i16 x = instr->params[6].value.i16;
    i16 y = instr->params[7].value.i16;
    i16 arrow_pos = instr->params[8].value.i16;
    dialog_opening_t opening = parse_dialog_opening(instr->params[9].value.u8);

    char text[512];
    message_by_index(scene->event.messages, message_id, text);

    int speed = 10;
    if (opening.speed == DIALOG_SPEED_PLUS_50) {
        speed = 15;
    } else if (opening.speed == DIALOG_SPEED_MINUS_50) {
        speed = 5;
    }

    x = GFX_RENDER_WIDTH / 2 + x;
    y = GFX_RENDER_HEIGHT / 2 + y;

    sprite_t* sprites = gfx_sprite_get_internals();
    texture_t texture = sprite_get_paletted_texture(F_EVENT__FRAME_BIN, 0);
    sprite_t* sprite = &sprites[0];

    transform_t transform = {
        .translation = { { x, y, 0.0f } },
        .rotation = { { 0.0f, 0.0f, 0.0f } },
        .scale = { { 20.0f, 20.0f, 20.0f } },
    };
    *sprite = gfx_sprite_create(SPRITE_2D, texture, (vec2s) { { 0.0f, 0.0f } }, (vec2s) { { 32, 32 } }, transform);

    transition_add(instr->opcode, &sprite->transform.scale, 20.0f, 80.0f, speed);
    transition_add(instr->opcode, &sprite->transform.scale, 20.0f, 40.0f, speed);

    (void)unit_id;
    (void)portrait_row;
    /* sprite2d_t* portrait = &sprites[1]; */
    /* texture_t portrait_texture = sprite_get_evtface_bin_texture(portrait_row + 1, 0); */
    /* *portrait = gfx_sprite2d_create(portrait_texture, (vec2s) { { unit_id * 32.0f, 0.0f } }, (vec2s) { { 32, 48 } }, x, y, 20.0f); */

    (void)dialog;
    (void)arrow_pos;
    (void)opening;
}

void fn_camera(const instruction_t* instr) {
    camera_t* cam = camera_get_internals();
    transform_t* transform = gfx_model_get_transform();

    f32 x = parse_coord(instr->params[0].value.i16);
    f32 y = parse_coord(instr->params[1].value.i16);
    f32 z = parse_coord(instr->params[2].value.i16);
    f32 pitch_rad = parse_rad(instr->params[3].value.i16);
    f32 maprot_rad = parse_rad(instr->params[4].value.i16);
    f32 yaw_rad = parse_rad(instr->params[5].value.i16);
    f32 zoom = parse_zoom(instr->params[6].value.i16);
    f32 duration = (f32)instr->params[7].value.i16;

    transition_add(instr->opcode, &cam->position.x, cam->position.x, x, duration);
    transition_add(instr->opcode, &cam->position.y, cam->position.y, y, duration);
    transition_add(instr->opcode, &cam->position.z, cam->position.z, z, duration);
    transition_add(instr->opcode, &cam->yaw_rad, cam->yaw_rad, yaw_rad, duration);
    transition_add(instr->opcode, &cam->pitch_rad, cam->pitch_rad, pitch_rad, duration);
    transition_add(instr->opcode, &cam->zoom, cam->zoom, zoom, duration);

    transition_add(instr->opcode, &transform->rotation.y, transform->rotation.y, maprot_rad, duration);
}

void fn_wait_for_instruction(const instruction_t* instr) {
    waittype_e waittype = instr->params[0].value.u8;
    vm_wait(waittype);
}

void fn_warp_unit(const instruction_t* instr) {
    u8 unit_id = instr->params[0].value.u8;
    u8 unused = instr->params[1].value.u8; // always 0x00
    u8 tile_x = instr->params[2].value.u8;
    u8 tile_y = instr->params[3].value.u8;
    u8 elevation = instr->params[4].value.u8; // 0x00 lower, 0x01 upper
    u8 facing = instr->params[5].value.u8;    // 0x00 south, 0x01 west, 0x02 north, 0x03 east

    // FIXME: unit_id is probably not what we want to use here, but we aren't working
    // with units yet so ignore for now.
    if (unit_id >= 100) {
        printf("Invalid unit id %d\n", unit_id);
        return;
    }

    texture_t texture = sprite_get_paletted_texture(F_EVENT__UNIT_BIN, 0);
    transform_t transform = {
        .translation = { { tile_x * 24.0f, elevation * 5.0f, tile_y * 24.0f } },
        .rotation = { { 0.0f, facing * 90.0f, 0.0f } },
        .scale = { { 15.0f, 15.0f, 15.0f } }
    };

    sprite_t* sprite = &gfx_sprite_get_internals()[unit_id];
    *sprite = gfx_sprite_create(SPRITE_3D, texture, (vec2s) { { unit_id * 32.0f, 0.0f } }, (vec2s) { { 32.0f, 40.0f } }, transform);

    (void)unused;
}
