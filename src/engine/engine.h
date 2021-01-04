//
// Created by albert on 2/5/20.
//

#ifndef TILEDTEST_ENGINE_H
#define TILEDTEST_ENGINE_H

#include <stdbool.h>
#include <sys/types.h>
#include <tmx.h>
#include <SDL_render.h>
#include <SDL_events.h>

// --- callbacks ---

extern void*  (*engine_callback_img_load_func) (const char *path);
extern void   (*engine_callback_img_free_func) (void *address);
extern int    (*engine_callback_img_with_func) (void *address);
extern void   (*engine_callback_img_draw_region_func) (void *address, float sx, float sy, float sw, float sh, float dx, float dy, int flags);
extern void   (*engine_callback_img_draw_scaled_bitmap_func) (void *address, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh, int flags);
extern uint32_t (*engine_callback_get_time) (void);

typedef struct TILESET Tileset;
typedef struct LAYER_MAP LayerMap;
typedef struct PARALLAX_LAYER ParallaxLayer;
typedef struct PARALLAX_MAP ParallaxMap;
typedef struct ENTITY Entity;
typedef struct ANIMATION Animation;
typedef struct ANIMATION_SHEET AnimationSheet;
typedef struct CLOCK Clock;
typedef struct TIMER Timer;
typedef struct SYSTEM System;
typedef struct GAME Game;
typedef struct TILEDEF Tiledef;
typedef struct COLLISION_MAP CollisionMap;
typedef struct TRACE_RESULT TraceResult;
typedef struct TEXTURE Texture;
typedef struct INPUT Input;

// --- TILEDEF ---

struct TILEDEF {
    int x1;
    int y1;
    int x2;
    int y2;
    bool solid;
};

// --- TILESET

struct TILESET {
    const char *name;
    tmx_map *map;
};

Tileset *engine_tileset_create(const char *resource);
void     engine_tileset_destroy(Tileset *tileset);

// --- LAYER_MAP ---

struct LAYER_MAP {
    Tileset *tileset;
    int tile_size;
    int width;
    int height;
    int scroll_x;
    int scroll_y;
    float distance;
    const char *layer_name;
    bool foreground;
    bool enabled;
    bool repeat;
    bool pre_render;
    int chunk_size;
    int chunk_rows;
    int chunk_cols;
    struct ALLEGRO_BITMAP **chunks;
};

LayerMap  *engine_map_layer_create(int tile_size, const char *layer_name, Tileset *tileset);
void       engine_map_layer_destroy(LayerMap *map_layer);
void       engine_map_layer_draw(LayerMap *layer_map, System *system);
void       engine_map_layer_draw_tiled(LayerMap *layer_map, System *system);
void       engine_map_layer_draw_pre_rendered(LayerMap *layer_map, System *system);
void       engine_map_layer_screen_pos(LayerMap *layer_map, int x, int y);
tmx_layer *engine_map_layer_find_layer(tmx_map *map, const char *name);
void       engine_map_layer_pre_render_map_to_chunks(LayerMap *layer_map, System *system);

// -- PARALLAX_LAYER --

struct PARALLAX_LAYER {
    Texture *image;
    float distance;
    int offset_x;
    int offset_y;
    int scroll_x;
    int scroll_y;
};

ParallaxLayer *engine_parallax_layer_create(const char *image_path, float distance);
void           engine_parallax_layer_destroy(ParallaxLayer *parallax_layer);
void           engine_parallax_layer_draw(ParallaxLayer *parallax_layer, Game *game);
void           engine_parallax_layer_screen_pos(ParallaxLayer *parallax_layer, int x, int y);

// -- PARALLAX_MAP --

struct PARALLAX_MAP {
    ParallaxLayer **layers;
    size_t layers_l;
};

ParallaxMap *engine_parallax_map_create();
void         engine_parallax_map_add_layer(ParallaxMap *parallax_map, const char *image_path, float distance);
void         engine_parallax_map_draw(ParallaxMap *parallax_map, Game *game);
void         engine_parallax_map_destroy(ParallaxMap *parallax_map);

// -- COLLISION_MAP ---

struct COLLISION_MAP {
    int tile_size;
    size_t last_slope;
    Tiledef *tiledef;
    int width;
    int height;
    int32_t *data;
    const char *name;
};

struct TRACE_RESULT {
    bool collision_x;
    bool collision_y;
    bool collision_slope;
    float pos_x;
    float pos_y;
    size_t tile_x;
    size_t tile_y;
};

CollisionMap *engine_collision_map_create(u_int tile_size, Tiledef *tiledef, LayerMap *layer_map);
void          engine_collision_map_destroy(CollisionMap *collision_map);
void          engine_collision_map_trace(CollisionMap *collision_map, TraceResult *res, float x, float y, float vx, float vy, int width, int height);
void          engine_collision_map_trace_step(CollisionMap *collision_map, TraceResult *res, float x, float y, float vx, float vy, int width, int height, float rvx, float rvy, int step);

