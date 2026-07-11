#ifndef VIRUS_H
#define VIRUS_H

#include "types.h"

/*
 * virus_update - Executes the daily simulation step for the pathogen.
 * Updates local infections, handles cross-border transmission,
 * manages healthcare degradation, computes mortality, and triggers mutations.
 * Must be called once per simulated day.
 */
void virus_update(GameState *gs);

#endif /* VIRUS_H */
