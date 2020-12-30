//
// Created by albert on 10/5/20.
//

#include <stdlib.h>
#include <stdio.h>
#include "engine.h"
#include <string.h>
#include <math.h>


Entity *engine_entity_create(System *system, const char *name, int x, int y,
                             void* (*entity_init_func)(Entity *ent, System *system),
                             void* (*entity_update_func)(Entity *ent, System *system),
                             void* (*entity_destroy_func)(Entity *ent)) {
    Entity *ent = malloc(sizeof(Entity));
    ent->system = system;
    ent->id = 0;
    ent->pos_x = x;
    ent->pos_y = y;
    ent->gravity_factor = 1;
    ent->max_vel_x = 1;
    ent->max_vel_y = 1;
    ent->accel_x = 0;
    ent->accel_y = 0;
    ent->vel_x = 0;
    ent->vel_y = 0;
    ent->anims_l = 0;
    ent->offset_x = 0;
    ent->offset_y = 0;
    ent->offset_w = 0;
    ent->offset_h = 0;
    ent->friction_x = 0;
    ent->friction_y = 0;
    ent->entity_init_func = entity_init_func;
    ent->entity_update_func = entity_update_func;
    ent->entity_destroy_func = entity_destroy_func;
    ent->entity_init_func(ent, system);
    ent->killed = false;
    ent->standing = false;
    ent->falling = false;
    return ent;
}

void engine_entity_destroy(Entity *entity) {
    entity->entity_destroy_func(entity);
    for(size_t i=0; i < entity->anims_l; i++) {
        engine_animation_destroy(entity->anims[i]);
    }
    free(entity->anims);
    free(entity);
}

float engine_entity_get_new_velocity(System *system, float vel, float accel, float friction, float max_vel) {
    float new_vel = 0;
    if (accel) {
        new_vel = (float)(vel + accel * system->tick);
    }
    else if (friction) {
        float delta = (float)(friction * system->tick);
        if (vel - delta > 0) {
            new_vel = vel - delta;
        }
        else if (vel + delta < 0) {
            new_vel = vel + delta;
        }
        else {
            return 0;
        }
    }
    else {
        new_vel = vel;
    }
    if (new_vel < -max_vel) {
        new_vel = -max_vel;
    } else if (new_vel > max_vel) {
        new_vel = max_vel;
    }
    return new_vel;
}

void engine_entity_handle_movement_trace(Entity *entity, TraceResult *res) {
    entity->standing = false;

    if (res->collision_y) {
        if (entity->vel_y > 0) {
            entity->standing = true;
        }
        entity->vel_y = 0;
    }
    if (res->collision_x) {
        entity->vel_x = 0;
    }
    entity->pos_x = res->pos_x;
    entity->pos_y = res->pos_y;
}

void engine_entity_update(Entity *entity, CollisionMap *collision_map) {
    const float game_gravity = 0.002;

    entity->entity_update_func(entity, entity->system);

    entity->last_x = entity->pos_x;
    entity->last_y = entity->pos_y;

    entity->vel_y += (float)(game_gravity * entity->system->tick * entity->gravity_factor);

    entity->vel_x = engine_entity_get_new_velocity(entity->system, entity->vel_x, entity->accel_x, entity->friction_x, entity->max_vel_x);
    entity->vel_y = engine_entity_get_new_velocity(entity->system, entity->vel_y, entity->accel_y, entity->friction_y, entity->max_vel_y);

    float mx = (float)(entity->vel_x * entity->system->tick );
    float my = (float)(entity->vel_y * entity->system->tick );

    TraceResult res;
    engine_collision_map_trace(collision_map,
                               &res,
                               entity->pos_x,
                               entity->pos_y,
                               mx,
                               my,
                               entity->size_x,
                               entity->size_y
                               );
    engine_entity_handle_movement_trace(entity, &res);

    if(entity->anims) {
        engine_animation_update(entity->current_anim);
    }

}

void engine_entity_draw(Entity *entity, Game *game) {
    if(entity->current_anim) {
        engine_animation_draw(entity->current_anim,
                              entity->pos_x - entity->offset_x - game->r_screen_x,
                              entity->pos_y - entity->offset_y - game->r_screen_y
        );
    }
}

Animation *engine_entity_add_anim(Entity *entity, const char *name, float frame_time, const u_short *sequence, size_t seq_l, bool stop) {
    Animation *a = engine_animation_create(entity->anim_sheet, name, frame_time, seq_l, sequence, stop);
    entity->anims_l++;
    if(entity->anims_l == 1) {
        entity->anims = malloc(sizeof(Animation));
    }
    else {
        entity->anims = realloc(entity->anims, sizeof(Animation) * entity->anims_l);
    }
    entity->anims[entity->anims_l - 1] = a;
    if (!entity->current_anim) {
        entity->current_anim = a;
    }
    return a;
}

void engine_entity_select_anim(Entity *entity, const char *name) {
    bool flip_x = entity->current_anim->flip_x;
    for(size_t i=0; i < entity->anims_l; i++) {
        if (strcmp(name, entity->anims[i]->name) == 0) {
            entity->current_anim = entity->anims[i];
            entity->current_anim->flip_x = flip_x;
        }
    }
}
