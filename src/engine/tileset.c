//
// Created by albert on 10/5/20.
//

#include <stdlib.h>
#include "engine.h"

Tileset *engine_tileset_create(const char *resource) {
    tmx_map *map = tmx_load(resource);
    if (map == NULL) {
        tmx_perror("Cannot load map");
        return NULL;
    }
    Tileset *tileset = malloc(sizeof(Tileset));
    tileset->map = map;
    return tileset;
}

void engine_tileset_destroy(Tileset *tileset) {
    tmx_map_free(tileset->map);
    free(tileset);
}

