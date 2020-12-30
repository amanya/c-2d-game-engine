//
// Created by albert on 10/5/20.
//

#include <stdlib.h>
#include <stdio.h>
#include "engine.h"

Clock engine_global_clock = {
        .last = 0,
        .time = 1,
        .delta = 0,
        .time_scale = 0.8f,
        .max_step = 50
};

void engine_global_clock_init() {
    engine_global_clock.last = engine_global_clock.time = engine_callback_get_time();
}

void engine_global_clock_step(void) {
    uint32_t current = engine_callback_get_time();
    if(current > engine_global_clock.last) {
        uint32_t delta = (current - engine_global_clock.last);
        engine_global_clock.time += ((delta < engine_global_clock.max_step) ? delta : engine_global_clock.max_step) * engine_global_clock.time_scale;
        engine_global_clock.delta = delta;
    }
    engine_global_clock.last = current;
}

Timer *engine_timer_create(u_int seconds) {
    Timer *timer = malloc(sizeof(Timer));
    timer->base = engine_global_clock.time;
    timer->last = engine_global_clock.time;
    timer->target = seconds;
    timer->paused_at = 0;
    return timer;
}

void engine_timer_set(Timer *timer, u_int seconds) {
    timer->target = seconds;
    timer->base = engine_global_clock.time;
    timer->paused_at = 0;
}

void engine_timer_reset(Timer *timer) {
    timer->base = engine_global_clock.time;
    timer->paused_at = 0;
}

uint32_t engine_timer_tick(Timer *timer) {
    if (engine_global_clock.time > timer->last) {
        uint32_t delta = engine_global_clock.time - timer->last;
        timer->last = engine_global_clock.time;
        return timer->paused_at ? 0 : delta;
    }
    return 0;
}

uint32_t engine_timer_delta(Timer *timer) {
    uint32_t time = 0;
    if (timer->paused_at) {
        time = timer->paused_at;
    } else {
        time = engine_global_clock.time;
    }
    uint32_t delta = time - timer->base - timer->target;
    return delta < 0 ? 0 : delta;
}

void engine_timer_pause(Timer *timer) {
    if (!timer->paused_at) {
        timer->paused_at = engine_global_clock.time;
    }
}

void engine_timer_unpause(Timer *timer) {
    if (timer->paused_at) {
        timer->base += engine_global_clock.time - timer->paused_at;
        timer->paused_at = 0;
    }
}

void engine_timer_destroy(Timer *timer) {
    free(timer);
}

