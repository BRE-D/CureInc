#include "virus.h"
#include <stdlib.h>

/* Game balance values, not medical predictions. */
#define MUTATION_MIN_DAYS 20
#define MUTATION_MAX_DAYS 30
#define HOSPITAL_BED_SHARE 0.05f
#define HOSPITAL_CASE_SHARE 0.10f

/* Keep a number inside an allowed range. */
static float clamp(float value, float low, float high)
{
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

/* Local research improves the region's existing healthcare score. */
static float effective_healthcare(const Region *r)
{
    return clamp(r->healthcareCapacity + r->cureResearch * 0.002f,
                 0.0f, 1.0f);
}

/* Set starting virus values. */
void virus_init(Virus *v)
{
    *v = (Virus){0};

    v->infectivity = 0.13f;
    v->severity = 0.005f;
    v->recoveryRate = 0.030f;
    v->mutationRate = 0.12f;
    v->lastMutation = TRAIT_NONE;
}

/* Return true if the virus has this trait. */
int virus_has_trait(const Virus *v, MutationTrait t)
{
    return (v->activeTraits & t) != 0;
}

/* Convert a trait into text for the UI/event log. */
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

/* Mutation happens between 20 and 30 days after the previous mutation. */
int virus_try_mutate(Virus *v, int day)
{
    int elapsed = day - v->lastMutationDay;

    if (elapsed < MUTATION_MIN_DAYS) return 0;

    /* Random chance on days 20-29; guaranteed on day 30. */
    if (elapsed < MUTATION_MAX_DAYS) {
        double roll = (double)rand() / ((double)RAND_MAX + 1.0);

        if (roll >= v->mutationRate) return 0;
    }

    MutationTrait chosen = (MutationTrait)(1 << (rand() % 8));

    v->activeTraits |= chosen;
    v->lastMutation = chosen;
    v->lastMutationDay = day;

    /* Every mutation raises spread by 3% and resistance by 5 points. */
    v->infectivity *= 1.03f;
    v->resistance += 0.05f;

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

/* 1.0 means full capacity; above 1.0 means overloaded. */
float virus_hospital_load(const Region *r)
{
    float beds = effective_healthcare(r) * HOSPITAL_BED_SHARE;
    float demand = r->infected * HOSPITAL_CASE_SHARE;

    if (beds < 0.0001f) beds = 0.0001f;

    return demand / beds;
}

/* Calculate global values from regional values. */
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

/* Call exactly once for each completed game day. */
void virus_update(GameState *gs)
{
    Virus *v = &gs->virus;
    float previous[MAX_REGIONS];

    /* Snapshot prevents region update order from changing the result. */
    for (int i = 0; i < MAX_REGIONS; i++)
        previous[i] = gs->regions[i].infected;

    for (int i = 0; i < MAX_REGIONS; i++) {
        Region *r = &gs->regions[i];

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

        float imported = sourcePopulation > 0.0f
            ? sourceCases / sourcePopulation
            : 0.0f;

        float mixing = GLOBAL_MIXING_RATE;

        if (virus_has_trait(v, TRAIT_FAST_SPREAD))
            mixing *= 1.5f;

        float prevention = 1.0f - healthcare * 0.30f;

        if (virus_has_trait(v, TRAIT_STEALTH))
            prevention = 1.0f - healthcare * 0.15f;

        float exposure = v->infectivity * climate * prevention *
            (previous[i] * (1.0f - border * 0.5f)
             + mixing * imported * (1.0f - border));

        float newCases = clamp(exposure * healthy, 0.0f, healthy);

        float deathRate = v->severity * (1.0f - healthcare * 0.5f);

        if (virus_hospital_load(r) > 1.0f)
            deathRate *= 2.0f;

        float deaths = clamp(
            previous[i] * deathRate,
            0.0f, previous[i]
        );

        float recoveries = clamp(
            previous[i] * v->recoveryRate,
            0.0f, previous[i] - deaths
        );

        r->infected = previous[i] + newCases - deaths - recoveries;
        r->dead += deaths;

        /* Recovered people automatically rejoin the healthy remainder. */

        if (virus_hospital_load(r) > 1.0f)
            r->overloadedDays++;
        else
            r->overloadedDays = 0;
    }
}
