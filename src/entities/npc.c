//
// Created by albert on 30/12/20.
//

#include "../engine/engine.h"

#define NPC_GROUND_ACCEL 0.2f
#define NPC_AIR_ACCEL 0.1f
#define NPC_VEL_JUMP 0.6f

void npc_init(Entity *ent, System *system) {
    ent->vel_x = 0;
    ent->accel_x = 0;
    ent->vel_y = 0;
    ent->accel_y = 0;
    ent->max_vel_x = 0.2f;
    ent->max_vel_y = 0.8f;
    ent->size_x = 10;
    ent->size_y = 30;
    ent->offset_x = 20;
    ent->offset_y = 6;
    ent->gravity_factor = 1;
    ent->friction_x = 0.01f;
    ent->friction_y = 0.001f;
    ent->type = ENTITY_TYPE_B;
    ent->collides = ENTITY_COLLIDES_LITE;
    ent->check_against = ENTITY_TYPE_A;

    Texture *texture = engine_texture_create("../assets/adventurer-v1.5-Sheet.png");
    AnimationSheet *animation_sheet = engine_animation_sheet_create(50, 37, texture);
    ent->anim_sheet = animation_sheet;

    const ushort sequence_walk[] = {8, 9, 10, 11, 12, 13};
    const ushort sequence_idle[] = {0, 1, 2, 3};
    const ushort sequence_fall[] = {17};
    const ushort sequence_jump[] = {17};
    //engine_entity_add_anim(ent, "walk", (float)100, sequence_walk, sizeof(sequence_walk) / sizeof(sequence_walk[0]), false);
    //engine_entity_add_anim(ent, "idle", (float)100, sequence_idle, sizeof(sequence_idle) / sizeof(sequence_idle[0]), false);
    //engine_entity_add_anim(ent, "fall", (float)100, sequence_fall, sizeof(sequence_fall) / sizeof(sequence_fall[0]), false);
    //engine_entity_add_anim(ent, "jump", (float)100, sequence_jump, sizeof(sequence_jump) / sizeof(sequence_jump[0]), false);

    engine_entity_select_anim(ent, "idle");
}

void npc_update(Entity *ent, System *system) {
}

void npc_destroy(Entity *npc) {
    engine_texture_destroy(npc->anim_sheet->image);
    engine_animation_sheet_destroy(npc->anim_sheet);
    engine_entity_destroy(npc);
}

