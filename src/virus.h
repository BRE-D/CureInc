#ifndef VIRUS_H
#define VIRUS_H

#include "types.h"

void virus_init(Virus *v);
void virus_update(GameState *gs);
void virus_refresh_totals(GameState *gs);

int virus_try_mutate(Virus *v, int day);
int virus_has_trait(const Virus *v, MutationTrait t);

const char *virus_trait_name(MutationTrait t);
float virus_hospital_load(const Region *r);

#endif