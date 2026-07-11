#ifndef VIRUS_H
#define VIRUS_H
#include "types.h"

void  Virus_Init(Virus *v);
void  Virus_Update(Virus *v, float dtDays);
void  Virus_TryMutate(Virus *v, float dtDays);
int   Virus_HasTrait(const Virus *v, MutationTrait t);
const char *Virus_TraitName(MutationTrait t);

#endif