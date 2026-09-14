#include "cure.h"


//Initialize the cure state with base values
void cure_init(CureState *c) {
    *c = (CureState){0};
    c->phase = PHASE_DISCOVERY;
    c->stability = 1.0f;
    c->effectiveness = 1.0f;
    c->productionRate = 1.0f;
    c->funding = 50.0f;
    c->fundingPerTick = 10.0f;
    c->rpPerTick = 0.5f;
}

// One vaccine unit provides doses for this fraction of original world population.
#define VACCINE_UNIT_SHARE 0.01f
// Vaccine units needed to open Distribution automatically.
#define INITIAL_STOCK_GOAL 10.0f


//Returns vaccine units produced per day: Base:1.0, +0.5 per production level and +0.2 per scientist
float cure_production_rate(const CureState *c)
{
    return 1.0f + c->productionLevel * 0.5f + c->scientistCount * 0.2f;
}


// Return actual research points per day after local research, scientists, lab level, stability, and
// resistance. Returns 0 when the production phase begins
float cure_research_rate(const GameState *gs)
{
    //Returns 0 if phase is after the trials phase
    const CureState *c = &gs->cure;
    if (c->phase != PHASE_DISCOVERY && c->phase != PHASE_TRIALS)
        return 0.0f;

    // Sum of local research contributions to base research points per day.
    float regionalBoost = 0.0f;

    for (int i = 0; i < MAX_REGIONS; i++)
        regionalBoost += gs->regions[i].cureResearch * 0.008f;


    // Add base research rate + 20% addition for each scientist + 25% addition for each lab level
    // multiplied by stability and reduced by virus resistance upto 50%
    return (c->rpPerTick + regionalBoost)
        * (1.0f + c->scientistCount * 0.20f)
        * (1.0f + c->labLevel * 0.25f)
        * c->stability * (1.0f - gs->virus.resistance * 0.5f);
}


// Return current phase progress clamped to 0..100: research points, stockpile relative to 10 units, or
// protected percentage of survivors.
// c: cure data; const means read-only access.
float cure_phase_progress(const CureState *c)
{
    // Current phase completion percentage, limited to 0..100 before returning.
    float progress = c->researchProgress;
    if (c->phase == PHASE_PRODUCTION)
        progress = c->vaccineStockpile / INITIAL_STOCK_GOAL * 100.0f;
    else if (c->phase == PHASE_DISTRIBUTION)
        progress = c->globalDistributed * 100.0f;

    if (progress < 0.0f) return 0.0f;
    if (progress > 100.0f) return 100.0f;
    return progress;
}


