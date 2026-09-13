#include "cure.h"

// নতুন খেলার research, টাকা ও vaccine-এর প্রাথমিক মান।
void cure_init(CureState *c) {
    *c = (CureState){0}; // আগে সব field শূন্য করি; তাই পুরোনো খেলার তথ্য থাকে না।
    c->phase = PHASE_DISCOVERY;
    c->stability = 1.0f;
    c->effectiveness = 1.0f;
    c->productionRate = 1.0f;
    c->funding = 50.0f; // শুরুর টাকা।
    c->fundingPerTick = 10.0f; // প্রতিদিন আয়; ৫ দিনে প্রথম scientist কেনা যায়।
    c->rpPerTick = 0.5f; // upgrade ছাড়া দৈনিক মূল research।
}

#define VACCINE_UNIT_SHARE 0.01f // ১ unit-এ শুরুর জনসংখ্যার ১%-এর dose।
#define INITIAL_STOCK_GOAL 10.0f // এই মজুত হলে Distribution খুলবে।

// প্রতিদিন কয় unit vaccine তৈরি হবে; scientist ও upgrade উৎপাদন বাড়ায়।
float cure_production_rate(const CureState *c)
{
    return 1.0f + c->productionLevel * 0.5f + c->scientistCount * 0.2f;
}

// আসল research/day; simulation এবং UI দুটোই এই একই formula ব্যবহার করে।
float cure_research_rate(const GameState *gs)
{
    const CureState *c = &gs->cure;
    if (c->phase != PHASE_DISCOVERY && c->phase != PHASE_TRIALS)
        return 0.0f;

    float regionalBoost = 0.0f;
    for (int i = 0; i < MAX_REGIONS; i++)
        regionalBoost += gs->regions[i].cureResearch * 0.008f; // ১৫ point-এ +০.১২ মূল RP/day।

    // আগে মূল research, পরে scientist, lab, stability এবং resistance-এর প্রভাব।
    return (c->rpPerTick + regionalBoost)
        * (1.0f + c->scientistCount * 0.20f) // scientist-প্রতি multiplier-এ ০.২০ যোগ।
        * (1.0f + c->labLevel * 0.25f) // lab level-প্রতি multiplier-এ ০.২৫ যোগ।
        * c->stability * (1.0f - gs->virus.resistance * 0.5f);
}

// বর্তমান ধাপের সঠিক progress: research, stock অথবা vaccination।
float cure_phase_progress(const CureState *c)
{
    float progress = c->researchProgress;
    if (c->phase == PHASE_PRODUCTION)
        progress = c->vaccineStockpile / INITIAL_STOCK_GOAL * 100.0f;
    else if (c->phase == PHASE_DISTRIBUTION)
        progress = c->globalDistributed * 100.0f;

    if (progress < 0.0f) return 0.0f;
    if (progress > 100.0f) return 100.0f;
    return progress;
}

// প্রতিদিন আয় যোগ করি; ধাপ অনুযায়ী research, উৎপাদন বা বিতরণ করি।
void cure_update(GameState *gs, float dtDays)
{
    CureState *c = &gs->cure;
    if (dtDays <= 0.0f) return;

    c->funding += c->fundingPerTick * dtDays;
    c->productionRate = cure_production_rate(c);

    // UI-তে সব ধাপেই বর্তমান vaccine effectiveness দেখাই।
    c->effectiveness = c->stability * (1.0f - gs->virus.resistance * 0.25f);
    if (c->effectiveness < 0.0f) c->effectiveness = 0.0f;
    if (c->effectiveness > 1.0f) c->effectiveness = 1.0f;

    if (c->phase == PHASE_DISCOVERY || c->phase == PHASE_TRIALS) {
        c->researchProgress += cure_research_rate(gs) * dtDays;
        if (c->researchProgress >= 100.0f) {
            c->researchProgress = 0.0f;
            c->phase++; // ধাপ শেষ; পরের ধাপে যাই।
        }
        return; // research চলাকালে vaccine তৈরি হয় না।
    }

    c->vaccineStockpile += c->productionRate * dtDays;

    if (c->phase == PHASE_PRODUCTION) {
        if (c->vaccineStockpile >= INITIAL_STOCK_GOAL) {
            c->phase = PHASE_DISTRIBUTION;
            if (c->completionDay == 0) c->completionDay = gs->day;
        }
        return;
    }

    if (c->phase != PHASE_DISTRIBUTION || c->effectiveness <= 0.0f)
        return;

    float totalPop = 0.0f;
    for (int i = 0; i < MAX_REGIONS; i++)
        totalPop += gs->regions[i].population;
    if (totalPop <= 0.0f || c->vaccineStockpile <= 0.0f) return;

    // সমান স্থানীয় fraction দিলে বড় অঞ্চল নিজের জনসংখ্যা অনুযায়ী বেশি dose পায়।
    float offeredFraction = c->vaccineStockpile * VACCINE_UNIT_SHARE;
    float usedUnits = 0.0f;
    for (int i = 0; i < MAX_REGIONS; i++) {
        Region *r = &gs->regions[i];
        float healthy = 1.0f - r->infected - r->dead - r->vaccinated;
        if (healthy < 0.0f) healthy = 0.0f;

        float vaccinatedToday = offeredFraction;
        if (vaccinatedToday > healthy) vaccinatedToday = healthy;

        r->vaccinated += vaccinatedToday * c->effectiveness; // শুধু সফল dose-এ সুরক্ষা।
        // ব্যর্থ dose-ও খরচ হয়; তবে অব্যবহৃত stock থেকে যায়।
        usedUnits += vaccinatedToday / VACCINE_UNIT_SHARE
            * (r->population / totalPop);
    }
    c->vaccineStockpile -= usedUnits; // vaccine আক্রান্তকে সরাসরি সারায় না।
    if (c->vaccineStockpile < 0.0f) c->vaccineStockpile = 0.0f;
}

// টাকা যথেষ্ট হলে scientist কিনি; সফল হলে ১, না হলে ০ ফেরত দিই।
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

// টাকা ও সর্বোচ্চ level পরীক্ষা করে laboratory উন্নত করি।
int cure_upgrade_lab(CureState *c)
{
    const int MAX_LAB_LEVEL = 3;

    if (c->phase >= PHASE_PRODUCTION || c->labLevel >= MAX_LAB_LEVEL) // research শেষ হলে অকারণে টাকা কাটব না।
        return 0;

    float cost = LAB_BASE_COST * (c->labLevel + 1);

    if (c->funding >= cost)
    {
        c->funding -= cost;
        c->labLevel++;
        return 1;
    }
    return 0;
}

// উৎপাদন level বাড়িয়ে নতুন দৈনিক ক্ষমতা সঙ্গে সঙ্গে বসাই।
int cure_upgrade_production(CureState *c)
{
    const int MAX_PRODUCTION_LEVEL = 3;

    if (c->productionLevel >= MAX_PRODUCTION_LEVEL)
        return 0;

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
