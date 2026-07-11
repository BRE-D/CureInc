#include "cure.h"

/*
 * cure_init - Sets starting values for the cure research/production
 *             pipeline. Called once at the start of a new game.
 */
void cure_init(CureState *c)
{
    c->phase             = PHASE_DISCOVERY;
    c->researchProgress  = 0.0f;
    c->stability          = 1.0f;
    c->effectiveness      = 0.0f;
    c->productionRate     = 0.0f;
    c->globalDistributed  = 0.0f;
    c->funding            = 100.0f;
    c->fundingPerTick     = 5.0f;
    c->researchPoints     = 0.0f;
    c->rpPerTick          = 1.0f;
}

/*
 * cure_update - Minimal placeholder progression through the four
 *               research phases, ending in global distribution.
 *               This is intentionally simple: enough to give the
 *               game a real win condition. Skill-tree modifiers,
 *               funding costs, and stability effects (from virus
 *               mutation) can be layered on top later without
 *               changing this function's shape.
 */
void cure_update(CureState *c, float dtDays)
{
    c->funding += c->fundingPerTick * dtDays;
    c->researchPoints += c->rpPerTick * dtDays;

    if (c->phase < PHASE_DISTRIBUTION)
    {
        c->researchProgress += c->rpPerTick * dtDays;

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
        if (c->globalDistributed > 1.0f) c->globalDistributed = 1.0f;
    }
}