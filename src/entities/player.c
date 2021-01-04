//
// Created by albert on 11/9/20.
//

#include "../engine/engine.h"

#define PLAYER_GROUND_ACCEL 0.2f
#define PLAYER_AIR_ACCEL 0.1f
#define PLAYER_VEL_JUMP 0.6f

void player_init(Entity *ent, System *system) {
    ent->vel_x = 0;
    ent->accel_x = 0;
    ent->vel_y = 0;
    ent->accel_y = 0;
    ent->max_vel_x = 0.2f;
    ent->max_vel_y = 0.8f;
    ent->size_x = 24;
    ent->size_y = 24;
    ent->offset_x = 0;
    ent->offset_y = 0;
    ent->gravity_factor = 1;
    ent->friction_x = 0.01f;
    ent->friction_y = 0.001f;
    ent->bounciness = 0;
    ent->min_bounce_velocity = 0;
    ent->type = ENTITY_TYPE_A;
    ent->collides = ENTITY_COLLIDES_ACTIVE;
    ent->check_against = ENTITY_TYPE_B;

    Texture *texture_run = engine_texture_create("../assets/MR-Platformer-PixelAssets-v1/Main/Player/Player-Run-24x24.png");
    Texture *texture_idle = engine_texture_create("../assets/MR-Platformer-PixelAssets-v1/Main/Player/Player-Idle-24x24.png");
    Texture *texture_jump = engine_texture_create("../assets/MR-Platformer-PixelAssets-v1/Main/Player/Player-Jump-24x24.png");
    Texture *texture_fall = engine_texture_create("../assets/MR-Platformer-PixelAssets-v1/Main/Player/Player-Fall-24x24.png");

    AnimationSheet *animation_sheet_run = engine_animation_sheet_create(24, 24, texture_run);
    AnimationSheet *animation_sheet_idle = engine_animation_sheet_create(24, 24, texture_idle);
    AnimationSheet *animation_sheet_jump = engine_animation_sheet_create(24, 24, texture_jump);
    AnimationSheet *animation_sheet_fall = engine_animation_sheet_create(24, 24, texture_fall);

    const ushort sequence_walk[] = {0,1,2,3,4,5,6,7};
    const ushort sequence_idle[] = {0,1,2,1};
    const ushort sequence_fall[] = {0,1,2,3,4,5,6,7,8,9};
    const ushort sequence_jump[] = {0,0,1,2,3};

    engine_entity_add_anim(ent, animation_sheet_run, "walk", (float)100, sequence_walk, sizeof(sequence_walk) / sizeof(sequence_walk[0]), false);
    engine_entity_add_anim(ent, animation_sheet_idle, "idle", (float)100, sequence_idle, sizeof(sequence_idle) / sizeof(sequence_idle[0]), false);
    engine_entity_add_anim(ent, animation_sheet_fall, "fall", (float)100, sequence_fall, sizeof(sequence_fall) / sizeof(sequence_fall[0]), false);
    engine_entity_add_anim(ent, animation_sheet_jump, "jump", (float)100, sequence_jump, sizeof(sequence_jump) / sizeof(sequence_jump[0]), true);

    engine_entity_select_anim(ent, "idle");

    engine_input_bind(system->input, SDLK_SPACE, "jump");
    engine_input_bind(system->input, SDLK_a, "left");
    engine_input_bind(system->input, SDLK_d, "right");
}

void player_update(Entity *player_entity, System *system) {
    Input *input = system->input;
    bool flip_x = player_entity->current_anim->flip_x;

    float accel = player_entity->standing ? PLAYER_GROUND_ACCEL : PLAYER_AIR_ACCEL;
    if(engine_input_action_state(input, "left")) {
        player_entity->accel_x = -accel;
        flip_x = true;
    } else if(engine_input_action_state(input, "right")) {
        player_entity->accel_x = accel;
        flip_x = false;
    } else {
        player_entity->accel_x = 0;
    }
    if(player_entity->standing && engine_input_action_state(input, "jump")) {
        if(player_entity->vel_y == 0) {
            player_entity->vel_y = -PLAYER_VEL_JUMP;
            player_entity->falling = false;
        }
    } else if (!player_entity->standing && !engine_input_action_state(input, "jump") && !player_entity->falling) {
        player_entity->vel_y = player_entity->vel_y / 3;
        player_entity->falling = true;
    }
    player_entity->current_anim->flip_x = flip_x;

    if(player_entity->vel_y < 0 && !player_entity->standing) {
        engine_entity_select_anim(player_entity, "jump");
    } else if(player_entity->vel_y >= 0 && !player_entity->standing) {
        engine_entity_select_anim(player_entity, "fall");
    } else if(player_entity->vel_x != 0) {
        engine_entity_select_anim(player_entity, "walk");
    } else {
        engine_entity_select_anim(player_entity, "idle");
    }
}

void player_destroy(Entity *player_entity) {
    engine_texture_destroy(player_entity->anim_sheet->image);
    engine_animation_sheet_destroy(player_entity->anim_sheet);
    engine_entity_destroy(player_entity);
}

