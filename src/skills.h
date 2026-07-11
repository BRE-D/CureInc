#ifndef SKILLS_H
#define SKILLS_H

#include "types.h"

/*
 * skills_init - Populates the upgrades tree with starting configurations.
 */
void skills_init(GameState *gs);

/*
 * skills_unlock - Unlocks a skill node if conditions (RP cost, prerequisites) are met.
 * Returns 1 if unlock is successful, 0 otherwise.
 */
int skills_unlock(GameState *gs, int skill_index);

#endif /* SKILLS_H */