// Add daily income, refresh capacity/effectiveness, then advance research or vaccine
// production/distribution. Called by day_tick with one day. Consume all administered doses, including
// unsuccessful ones; returns nothing.
// gs: shared game state; const means this function only reads it.
// dtDays: elapsed game days (main passes 1).
void cure_update(GameState *gs, float dtDays)
{
    CureState *c = &gs->cure;
    if (dtDays <= 0.0f) return;

    c->funding += c->fundingPerTick * dtDays;
    c->productionRate = cure_production_rate(c);


    // Refresh this in every phase so the UI always shows the current expected success fraction.
    c->effectiveness = c->stability * (1.0f - gs->virus.resistance * 0.25f);
    if (c->effectiveness < 0.0f) c->effectiveness = 0.0f;
    if (c->effectiveness > 1.0f) c->effectiveness = 1.0f;

    // For the research progression up until the production phase 
    if (c->phase == PHASE_DISCOVERY || c->phase == PHASE_TRIALS) {
        c->researchProgress += cure_research_rate(gs) * dtDays;
        if (c->researchProgress >= 100.0f) {
            c->researchProgress = 0.0f;
            // Advance one phase and reset its research counter up until the production phase
            c->phase++;
        }
        return;
    }

    // Total stored vaccine
    c->vaccineStockpile += c->productionRate * dtDays;

    if (c->phase == PHASE_PRODUCTION) {
        if (c->vaccineStockpile >= INITIAL_STOCK_GOAL) {
            // If a goal of initial stock is reached then move to the next phase
            c->phase = PHASE_DISTRIBUTION;
            // Record of the day the production phase ends
            if (c->completionDay == 0) c->completionDay = gs->day;
        }
        return;
    }

    // Don't distribute if not in distribution phase or if cure is not effective
    if (c->phase != PHASE_DISTRIBUTION || c->effectiveness <= 0.0f)
        return;

    // Sum of original regional populations in millions.
    float totalPop = 0.0f;
    
    for (int i = 0; i < MAX_REGIONS; i++)
        totalPop += gs->regions[i].population;
    if (totalPop <= 0.0f || c->vaccineStockpile <= 0.0f) return;


    // Dose supply offered to each region as a fraction of its original population.
    // Offer the same local population fraction; larger regions therefore receive more doses.
    float offeredFraction = c->vaccineStockpile * VACCINE_UNIT_SHARE;
    // Total vaccine units administered across regions, including unsuccessful doses.
    float usedUnits = 0.0f;
    
    for (int i = 0; i < MAX_REGIONS; i++) {
        // Pointer to the region currently being processed.
        Region *r = &gs->regions[i];
        // Total healthy population
        float healthy = 1.0f - r->infected - r->dead - r->vaccinated;
        if (healthy < 0.0f) healthy = 0.0f;

        // Each region gets the same fraction but if it exceeds the healthy population then equate
        float vaccinatedToday = offeredFraction;
        if (vaccinatedToday > healthy) vaccinatedToday = healthy;

        // Only successful doses add protection. Vaccines do not directly cure infected people.
        r->vaccinated += vaccinatedToday * c->effectiveness;

        // Convert administered regional fractions back into globally weighted units; failed doses also
        // cost stock.
        usedUnits += vaccinatedToday / VACCINE_UNIT_SHARE
            * (r->population / totalPop);
    }
    c->vaccineStockpile -= usedUnits;
    if (c->vaccineStockpile < 0.0f) c->vaccineStockpile = 0.0f;
}


// Buy one scientist if funding covers SCIENTIST_COST and refresh production capacity. Return 1 on
// purchase, otherwise 0. Scientists improve research and production.
int cure_hire_scientist(CureState *c)
{

    if (c->funding >= SCIENTIST_COST)
    {
        c->funding -= SCIENTIST_COST;
        c->scientistCount++;
        c->productionRate = cure_production_rate(c);
        return 1;
    }
    return 0;
}


// Buy the next lab level only during research and below level 3. Charge base cost times next level; return
// 1 on purchase, otherwise 0. Later phases have no research to accelerate.
// c: cure data; const means read-only access.
int cure_upgrade_lab(CureState *c)
{
    // Highest purchasable laboratory level.
    const int MAX_LAB_LEVEL = 3;

    if (c->phase >= PHASE_PRODUCTION || c->labLevel >= MAX_LAB_LEVEL)
        return 0;

    // Price of the next upgrade, calculated from its base price and next level.
    float cost = LAB_BASE_COST * (c->labLevel + 1);

    if (c->funding >= cost)
    {
        c->funding -= cost;
        c->labLevel++;
        return 1;
    }
    return 0;
}


// Buy the next production level below level 3 and refresh units/day. Charge base cost times next level;
// return 1 on purchase, otherwise 0.
// c: cure data; const means read-only access.
int cure_upgrade_production(CureState *c)
{
    // Highest purchasable production level.
    const int MAX_PRODUCTION_LEVEL = 3;

    if (c->productionLevel >= MAX_PRODUCTION_LEVEL)
        return 0;

    // Price of the next upgrade, calculated from its base price and next level.
    float cost = PRODUCTION_BASE_COST * (c->productionLevel + 1);

    if (c->funding >= cost)
    {
        c->funding -= cost;
        c->productionLevel++;
        c->productionRate = cure_production_rate(c);
        return 1;
    }
    return 0;
}
