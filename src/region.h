// Header guard: include these declarations only once per compiled source file.
#ifndef REGION_H
#define REGION_H

#include "types.h"

// Set the eight starting regions. Named initializers specify chosen fields; omitted fields become zero.
// Seed infections in Westeros and Essos, then update region states; returns nothing.
// gs: shared game state; const means this function only reads it.
void region_init(GameState *gs);

// Set each region state from its infected fraction: clean up to 0.000001, infected below 0.30, critical
// below 0.60, otherwise devastated. UI card colors use separate thresholds; returns nothing.
// gs: shared game state; const means this function only reads it.
void region_update_states(GameState *gs);

#endif
