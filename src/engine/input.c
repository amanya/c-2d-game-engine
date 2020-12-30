//
// Created by albert on 21/6/20.
//

// https://github.com/phoboslab/Impact/blob/master/lib/impact/input.js

#include "engine.h"

Input *engine_input_create() {
   Input *input = malloc(sizeof(Input));
   input->bindings = NULL;
   input->bindings_l = 0;
   input->actions = NULL;
   input->actions_l = 0;
   return input;
}

bool engine_input_action_exists(Input *input, const char *action) {
    for(size_t i = 0; i < input->actions_l; i++) {
        if(input->actions[i]->action == action) {
            return true;
        }
    }
    return false;
}

void engine_input_bind(Input *input, SDL_KeyCode code, char *action) {
    InputBinding *binding = malloc(sizeof(InputBinding));
    binding->action = action;
    binding->code = code;
    input->bindings_l++;
    if(input->bindings_l == 1) {
        input->bindings = malloc(sizeof(InputBinding));
    }
    else {
        input->bindings = realloc(input->bindings, sizeof(InputBinding) * input->bindings_l);
    }
    input->bindings[input->bindings_l - 1] = binding;

    if(!engine_input_action_exists(input, action)) {
        InputAction *input_action = malloc(sizeof(InputAction));
        input_action->action = action;
        input_action->state = false;
        input_action->locked = false;
        input_action->pressed = false;
        input_action->delayed_key_up = false;
        input->actions_l++;
        if(input->actions_l == 1) {
            input->actions = malloc(sizeof(InputAction));
        }
        else {
            input->actions = realloc(input->actions, sizeof(InputAction) * input->actions_l);
        }
        input->actions[input->actions_l - 1] = input_action;
    }
}

InputBinding *engine_input_find_binding_by_code(Input *input, SDL_KeyCode code) {
    for(size_t i = 0; i < input->bindings_l; i++) {
        if(input->bindings[i]->code == code) {
            return input->bindings[i];
        }
    }
    return NULL;
}

InputAction *engine_input_find_action(Input *input, char *action) {
    for(size_t i = 0; i < input->actions_l; i++) {
        if(strcmp(input->actions[i]->action, action) == 0) {
            return input->actions[i];
        }
    }
    return NULL;
}

void engine_input_key_down(Input *input, SDL_Event event) {
    SDL_KeyCode code = event.key.keysym.sym;

    InputBinding *binding = engine_input_find_binding_by_code(input, code);
    if (!binding) {
        return;
    }

    InputAction *action = engine_input_find_action(input, binding->action);
    if(action) {
        action->state = true;
        if(!action->locked) {
            action->pressed = true;
            action->locked = true;
        }
    }
}

void engine_input_key_up(Input *input, SDL_Event event) {
    SDL_KeyCode code = event.key.keysym.sym;

    InputBinding *binding = engine_input_find_binding_by_code(input, code);
    if (!binding) {
        return;
    }

    InputAction *action = engine_input_find_action(input, binding->action);
    if(action) {
        action->state = false;
        action->pressed = false;
    }
}

bool engine_input_action_state(Input *input, char *action) {
    InputAction *input_action = engine_input_find_action(input, action);
    if(input_action) {
        return input_action->state;
    }
    return false;
}

bool engine_input_action_pressed(Input *input, char *action) {
    InputAction *input_action = engine_input_find_action(input, action);
    if(input_action) {
        return input_action->pressed;
    }
    return false;
}

bool engine_input_action_released(Input *input, char *action) {
    InputAction *input_action = engine_input_find_action(input, action);
    if(action) {
        return input_action->delayed_key_up;
    }
    return false;
}

void engine_input_action_clear_pressed(Input *input, char *action) {
    for(size_t i = 0; i < input->actions_l; i++) {
        input->actions[i]->state = false;
        input->actions[i]->locked = false;
    }
}

void engine_input_destroy(Input *input) {
    for(size_t i=0; i < input->bindings_l; i++) {
        free(input->bindings[i]);
    }
    for(size_t i=0; i < input->actions_l; i++) {
        free(input->actions[i]);
    }
    free(input);
}
