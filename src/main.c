#include <SDL2/SDL.h>
#include <SDL_vulkan.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "engine/engine.h"
#include "entities/player.h"

const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 200;

const int SCALE = 8;

const int REAL_WIDTH = SCREEN_WIDTH * SCALE;
const int REAL_HEIGHT = SCREEN_HEIGHT * SCALE;

const int SCREEN_FPS = 30;

#define DEBUG_BUFFER_MAX_LENGTH 500

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Renderer *engine_texture_global_renderer = NULL;

void * SDL_text_loader(const char *path) {
    return IMG_LoadTexture(engine_texture_global_renderer, path);
}

enum KEY_PRESS
{
    KEY_PRESS_LEFT,
    KEY_PRESS_RIGHT,
    KEY_PRESS_JUMP,
    KEY_PRESS_QUIT,
    KEY_PRESS_MAX
};

unsigned char key[KEY_PRESS_MAX];


bool init() {
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Unable to initialize SDL! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

    if( TTF_Init() == -1 )
    {
        printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
        return false;
    }

    window = SDL_CreateWindow("sdlscroller", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, REAL_WIDTH, REAL_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if(window == NULL) {
        printf("Unable to initialize window! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(renderer == NULL) {
        printf("Unable to create renderer! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

    int img_flags = IMG_INIT_PNG;
    if(!(IMG_Init(img_flags) & img_flags)) {
        printf("Unable to initialize image! SLD Error: %s\n", IMG_GetError());
    }

    return true;
}

void deinit() {
    SDL_DestroyWindow(window);
    window = NULL;

    SDL_Quit();
}

SDL_Texture* load_texture(char *path) {
    SDL_Texture* new_texture = NULL;

    SDL_Surface *loaded_surface = IMG_Load(path);

    if(loaded_surface == NULL) {
        printf("Unable to load image %s! SDL_image error: %s\n", path, IMG_GetError());
        return NULL;
    }

    new_texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
    if(new_texture == NULL) {
        printf("Unable to create texture from %s! SDL error: %s\n", path, SDL_GetError());
    }

    SDL_FreeSurface(loaded_surface);

    return new_texture;
}

int texture_width(SDL_Texture* texture) {
    int w, h;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    return w;
}


void keyboard_init() {
    memset(key, 0, sizeof(key));
}

int main(int argc, char **argv) {
    char *DEBUG_BUFFER = (char *)malloc(DEBUG_BUFFER_MAX_LENGTH * sizeof(char));

    if(!init()) {
        printf("Failed to initialize!\n");
        return 1;
    }

    SDL_Texture *sheet = load_texture("../assets/adventurer-v1.5-Sheet.png");

    if(!sheet) {
        printf("Failed to load media!\n");
    }

    // engine init
    keyboard_init();

    tmx_img_load_func = SDL_text_loader;
    tmx_img_free_func = (void (*)(void *))SDL_DestroyTexture;

    engine_callback_get_time = (Uint32 (*)(void))SDL_GetTicks;
    engine_callback_img_load_func = (void *(*)(const char *))load_texture;
    engine_callback_img_free_func = (void (*)(void *))SDL_DestroyTexture;
    engine_callback_img_with_func = (int (*)(void *))texture_width;
    System *system = engine_system_create(SCREEN_FPS, SCREEN_WIDTH, SCREEN_HEIGHT, SCALE);
    engine_texture_global_renderer = renderer;

    Game *game = engine_game_create();
    game->system = system;

    engine_input_bind(system->input, SDLK_ESCAPE, "quit");
    engine_input_bind(system->input, SDLK_p, "pause");

    Tileset *tileset = engine_tileset_create("../assets/sidescroller3.tmx");

    size_t num_layers = 2;
    LayerMap  *layers[num_layers];
    layers[0] = engine_map_layer_create(32, "Ground", tileset);
    layers[0]->foreground = false;
    layers[1] = engine_map_layer_create(32, "Fringe", tileset);
    layers[1]->foreground = true;
    game->layer_maps = layers;
    game->layer_maps_l = num_layers;

    LayerMap *map_layer_collision = engine_map_layer_create(32, "Collision", tileset);
    CollisionMap *collision_map = engine_collision_map_create(32, NULL, map_layer_collision);
    game->collision_map = collision_map;

    size_t num_entities = 1;
    Entity *entities[num_entities];
    entities[0] = engine_entity_create(system, "player", 100, 0, (void* (*)(Entity *, System *))player_init, (void* (*)(Entity *, System *))player_update, (void* (*)(Entity *))player_destroy);
    game->entities = entities;
    game->entities_l = num_entities;

    game->map_width = (int)(game->layer_maps[0]->tileset->map->width * game->layer_maps[0]->tile_size);
    game->map_height = (int)(game->layer_maps[0]->tileset->map->height * game->layer_maps[0]->tile_size);
    game->camera_ent = entities[0];

    int frame_cnt = 0;
    Timer *fps_timer = engine_timer_create(0);

    bool done = false;
    SDL_Event e;

    while(!done) {
        while(SDL_PollEvent(&e) != 0) {
            if(engine_input_action_state(system->input, "quit")) {
                done = true;
            }
            else if(engine_input_action_state(system->input, "pause")) {
                if(system->timer->paused_at != 0) {
                    engine_timer_unpause(system->timer);
                } else {
                    engine_timer_pause(system->timer);
                }
            }
            else if(e.type == SDL_KEYUP) {
                engine_input_key_up(system->input, e);
            }
            else if(e.type == SDL_KEYDOWN) {
                engine_input_key_down(system->input, e);
            }
        }
        engine_system_run(system);
        engine_game_update(game);

        float avg_fps = frame_cnt / (engine_timer_delta(fps_timer) / 1000.f);
        if(avg_fps > 2000000) {
            avg_fps = 0;
        }

        frame_cnt++;

        engine_game_draw(game);
        memset(DEBUG_BUFFER, 0, DEBUG_BUFFER_MAX_LENGTH);
        //sprintf(DEBUG_BUFFER, "fps: %f\nx: %i\ny: %i\nvel x: %f\nvel y: %f\nacc x: %f\nacc y: %f\ntick: %i", avg_fps, entities[0]->pos_x, entities[0]->pos_y, entities[0]->vel_x, entities[0]->vel_y, entities[0]->accel_x, entities[0]->accel_y, system->tick);
        //Texture *text = engine_texture_create_from_text(DEBUG_BUFFER, BLACK);
        //SDL_RendererFlip flip = SDL_FLIP_NONE;
        //SDL_Rect clip = {0, 0, text->width, text->height};
        //engine_texture_render(text, 10, 10, &clip, flip);
        //engine_texture_destroy(text);

        SDL_RenderSetScale(renderer, SCALE, SCALE);
        SDL_RenderPresent(renderer);
    }

    deinit();
    engine_collision_map_destroy(collision_map);
    engine_tileset_destroy(tileset);
    engine_system_destroy(system);

    return 0;
}
