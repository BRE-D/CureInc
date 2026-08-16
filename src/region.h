#ifndef REGION_H
#define REGION_H

#include "types.h"


void region_init(GameState *gs);
/*
region_init:
 This function fills in 8 world regions 
 with their starting stats — population size, healthcare capacity, public trust, border control, and starting infection level.
 It's basically our game's opening scenario setup.
 */


void region_update_states(GameState *gs);
/*
region_update_states: This runs every simulated day and re-labels each 
region based on how infected it currently is.
*/
#endif 