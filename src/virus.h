#ifndef VIRUS_H
#define VIRUS_H
#include "types.h"

void  virus_init(Virus *v);
void  virus_update(Virus *v, float dtDays);
void  virus_try_mutate(Virus *v, float dtDays);
int   virus_has_trait(const Virus *v, MutationTrait t);
const char *virus_trait_name(MutationTrait t);

#endif