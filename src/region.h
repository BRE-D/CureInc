#ifndef REGION_H
#define REGION_H

#include "types.h"


void region_init(GameState *gs);
/*
region_init:
 This function fills in our 8 world regions (the GoT-themed ones mapping to real continents) 
 with their starting stats — population size, healthcare capacity, public trust, border control, and starting infection level.
 It's basically our game's opening scenario setup.
 */


void region_update_states(GameState *gs);
/*
region_update_states: This runs every simulated day and re-labels each 
region based on how infected it currently is.
For example:
under 5% → CLEAN
under 30% → INFECTED
under 60% → CRITICAL
60%+ → DEVASTATED

This is literally the "System Reacts" part of your gameplay loop —
it's the game reading the current infection numbers and updating each region's 
status tier accordingly.
*/
#endif 