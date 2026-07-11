#include "virus.h"
#include <stdlib.h>

void Virus_Init(Virus *v)
{
    v->infectivity    = 0.08f;
    v->severity        = 0.02f;
    v->resistance       = 0.0f;
    v->mutationRate     = 0.03f;   /* ~3% chance per day */
    v->activeTraits     = TRAIT_NONE;
    v->globalInfected   = 0.0001f; /* patient zero, not literally 0 */
    v->globalDead       = 0.0f;
}

void Virus_Update(Virus *v, float dtDays)
{
    float growth = v->infectivity * v->globalInfected
                   * (1.0f - v->globalInfected) * dtDays;

    v->globalInfected += growth;
    if (v->globalInfected > 1.0f) v->globalInfected = 1.0f;

    v->globalDead += v->severity * v->globalInfected * dtDays;
    if (v->globalDead > v->globalInfected) v->globalDead = v->globalInfected;
}

void Virus_TryMutate(Virus *v, float dtDays)
{
    float roll = (float)rand() / (float)RAND_MAX;
    if (roll > v->mutationRate * dtDays) return;

    /* find traits not yet active */
    MutationTrait candidates[8];
    int count = 0;
    for (int i = 0; i < 8; i++) {
        MutationTrait t = (MutationTrait)(1 << i);
        if (!(v->activeTraits & t)) candidates[count++] = t;
    }
    if (count == 0) return; /* fully mutated */

    MutationTrait chosen = candidates[rand() % count];
    v->activeTraits |= chosen;

    switch (chosen) {
        case TRAIT_AIRBORNE:        v->infectivity += 0.15f; break;
        case TRAIT_DRUG_RESISTANT:  v->resistance   += 0.20f; break;
        case TRAIT_LETHAL:          v->severity     += 0.15f; break;
        case TRAIT_FAST_SPREAD:     v->infectivity += 0.10f; break;
        case TRAIT_LONG_INCUBATION: v->infectivity += 0.05f; break;
        /* STEALTH, COLD_ADAPTED, HOT_ADAPTED: flag only for now,
           region.c will read these bits once it exists */
        default: break;
    }
}

int Virus_HasTrait(const Virus *v, MutationTrait t)
{
    return (v->activeTraits & t) != 0;
}

const char *Virus_TraitName(MutationTrait t)
{
    switch (t) {
        case TRAIT_AIRBORNE:        return "Airborne";
        case TRAIT_DRUG_RESISTANT:  return "Drug-Resistant";
        case TRAIT_STEALTH:         return "Stealth";
        case TRAIT_LETHAL:          return "Lethal";
        case TRAIT_FAST_SPREAD:     return "Fast Spread";
        case TRAIT_COLD_ADAPTED:    return "Cold-Adapted";
        case TRAIT_HOT_ADAPTED:     return "Hot-Adapted";
        case TRAIT_LONG_INCUBATION: return "Long Incubation";
        default: return "Unknown";
    }
}