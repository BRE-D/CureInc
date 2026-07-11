#ifndef EVENTS_H
#define EVENTS_H

#include "types.h"

/*
 * event_log_add - Adds a new event entry to the rolling log.
 * If the log is full (at MAX_EVENTS), it discards the oldest event (FIFO).
 */
void event_log_add(GameState *gs, const char *title, const char *description);

#endif /* EVENTS_H */
