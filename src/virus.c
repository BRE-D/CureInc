#include "virus.h"
#include <stdlib.h>

#define MUTATION_MIN_DAYS 20
#define MUTATION_MAX_DAYS 30
#define HOSPITAL_BED_SHARE 0.05f
#define HOSPITAL_CASE_SHARE 0.10f

// সংখ্যাকে সর্বনিম্ন ও সর্বোচ্চ সীমার মধ্যে রাখি।
static float clamp(float value, float low, float high)
{
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

// স্থানীয় research বাড়লে হাসপাতালের কার্যকর ক্ষমতা বাড়ে।
static float effective_healthcare(const Region *r)
{
    return clamp(r->healthcareCapacity + r->cureResearch * 0.002f,
                 0.0f, 1.0f);
}

// নতুন খেলার virus-এর প্রাথমিক মান বসাই।
void virus_init(Virus *v)
{
    *v = (Virus){0};

    v->infectivity = 0.13f; // বাড়ালে রোগ দ্রুত ছড়াবে।
    v->severity = 0.005f; // বাড়ালে আক্রান্তদের মৃত্যু বেশি হবে।
    v->recoveryRate = 0.030f; // দিনে আক্রান্তদের ৩% সুস্থ হওয়ার মূল হার।
    v->mutationRate = 0.12f;
    v->lastMutation = TRAIT_NONE;
}

// & দিয়ে দেখি নির্দিষ্ট বৈশিষ্ট্যের bit চালু আছে কি না।
int virus_has_trait(const Virus *v, MutationTrait t)
{
    return (v->activeTraits & t) != 0;
}

// বৈশিষ্ট্যের code থেকে পর্দায় দেখানোর নাম পাই।
const char *virus_trait_name(MutationTrait t)
{
    switch (t) {
        case TRAIT_AIRBORNE: return "Airborne";
        case TRAIT_DRUG_RESISTANT: return "Drug-Resistant";
        case TRAIT_STEALTH: return "Stealth";
        case TRAIT_LETHAL: return "Lethal";
        case TRAIT_FAST_SPREAD: return "Fast Spread";
        case TRAIT_COLD_ADAPTED: return "Cold-Adapted";
        case TRAIT_HOT_ADAPTED: return "Hot-Adapted";
        case TRAIT_LONG_INCUBATION: return "Long Incubation";
        default: return "None yet";
    }
}

// আগের mutation-এর ২০–৩০ দিন পরে virus বদলায়।
int virus_try_mutate(Virus *v, int day)
{
    int elapsed = day - v->lastMutationDay;

    if (elapsed < MUTATION_MIN_DAYS) return 0;

    if (elapsed < MUTATION_MAX_DAYS) { // ৩০ দিন হলে আর random roll নয়, mutation নিশ্চিত।
        double roll = (double)rand() / ((double)RAND_MAX + 1.0);

        if (roll >= v->mutationRate) return 0;
    }

    MutationTrait chosen = (MutationTrait)(1 << (rand() % 8));

    v->activeTraits |= chosen; // পুরোনো trait না মুছে নতুন bit চালু করি।
    v->lastMutation = chosen;
    v->lastMutationDay = day;

    v->infectivity *= 1.03f; // আগের infectivity-এর তুলনায় ৩% বৃদ্ধি।
    v->resistance += 0.05f; // resistance-এ ৫ percentage point যোগ।

    switch (chosen) {
        case TRAIT_AIRBORNE:
            v->infectivity += 0.005f;
            break;

        case TRAIT_DRUG_RESISTANT:
            v->resistance += 0.02f;
            break;

        case TRAIT_LETHAL:
            v->severity += 0.001f;
            break;

        case TRAIT_LONG_INCUBATION:
            v->infectivity += 0.003f;
            break;

        default:
            break;
    }

    v->infectivity = clamp(v->infectivity, 0.0f, 0.35f);
    v->severity = clamp(v->severity, 0.0f, 0.02f);
    v->resistance = clamp(v->resistance, 0.0f, 0.80f);

    return 1;
}

// রোগীর চাহিদা / শয্যা; ১-এর বেশি হলে হাসপাতাল overloaded।
float virus_hospital_load(const Region *r)
{
    float beds = effective_healthcare(r) * HOSPITAL_BED_SHARE;
    float demand = r->infected * HOSPITAL_CASE_SHARE;

    if (beds < 0.0001f) beds = 0.0001f;

    return demand / beds;
}

// সব অঞ্চলের মানুষ গুনে বিশ্বব্যাপী হার বের করি; শতাংশের সরল গড় নয়।
void virus_refresh_totals(GameState *gs)
{
    float population = 0.0f;
    float infected = 0.0f;
    float dead = 0.0f;
    float vaccinated = 0.0f;

    for (int i = 0; i < MAX_REGIONS; i++) {
        const Region *r = &gs->regions[i];

        population += r->population;
        infected += r->population * r->infected;
        dead += r->population * r->dead;
        vaccinated += r->population * r->vaccinated;
    }

    if (population <= 0.0f) return;

    gs->virus.globalInfected = infected / population;
    gs->virus.globalDead = dead / population;

    float living = population - dead;

    gs->cure.globalDistributed = living > 0.0f
        ? clamp(vaccinated / living, 0.0f, 1.0f)
        : 0.0f;
}

// দিনে একবার নতুন সংক্রমণ, মৃত্যু ও সুস্থ হওয়া হিসাব করি।
void virus_update(GameState *gs)
{
    Virus *v = &gs->virus;
    float previous[MAX_REGIONS];

    // একই দিনের শুরুর আক্রান্তের তথ্য রাখি; region-এর ক্রমে ফল বদলাবে না।
    for (int i = 0; i < MAX_REGIONS; i++)
        previous[i] = gs->regions[i].infected;

    for (int i = 0; i < MAX_REGIONS; i++) {
        Region *r = &gs->regions[i];

        // সুস্থ = মোট ১ - আক্রান্ত - মৃত - vaccine-এ সুরক্ষিত।
        float healthy = clamp(
            1.0f - previous[i] - r->dead - r->vaccinated,
            0.0f, 1.0f
        );

        float healthcare = effective_healthcare(r);

        float border = r->bordersClosed ? 1.0f : r->borderControl;
        border = clamp(border, 0.0f, 1.0f);

        float climate = 1.0f;

        if (r->climate == CLIMATE_COLD &&
            virus_has_trait(v, TRAIT_COLD_ADAPTED))
            climate = 1.15f;

        if (r->climate == CLIMATE_HOT &&
            virus_has_trait(v, TRAIT_HOT_ADAPTED))
            climate = 1.15f;

        float sourceCases = 0.0f;
        float sourcePopulation = 0.0f;

        for (int j = 0; j < MAX_REGIONS; j++) {
            if (j == i) continue;

            const Region *source = &gs->regions[j];

            sourcePopulation += source->population;

            if (!source->bordersClosed)
                sourceCases += previous[j] * source->population;
        }

        // অন্য অঞ্চলের জনসংখ্যা অনুযায়ী বাইরের আক্রান্তের গড় বের করি।
        float imported = sourcePopulation > 0.0f
            ? sourceCases / sourcePopulation
            : 0.0f;

        float mixing = GLOBAL_MIXING_RATE;

        if (virus_has_trait(v, TRAIT_FAST_SPREAD))
            mixing *= 1.5f;

        float prevention = 1.0f - healthcare * 0.30f;

        if (virus_has_trait(v, TRAIT_STEALTH))
            prevention = 1.0f - healthcare * 0.15f;

        // স্থানীয় সংক্রমণ এবং বাইরের সংক্রমণ যোগ করে প্রতিরোধের প্রভাব ধরি।
        float exposure = v->infectivity * climate * prevention *
            (previous[i] * (1.0f - border * 0.5f)
             + mixing * imported * (1.0f - border));

        // সুস্থ মানুষের চেয়ে বেশি নতুন আক্রান্ত হওয়া সম্ভব নয়।
        float newCases = clamp(exposure * healthy, 0.0f, healthy);

        float deathRate = v->severity * (1.0f - healthcare * 0.5f);

        if (virus_hospital_load(r) > 1.0f)
            deathRate *= 2.0f; // হাসপাতাল ভরে গেলে মৃত্যুর হার দ্বিগুণ।

        float deaths = clamp(
            previous[i] * deathRate,
            0.0f, previous[i]
        );

        // একই মানুষকে একই দিনে মৃত ও সুস্থ দুই জায়গায় গোনা যাবে না।
        float recoveries = clamp(
            previous[i] * v->recoveryRate,
            0.0f, previous[i] - deaths
        );

        r->infected = previous[i] + newCases - deaths - recoveries;
        r->dead += deaths; // মৃত্যু জমে; recovered মানুষ আবার সুস্থ অংশে ফেরে।

        if (virus_hospital_load(r) > 1.0f)
            r->overloadedDays++;
        else
            r->overloadedDays = 0; // স্বাভাবিক হলে একটানা overload-এর হিসাব নতুন করে শুরু।
    }
}
