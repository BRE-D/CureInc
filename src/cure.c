#include "cure.h"

/*
 * cure_init - Sets starting values for the cure research/production
 *             pipeline. Called once at the start of a new game.
 */
void cure_init(CureState *c)
{
    c->phase             = PHASE_DISCOVERY;
    c->researchProgress  = 0.0f;   
    c->stability          = 1.0f;/* The cure begins at 100% stability. 
                                    If a virus mutates rapidly,
                                    this number might drop later,
                                    rendering research volatile.
                                   */
    c->effectiveness      = 0.0f;
    c->productionRate     = 30000.0f;
    c->completionDay      = 0;
    c->globalDistributed  = 0.0f;
    c->funding            = 500.0f;    /* Start with enough for immediate actions */
    c->fundingPerTick     = 25.0f;    /* Faster income - 20 days to afford regional funding */
    c->researchPoints     = 0.0f;
    c->rpPerTick          = 2.5f;     /* 40 days per phase instead of 100 */
}

/* Advance research, then turn daily vaccine production into regional protection.
 * Global coverage is calculated by virus_refresh_totals after this function.
 */
void cure_update(GameState *gs, float dtDays)
{
    CureState *c = &gs->cure;
    c->funding += c->fundingPerTick * dtDays;
    c->researchPoints += c->rpPerTick * dtDays;

    if (c->phase < PHASE_DISTRIBUTION) {
        float regionalBoost = 0.0f;
        for (int i = 0; i < MAX_REGIONS; i++)
            regionalBoost += gs->regions[i].cureResearch * 0.01f;

        float resistanceFactor = 1.0f - gs->virus.resistance * 0.5f;
        c->researchProgress += (c->rpPerTick + regionalBoost)
            * c->stability * resistanceFactor * dtDays;

        if (c->researchProgress >= 100.0f) {
            c->researchProgress = 0.0f;
            c->phase++;
            if (c->phase == PHASE_DISTRIBUTION)
                c->completionDay = gs->day;
        }
    }

    if (c->phase != PHASE_DISTRIBUTION) return;

    c->effectiveness = c->stability * (1.0f - gs->virus.resistance * 0.25f);

    float totalPopulation = 0.0f;
    for (int i = 0; i < MAX_REGIONS; i++)
        totalPopulation += gs->regions[i].population * 1000000.0f;
    if (totalPopulation <= 0.0f) return;

    float dailyShare = c->productionRate * dtDays / totalPopulation;
    for (int i = 0; i < MAX_REGIONS; i++) {
        Region *r = &gs->regions[i];
        float healthy = 1.0f - r->infected - r->dead - r->vaccinated;
        if (healthy < 0.0f) healthy = 0.0f;
        float protectedToday = dailyShare * c->effectiveness;
        if (protectedToday > healthy) protectedToday = healthy;
        r->vaccinated += protectedToday;
    }
}