// --- ENTITY ---

enum entity_type {
    ENTITY_TYPE_NONE,
    ENTITY_TYPE_A,
    ENTITY_TYPE_B,
    ENTITY_TYPE_BOTH
};

enum entity_collides {
    ENTITY_COLLIDES_NEVER,
    ENTITY_COLLIDES_LITE,
    ENTITY_COLLIDES_PASSIVE,
    ENTITY_COLLIDES_ACTIVE,
    ENTITY_COLLIDES_FIXED
};

struct ENTITY {
    u_int id;

    int size_x, size_y;
    int offset_x, offset_y, offset_w, offset_h;

    int pos_x, pos_y;
    int last_x, last_y;
    float vel_x, vel_y;
    float accel_x, accel_y;
    float friction_x, friction_y;
    float max_vel_x, max_vel_y;
    int z_index;
    float gravity_factor;
    bool standing;
    bool falling;
    float bounciness;
    float min_bounce_velocity;

    size_t anims_l;
    Animation **anims;
    AnimationSheet *anim_sheet;
    Animation *current_anim;

    System *system;
    Game *game;

    u_int health;

    u_int type;
    u_int check_against;
    u_int collides;

    bool killed;

    float slope_standing_min;
    float slope_standing_max;

    void* (*entity_init_func) (Entity *ent, System *system);
    void* (*entity_update_func) (Entity *ent, System *system);
    void* (*entity_destroy_func) (Entity *ent);
};

Entity    *engine_entity_create(Game *game, System *system, const char *name, int x, int y, void *(*entity_init_func)(Entity *ent, System *system), void *(*entity_update_func)(Entity *ent, System *system), void *(*entity_destroy_func)(Entity*ent));
void       engine_entity_destroy(Entity *entity);
void       engine_entity_update(Entity *entity, CollisionMap *collision_map);
void       engine_entity_draw(Entity *entity, Game *game);
Animation *engine_entity_add_anim(Entity *entity, AnimationSheet *animationSheet, const char *name, float frameTime, const u_short *sequence, size_t seq_l, bool stop);
void       engine_entity_select_anim(Entity *entity, const char *name);
bool       engine_entity_touches(Entity *entity, Entity *other);
float      engine_entity_distance_to(Entity *entity, Entity *other);
void       engine_entity_check_pair(Entity *a, Entity *b);
void       engine_entity_solve_collision(Entity *a, Entity *b);
void       engine_entity_separate_on_x_axis(Entity *left, Entity *right, Entity *weak);
void       engine_entity_separate_on_y_axis(Entity *top, Entity *bottom, Entity *weak);

// --- ANIMATION_SHEET ---

struct ANIMATION_SHEET {
    u_int width;
    u_int height;
    Texture *image;
};

AnimationSheet *engine_animation_sheet_create(unsigned int width, unsigned int height, Texture *texture);
void            engine_animation_sheet_destroy(AnimationSheet *animation_sheet);

// --- ANIMATION ---

struct ANIMATION {
    AnimationSheet *sheet;
    Timer *timer;
    u_short *sequence;
    bool flip_x;
    bool flip_y;
    size_t sequence_l;
    double frame_time;
    u_int frame;
    u_int tile;
    u_int loop_count;
    bool stop;
    const char *name;
};

Animation *engine_animation_create(AnimationSheet *sheet, const char *name, float frame_time, size_t seq_l, const u_short *sequence, bool stop);
void       engine_animation_destroy(Animation *animation);
void       engine_animation_update(Animation *animation);
void       engine_animation_draw(Animation *animation, int target_x, int target_y);

// --- TIMER ---

struct CLOCK {
    uint32_t last;
    uint32_t time;
    uint32_t delta;
    float time_scale;
    float max_step;
};

struct TIMER {
    uint32_t last;
    uint32_t base;
    u_int target;
    u_int paused_at;
};

void  engine_global_clock_init(void);
void  engine_global_clock_step(void);
Timer *engine_timer_create(u_int seconds);
void   engine_timer_destroy(Timer *timer);
void   engine_timer_set(Timer *timer, u_int seconds);
void   engine_timer_reset(Timer *timer);
uint32_t  engine_timer_tick(Timer *timer);
uint32_t  engine_timer_delta(Timer *timer);
void   engine_timer_pause(Timer *timer);
void   engine_timer_unpause(Timer *timer);

// --- SYSTEM ---

struct SYSTEM {
    u_int fps;
    int width;
    int height;
    u_int real_width;
    u_int real_height;
    int scale;
    uint32_t tick;
    bool running;
    Timer *timer;
    Input *input;
};

