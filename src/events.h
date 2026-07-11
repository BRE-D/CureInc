#ifndef EVENTS_H
#define EVENTS_H

#include "types.h"

//sets up event log at the start of game
void events_init(GameState *gs);

//a random event picker and adds to log
void events_trigger_random(GameState *gs);

//ticks event timers and removes expired ones
void events_update(GameState *gs, float delta);

#endif