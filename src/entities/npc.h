//
// Created by albert on 30/12/20.
//

#ifndef SDLSCROLLER_NPC_H
#define SDLSCROLLER_NPC_H

#include "../engine/engine.h"

void npc_init(Entity *ent, System *system);
void npc_update(Entity *ent, System *system);
void npc_destroy(Entity *ent);

#endif //SDLSCROLLER_NPC_H
