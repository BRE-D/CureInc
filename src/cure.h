#ifndef CURE_H
#define CURE_H

#include "types.h"

/*
 * cure_update - Executes daily progression for cure research, funding, and vaccine distribution.
 * Must be called once per simulated day.
 */
void cure_update(GameState *gs);

#endif /* CURE_H */
