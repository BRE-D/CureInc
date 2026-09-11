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
    c->productionRate     = 0.0f;
    c->globalDistributed  = 0.0f;
    c->funding            = 100.0f;
    c->fundingPerTick     = 5.0f;
    c->researchPoints     = 0.0f;
    c->rpPerTick          = 1.0f;
}

/*
 * cure_update - Advances the cure research pipeline through four phases.
 *               Research speed is affected by stability (virus mutations),
 *               and regional research contributions are aggregated to boost
 *               global progress. Distribution effectiveness depends on
 *               final stability value locked at the end of Phase 3.
 */
void cure_update(GameState *gs, float dtDays)
{
    CureState *c = &gs->cure;
    
    c->funding += c->fundingPerTick * dtDays;
    c->researchPoints += c->rpPerTick * dtDays;

    if (c->phase < PHASE_DISTRIBUTION)
    {
        /* Aggregate regional research contributions */
        float regionalBoost = 0.0f;
        for (int i = 0; i < MAX_REGIONS; i++) {
            regionalBoost += gs->regions[i].cureResearch * 0.01f;
        }
        
        /* Research progress: base rate + regional boost, scaled by stability */
        c->researchProgress += (c->rpPerTick + regionalBoost) * c->stability * dtDays;

        if (c->researchProgress >= 100.0f)
        {
            c->researchProgress = 0.0f;
            c->phase++;

            if (c->phase == PHASE_DISTRIBUTION)
                c->effectiveness = c->stability; /* lock in final potency */
        }
    }
    else
    {
        /* PHASE_DISTRIBUTION: roll out doses globally */
        c->globalDistributed += 0.02f * c->effectiveness * dtDays;
        if (c->globalDistributed > 1.0f) c->globalDistributed = 1.0f;//victory
    }
}