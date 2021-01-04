//
// Created by albert on 10/5/20.
//

#include <stdlib.h>
#include <stdio.h>
#include "engine.h"
#include <string.h>
#include <math.h>


Entity *engine_entity_create(Game *game, System *system, const char *name, int x, int y,
                             void* (*entity_init_func)(Entity *ent, System *system),
                             void* (*entity_update_func)(Entity *ent, System *system),
                             void* (*entity_destroy_func)(Entity *ent)) {
    Entity *ent = malloc(sizeof(Entity));
    ent->game = game;
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
    ent->bounciness = 0;
    ent->min_bounce_velocity = 0;
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
        if (entity->bounciness > 0 && fabsf(entity->vel_y) > entity->min_bounce_velocity) {
            entity->vel_y *= -entity->bounciness;
        } else {
            if (entity->vel_y > 0) {
                entity->standing = true;
            }
            entity->vel_y = 0;
        }
    }
    if (res->collision_x) {
        if (entity->bounciness > 0 && fabsf(entity->vel_x) > entity->min_bounce_velocity) {
            entity->vel_x *= -entity->bounciness;
        } else {
            entity->vel_x = 0;
        }
    }

    entity->pos_x = (int)res->pos_x;
    entity->pos_y = (int)res->pos_y;
}

void engine_entity_update(Entity *entity, CollisionMap *collision_map) {
    const float game_gravity = 0.002f;

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
                               (float)entity->pos_x,
                               (float)entity->pos_y,
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

Animation *engine_entity_add_anim(Entity *entity, AnimationSheet *animationSheet, const char *name, float frame_time, const u_short *sequence, size_t seq_l, bool stop) {
    Animation *a = engine_animation_create(animationSheet, name, frame_time, seq_l, sequence, stop);
    entity->anims_l++;
    if(entity->anims_l == 1) {
        entity->anims = malloc(sizeof(Animation *));
    }
    else {
        entity->anims = realloc(entity->anims, sizeof(Animation *) * entity->anims_l);
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

bool engine_entity_touches(Entity *entity, Entity *other) {
    return !(
            entity->pos_x >= other->pos_x + other->size_x ||
            entity->pos_x + entity->size_x <= other->pos_x ||
            entity->pos_y >= other->pos_y + other->size_y ||
            entity->pos_y + entity->size_y <= other->pos_y
            );
}

float engine_entity_distance_to(Entity *entity, Entity *other) {
    float xd = ((float)entity->pos_x + (float)entity->size_x / 2) - ((float)other->pos_x + (float)other->size_x / 2);
    float yd = ((float)entity->pos_y + (float)entity->size_y / 2) - ((float)other->pos_y + (float)other->size_y / 2);
    return sqrtf(xd * xd + yd * yd);
}

void engine_entity_check_pair(Entity *a, Entity *b) {
    if (
            a->collides && b->collides &&
            a->collides + b->collides > ENTITY_COLLIDES_ACTIVE
            ) {
        engine_entity_solve_collision(a, b);
    }
}

void engine_entity_solve_collision(Entity *a, Entity *b) {
    Entity *weak = NULL;
    if (
            a->collides == ENTITY_COLLIDES_LITE ||
            b->collides == ENTITY_COLLIDES_FIXED
            ) {
        weak = a;
    } else if (
            b->collides == ENTITY_COLLIDES_LITE ||
            a->collides == ENTITY_COLLIDES_FIXED
            ) {
        weak = b;
    }

    if(
            a->last_x + a->size_x > b->last_x &&
            a->last_x < b->last_x + b->size_x
            ) {
        if (a->last_y < b->last_y) {
            engine_entity_separate_on_y_axis(a, b, weak);
        } else {
            engine_entity_separate_on_y_axis(b, a, weak);
        }
        //engine_entity_collide_with(a, b, 'y');
        //engine_entity_collide_with(b, a, 'y');
    } else if (
            a->last_y + a->size_y > b->last_y &&
            a->last_y < b->last_y + b->size_y
            ) {
        if ( a->last_x < b->last_x) {
            engine_entity_separate_on_x_axis(a, b, weak);
        } else {
            engine_entity_separate_on_y_axis(b, a, weak);
        }
        //engine_entity_collide_with(a, b, 'x');
        //engine_entity_collide_with(b, a, 'x');
    }
}

void engine_entity_separate_on_x_axis(Entity *left, Entity *right, Entity *weak) {
    float nudge = (float)(left->pos_x + left->size_x - right->pos_x);

    if (weak) {
        Entity *strong = left == weak ? right : left;
        weak->vel_x = -weak->vel_x * weak->bounciness + strong->vel_x;

        TraceResult res;
        engine_collision_map_trace(
                weak->game->collision_map, &res,
                (float)weak->pos_x, (float)weak->pos_y,
                weak == left ? -nudge : nudge, 0,
                weak->size_x, weak->size_y
                );
        weak->pos_x = (int)res.pos_x;
    } else {
        float v2 = (left->vel_x - right->vel_x)/2;
        left->vel_x = -v2;
        right->vel_x = v2;

        TraceResult res_left;
        engine_collision_map_trace(
                left->game->collision_map, &res_left,
                (float)left->pos_x, (float)left->pos_y,
                -nudge/2, 0,
                left->size_x, left->size_y
        );
        left->pos_x = (int)floorf(res_left.pos_x);

        TraceResult res_right;
        engine_collision_map_trace(
                right->game->collision_map, &res_right,
                (float)right->pos_x, (float)right->pos_y,
                nudge/2, 0,
                right->size_x, right->size_y
        );
        right->pos_x = (int)ceilf(res_right.pos_x);
    }
}

void engine_entity_separate_on_y_axis(Entity *top, Entity *bottom, Entity *weak) {
    float nudge = (float)(top->pos_y + top->size_y - bottom->pos_y);

    if (weak) {
        Entity *strong = top == weak ? bottom : top;
        weak->vel_y = -weak->vel_y * weak->bounciness + strong->vel_y;

        float nudge_x = 0;

        TraceResult res;
        engine_collision_map_trace(
                weak->game->collision_map, &res,
                (float)weak->pos_x, (float)weak->pos_y,
                nudge_x, weak == top ? -nudge : nudge,
                weak->size_x, weak->size_y
        );
        weak->pos_x = (int)res.pos_x;
        weak->pos_y = (int)res.pos_y;
    } else if (top->game->gravity && (bottom->standing || top->vel_y > 0)) {
        // bottom is standing
        TraceResult res_top;
        engine_collision_map_trace(
                top->game->collision_map, &res_top,
                (float)top->pos_x, (float)top->pos_y,
                0, (float)-(top->pos_y + top->size_y - bottom->pos_y),
                top->size_x, top->size_y
                );
        top->pos_y = (int)res_top.pos_y;

        if (top->bounciness > 0 && top->vel_y > top->min_bounce_velocity) {
            top->vel_y *= -top->bounciness;
        } else {
            top->standing = true;
            top->vel_y = 0;
        }
    } else {
        float v2 = (top->vel_y - bottom ->vel_y)/2;
        top->vel_y = -v2;
        bottom->vel_y = v2;

        float nudge_x = bottom->vel_x * bottom->system->tick;

        TraceResult res_top;
        engine_collision_map_trace(
                top->game->collision_map, &res_top,
                (float)top->pos_x, (float)top->pos_y,
                nudge_x, -nudge/2,
                top->size_x, top->size_y
        );
        top->pos_x = (int)floorf(res_top.pos_y);

        TraceResult res_bottom;
        engine_collision_map_trace(
                bottom->game->collision_map, &res_bottom,
                (float)bottom->pos_x, (float)bottom->pos_y,
                0, nudge/2,
                bottom->size_x, bottom->size_y
        );
        bottom->pos_y = (int)ceilf(res_bottom.pos_y);
    }
}