System *engine_system_create(u_int fps, u_int width, u_int height, u_int scale);
void    engine_system_run(System *system);
void    engine_system_destroy(System *system);
void    engine_system_clear(uint32_t color);
int     engine_system_get_draw_pos(System *system, int pos);
void    engine_system_resize(System *system, int width, int height, int scale);

// --- GAME ---

struct GAME {
    uint32_t clear_color;
    int gravity;
    int screen_x;
    int screen_y;
    int r_screen_x;
    int r_screen_y;
    int map_width;
    int map_height;
    Entity *camera_ent;

    Entity **entities;
    size_t entities_l;

    CollisionMap *collision_map;
    LayerMap **layer_maps;
    size_t layer_maps_l;

    ParallaxMap *parallax_map;

    System *system;

    int cell_size;
};

Game *engine_game_create();
void  engine_game_destroy(Game *game);
void  engine_game_run(Game *game);
void  engine_game_update(Game *game);
void  engine_game_draw(Game *game);
void  engine_game_update_entities(Game *game);
void  engine_game_draw_entities(Game *game);
void  engine_game_draw_debug(Game *game);
void engine_game_check_entities(Game *game);

// --- TEXTURE ---
extern SDL_Renderer *engine_texture_global_renderer;

struct TEXTURE {
    SDL_Texture* texture;
    int width, height;
};

Texture *engine_texture_create(char *path);
Texture *engine_texture_create_from_text(char *text, SDL_Color color);
void engine_texture_render(Texture *texture, int x, int y, SDL_Rect* clip, SDL_RendererFlip flip);
void engine_texture_destroy(Texture *texture);

// --- INPUT ---

typedef struct INPUT_BINDING {
    SDL_KeyCode code;
    char *action;
} InputBinding ;

typedef struct INPUT_ACTION {
    char *action;
    bool state;
    bool pressed;
    bool locked;
    bool delayed_key_up;
} InputAction;

struct INPUT {
    size_t bindings_l;
    size_t actions_l;
    InputBinding **bindings;
    InputAction **actions;
};

Input *engine_input_create();
void engine_input_bind(Input *input, SDL_KeyCode code, char *action);
void engine_input_key_down(Input *input, SDL_Event event);
void engine_input_key_up(Input *input, SDL_Event event);
void engine_input_destroy(Input *input);
void engine_input_action_clear_pressed(Input *input, char *action);
bool engine_input_action_released(Input *input, char *action);
bool engine_input_action_pressed(Input *input, char *action);
bool engine_input_action_state(Input *input, char *action);

// COLORS

#define LIGHTGRAY  (SDL_Color){ 200, 200, 200, 255 }        // Light Gray
#define GRAY       (SDL_Color){ 130, 130, 130, 255 }        // Gray
#define DARKGRAY   (SDL_Color){ 80, 80, 80, 255 }           // Dark Gray
#define YELLOW     (SDL_Color){ 253, 249, 0, 255 }          // Yellow
#define GOLD       (SDL_Color){ 255, 203, 0, 255 }          // Gold
#define ORANGE     (SDL_Color){ 255, 161, 0, 255 }          // Orange
#define PINK       (SDL_Color){ 255, 109, 194, 255 }        // Pink
#define RED        (SDL_Color){ 230, 41, 55, 255 }          // Red
#define MAROON     (SDL_Color){ 190, 33, 55, 255 }          // Maroon
#define GREEN      (SDL_Color){ 0, 228, 48, 255 }           // Green
#define LIME       (SDL_Color){ 0, 158, 47, 255 }           // Lime
#define DARKGREEN  (SDL_Color){ 0, 117, 44, 255 }           // Dark Green
#define SKYBLUE    (SDL_Color){ 102, 191, 255, 255 }        // Sky Blue
#define BLUE       (SDL_Color){ 0, 121, 241, 255 }          // Blue
#define DARKBLUE   (SDL_Color){ 0, 82, 172, 255 }           // Dark Blue
#define PURPLE     (SDL_Color){ 200, 122, 255, 255 }        // Purple
#define VIOLET     (SDL_Color){ 135, 60, 190, 255 }         // Violet
#define DARKPURPLE (SDL_Color){ 112, 31, 126, 255 }         // Dark Purple
#define BEIGE      (SDL_Color){ 211, 176, 131, 255 }        // Beige
#define BROWN      (SDL_Color){ 127, 106, 79, 255 }         // Brown
#define DARKBROWN  (SDL_Color){ 76, 63, 47, 255 }           // Dark Brown

#define WHITE      (SDL_Color){ 255, 255, 255, 255 }        // White
#define BLACK      (SDL_Color){ 0, 0, 0, 255 }              // Black
#define BLANK      (SDL_Color){ 0, 0, 0, 0 }                // Transparent
#define MAGENTA    (SDL_Color){ 255, 0, 255, 255 }          // Magenta
#define RAYWHITE   (SDL_Color){ 245, 245, 245, 255 }        // Ray White


#endif //TILEDTEST_ENGINE_H
