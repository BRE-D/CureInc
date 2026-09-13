// Header guard: include these declarations only once per compiled source file.
#ifndef EVENTS_H
#define EVENTS_H

#include "types.h"

// Clear active notifications and timers, reset their count, and forget the previous random event. Called
// by reset_game; returns nothing.
// gs: shared game state; const means this function only reads it.
void events_init(GameState *gs);

// Choose one of 14 events, try up to five times to avoid an immediate repeat, then log and apply it.
// Research bonuses stop after Trials. Called every seven game days; returns nothing.
// gs: shared game state; const means this function only reads it.
void events_trigger_random(GameState *gs);

// Subtract real frame time from active notification timers and hide expired entries. Called only during
// gameplay, so pausing also freezes notices; returns nothing.
// gs: shared game state; const means this function only reads it.
// delta: elapsed real seconds since the previous frame.
void events_update(GameState *gs, float delta);

// Store an 8-second notification in the first free slot, or replace the active slot with least time
// remaining. Increase active count only for a free slot. Text pointers must remain valid; returns nothing.
// gs: shared game state; const means this function only reads it.
// title: read-only notification title text.
// description: read-only notification detail text.
void events_add(GameState *gs, const char *title, const char *description);

#endif
