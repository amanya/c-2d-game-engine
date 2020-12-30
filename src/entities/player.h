//
// Created by albert on 11/9/20.
//

#ifndef SDLSCROLLER_PLAYER_H
#define SDLSCROLLER_PLAYER_H

#include "../engine/engine.h"

void player_init(Entity *ent, System *system);
void player_update(Entity *player_entity, System *system);
void player_destroy(Entity *player_entity);

#endif //SDLSCROLLER_PLAYER_H
