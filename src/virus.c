#include "virus.h"
#include <stdlib.h>

/*
 * virus_init - Sets the pathogen's starting biological stats.
 *              Called once at the start of a new game.
 */
void virus_init(Virus *v)
{
    v->infectivity    = 0.08f;
    v->severity       = 0.02f;
    v->resistance     = 0.0f;
    v->mutationRate   = 0.03f;    /* ~3% chance per day */
    v->activeTraits   = TRAIT_NONE;
    v->globalInfected = 0.0001f;
    v->globalDead     = 0.0f;
}

/*
 * virus_update - Flat, single-number infection model.
 *                NOT currently called from main.c: once regions are
 *                simulated, globalInfected/globalDead are calculated
 *                from region data instead (see main.c). Kept here as
 *                a standalone reference/test function.
 */
void virus_update(Virus *v, float dtDays)
{
    float growth = v->infectivity * v->globalInfected
                   * (1.0f - v->globalInfected) * dtDays;

    v->globalInfected += growth;
    if (v->globalInfected > 1.0f) v->globalInfected = 1.0f;

    v->globalDead += v->severity * v->globalInfected * dtDays;
    if (v->globalDead > v->globalInfected) v->globalDead = v->globalInfected;
}

/*
 * virus_try_mutate - Rolls a chance each day for the virus to gain
 *                    a new trait bit. Applies that trait's stat
 *                    modifier immediately on success.
 */
void virus_try_mutate(Virus *v, float dtDays)
{
    float roll = (float)rand() / (float)RAND_MAX;
    if (roll > v->mutationRate * dtDays) return;

    MutationTrait candidates[8];
    int count = 0;
    for (int i = 0; i < 8; i++) {
        MutationTrait t = (MutationTrait)(1 << i);
        if (!(v->activeTraits & t)) candidates[count++] = t;
    }
    if (count == 0) return; /* fully mutated, nothing left to gain */

    MutationTrait chosen = candidates[rand() % count];
    v->activeTraits |= chosen;

    switch (chosen) {
        case TRAIT_AIRBORNE:        v->infectivity += 0.15f; break;
        case TRAIT_DRUG_RESISTANT:  v->resistance   += 0.20f; break;
        case TRAIT_LETHAL:          v->severity     += 0.15f; break;
        case TRAIT_FAST_SPREAD:     v->infectivity += 0.10f; break;
        case TRAIT_LONG_INCUBATION: v->infectivity += 0.05f; break;
        /* STEALTH, COLD_ADAPTED, HOT_ADAPTED: bit is set but inert
           until region.c exposes a climate/detection field to react to. */
        default: break;
    }
}

int virus_has_trait(const Virus *v, MutationTrait t)
{
    return (v->activeTraits & t) != 0;
}

const char *virus_trait_name(MutationTrait t)
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