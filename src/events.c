#include "events.h"

/*
 * event_log_add - Adds a new event entry to the rolling log.
 * Shifty-FIFO style eviction of the oldest elements.
 */
void event_log_add(GameState *gs, const char *title, const char *description)
{
    if (!gs) return;

    // Shift events left if we hit the limit
    if (gs->eventCount >= MAX_EVENTS) {
        for (int i = 1; i < MAX_EVENTS; i++) {
            gs->eventLog[i - 1] = gs->eventLog[i];
        }
        gs->eventCount = MAX_EVENTS - 1;
    }

    // Insert new event
    gs->eventLog[gs->eventCount].title = title;
    gs->eventLog[gs->eventCount].description = description;
    gs->eventLog[gs->eventCount].active = 1;
    gs->eventLog[gs->eventCount].timer = 5.0f; // display lifetime in seconds
    gs->eventCount++;
}
