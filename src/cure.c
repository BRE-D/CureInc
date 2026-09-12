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
    c->funding            = 200.0f;    /* Start with some initial funding */
    c->fundingPerTick     = 50.0f;    /* Much faster income - 2 days for first scientist */
    c->researchPoints     = 0.0f;
    c->rpPerTick          = 5.0f;     /* Faster base research - 20 days per phase */
    
    /* Initialize gameplay systems */
    c->scientistCount     = 0;
    c->labLevel           = 0;
    c->productionLevel    = 0;
    c->vaccineStockpile   = 0.0f;
}

/*
 * cure_update - Advances the cure research pipeline through four phases.
 *               Research speed is affected by stability (virus mutations),
 *               scientist count, lab level, and regional research contributions.
 *               Production phase generates vaccine stockpile based on production level.
 *               Distribution phase deploys vaccines to reduce regional infections.
 */
void cure_update(GameState *gs, float dtDays)
{
    CureState *c = &gs->cure;
    
    c->funding += c->fundingPerTick * dtDays;
    c->researchPoints += c->rpPerTick * dtDays;

    /* Calculate scientist bonus: each scientist adds 10% to research speed */
    float scientistMultiplier = 1.0f + (c->scientistCount * 0.10f);
    
    /* Calculate lab bonus: each level adds 15% to research speed */
    float labMultiplier = 1.0f + (c->labLevel * 0.15f);

    if (c->phase == PHASE_DISCOVERY || c->phase == PHASE_TRIALS)
    {
        /* Research phases: Discovery and Trials */
        float regionalBoost = 0.0f;
        for (int i = 0; i < MAX_REGIONS; i++) {
            regionalBoost += gs->regions[i].cureResearch * 0.01f;
        }
        
        /* Research progress: base rate + regional boost + scientist/lab bonuses, scaled by stability */
        float totalResearchRate = (c->rpPerTick + regionalBoost) * scientistMultiplier * labMultiplier;
        c->researchProgress += totalResearchRate * c->stability * dtDays;

        if (c->researchProgress >= 100.0f)
        {
            c->researchProgress = 0.0f;
            c->phase++;
        }
    }
    else if (c->phase == PHASE_PRODUCTION)
    {
        /* Production phase: manufacture vaccine doses */
        
        /* Production rate based on facility level and scientist count */
        /* Base: 1.0/day, +0.5/day per production level, +0.2/day per scientist */
        c->productionRate = 1.0f + (c->productionLevel * 0.5f) + (c->scientistCount * 0.2f);
        
        /* Accumulate vaccine stockpile */
        c->vaccineStockpile += c->productionRate * dtDays;
        
        /* Auto-advance to distribution when stockpile reaches threshold */
        /* Need enough doses for initial distribution (10 units = ready for global rollout) */
        if (c->vaccineStockpile >= 10.0f)
        {
            c->phase = PHASE_DISTRIBUTION;
            c->effectiveness = c->stability; /* lock in final potency */
        }
    }
    else if (c->phase == PHASE_DISTRIBUTION)
    {
        /* Distribution phase: deploy vaccines to reduce infection */
        
        /* Continue producing vaccines */
        c->productionRate = 1.0f + (c->productionLevel * 0.5f) + (c->scientistCount * 0.2f);
        c->vaccineStockpile += c->productionRate * dtDays;
        
        /* Distribute vaccines globally */
        float distributionRate = 0.015f * c->effectiveness * dtDays;
        
        /* Consume stockpile for distribution (1 dose = 1% distribution) */
        float dosesNeeded = distributionRate * 100.0f;
        if (c->vaccineStockpile >= dosesNeeded)
        {
            c->vaccineStockpile -= dosesNeeded;
            c->globalDistributed += distributionRate;
            
            /* Apply vaccination to regions proportionally */
            float totalPop = 0.0f;
            for (int i = 0; i < MAX_REGIONS; i++) {
                totalPop += gs->regions[i].population;
            }
            
            for (int i = 0; i < MAX_REGIONS; i++) {
                float regionShare = gs->regions[i].population / totalPop;
                float regionVaccines = distributionRate * regionShare;
                gs->regions[i].vaccinated += regionVaccines;
                
                /* Reduce infection as vaccination increases */
                /* Each 1% vaccinated reduces infection by 0.5% directly */
                float infectionReduction = regionVaccines * 0.5f * c->effectiveness;
                gs->regions[i].infected -= infectionReduction;
                if (gs->regions[i].infected < 0.0f) gs->regions[i].infected = 0.0f;
                if (gs->regions[i].vaccinated > 1.0f) gs->regions[i].vaccinated = 1.0f;
            }
        }
        
        if (c->globalDistributed > 1.0f) c->globalDistributed = 1.0f; /* victory */
    }
}

/*
 * cure_hire_scientist - Hire a scientist to boost research speed.
 *                       Cost: 100 funding per scientist.
 *                       Effect: +10% research speed per scientist.
 */
int cure_hire_scientist(CureState *c)
{
    const float SCIENTIST_COST = 100.0f;
    
    if (c->funding >= SCIENTIST_COST)
    {
        c->funding -= SCIENTIST_COST;
        c->scientistCount++;
        return 1;
    }
    return 0;
}

/*
 * cure_upgrade_lab - Upgrade research lab to boost research speed.
 *                    Cost: 150 * (level + 1) funding.
 *                    Effect: +15% research speed per level.
 *                    Max level: 3
 */
int cure_upgrade_lab(CureState *c)
{
    const int MAX_LAB_LEVEL = 3;
    
    if (c->labLevel >= MAX_LAB_LEVEL)
        return 0; /* already at max level */
    
    float cost = 150.0f * (c->labLevel + 1);
    
    if (c->funding >= cost)
    {
        c->funding -= cost;
        c->labLevel++;
        return 1;
    }
    return 0;
}

/*
 * cure_upgrade_production - Upgrade vaccine production facility.
 *                           Cost: 200 * (level + 1) funding.
 *                           Effect: +0.5 doses/day per level.
 *                           Max level: 3
 */
int cure_upgrade_production(CureState *c)
{
    const int MAX_PRODUCTION_LEVEL = 3;
    
    if (c->productionLevel >= MAX_PRODUCTION_LEVEL)
        return 0; /* already at max level */
    
    float cost = 200.0f * (c->productionLevel + 1);
    
    if (c->funding >= cost)
    {
        c->funding -= cost;
        c->productionLevel++;
        return 1;
    }
    return 0;
}