#include "virus.h"
#include <stdlib.h>

// Minimum gap in game days between mutations.
#define MUTATION_MIN_DAYS 20
// Gap at which a mutation is guaranteed instead of rolling a chance.
#define MUTATION_MAX_DAYS 30
// Maximum bed share of original regional population, scaled by healthcare score.
#define HOSPITAL_BED_SHARE 0.05f
// Fraction of infected people assumed to require a hospital bed.
#define HOSPITAL_CASE_SHARE 0.10f


// Return value limited to the inclusive range low..high. Used to keep fractions and calculated counts
// inside valid bounds.
// value: number to limit.
// low: smallest allowed value.
// high: largest allowed value.
static float clamp(float value, float low, float high)
{
    if (value < low) return low;
    if (value > high) return high;
    return value;
}


// Return the region hospital score (0..1), including 0.002 extra capacity per local research point. Used
// by hospital load, spread prevention, and mortality calculations.
// r: region whose data is being examined.
static float effective_healthcare(const Region *r)
{
    return clamp(r->healthcareCapacity + r->cureResearch * 0.002f,
                 0.0f, 1.0f);
}


// Clear the virus state and set starting spread, death, recovery, and mutation rates. Called by
// reset_game; returns nothing.
// v: virus data; const means read-only access.
void virus_init(Virus *v)
{
    *v = (Virus){0};

    v->infectivity = 0.13f;
    v->severity = 0.005f;
    v->recoveryRate = 0.030f;
    v->mutationRate = 0.12f;
    v->lastMutation = TRAIT_NONE;
}


// Test a trait bit using bitwise AND. Return 1 if that bit is active, otherwise 0; does not change the
// virus.
// v: virus data; const means read-only access.
// t: mutation flag to test or name.
int virus_has_trait(const Virus *v, MutationTrait t)
{
    return (v->activeTraits & t) != 0;
}


// Return a display name for one mutation flag, or "None yet" for an unknown/no flag. The returned string
// is read-only.
// t: mutation flag to test or name.
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


// Attempt a mutation 20..30 days after the last one; day 30 guarantees it. Keep old trait bits, apply the
// chosen effects, and cap rates. Repeated traits are allowed. Return 1 if mutated, otherwise 0.
// v: virus data; const means read-only access.
// day: current game-day number.
int virus_try_mutate(Virus *v, int day)
{
    // Game days since the previous mutation.
    int elapsed = day - v->lastMutationDay;

    if (elapsed < MUTATION_MIN_DAYS) return 0;

    if (elapsed < MUTATION_MAX_DAYS) {
        // Random number in [0,1), compared with the daily mutation probability.
        double roll = (double)rand() / ((double)RAND_MAX + 1.0);

        if (roll >= v->mutationRate) return 0;
    }

    // One randomly selected mutation bit; it may already be present.
    MutationTrait chosen = (MutationTrait)(1 << (rand() % 8));

    // Bitwise OR adds the chosen flag without clearing previously acquired traits.
    v->activeTraits |= chosen;
    v->lastMutation = chosen;
    v->lastMutationDay = day;

    // Increase relative spread strength by 3%; resistance below rises by 5 percentage points.
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


// Return patient demand divided by available beds. A result above 1 means overload; a small minimum bed
// value prevents division by zero.
// r: region whose data is being examined.
float virus_hospital_load(const Region *r)
{
    // Available bed fraction of original regional population, with a nonzero minimum.
    float beds = effective_healthcare(r) * HOSPITAL_BED_SHARE;
    // Fraction of original regional population requiring hospital beds.
    float demand = r->infected * HOSPITAL_CASE_SHARE;

    if (beds < 0.0001f) beds = 0.0001f;

    return demand / beds;
}


// Recalculate population-weighted global totals after region changes. Infection and death use original
// population; vaccination uses living population. Called at initialization and after each daily update;
// returns nothing.
// gs: shared game state; const means this function only reads it.
void virus_refresh_totals(GameState *gs)
{
    // Total original world population in millions.
    float population = 0.0f;
    // Population-weighted number currently infected, in millions.
    float infected = 0.0f;
    // Population-weighted cumulative deaths, in millions.
    float dead = 0.0f;
    // Population-weighted number successfully protected, in millions.
    float vaccinated = 0.0f;

    // i: Zero-based index used to visit each item in this loop.
    for (int i = 0; i < MAX_REGIONS; i++) {
        // Pointer to the region currently being processed.
        const Region *r = &gs->regions[i];

        population += r->population;
        infected += r->population * r->infected;
        dead += r->population * r->dead;
        vaccinated += r->population * r->vaccinated;
    }

    if (population <= 0.0f) return;

    gs->virus.globalInfected = infected / population;
    gs->virus.globalDead = dead / population;

    // Surviving world population in millions; denominator for vaccine coverage.
    float living = population - dead;

    gs->cure.globalDistributed = living > 0.0f
        ? clamp(vaccinated / living, 0.0f, 1.0f)
        : 0.0f;
}


// Advance infections, deaths, recoveries, and hospital overload by one day. Read a snapshot of starting
// infections so region order does not change spread. Recoveries become susceptible again; returns nothing.
// gs: shared game state; const means this function only reads it.
void virus_update(GameState *gs)
{
    // Pointer to the shared virus state.
    Virus *v = &gs->virus;
    // Array storing each region infected fraction before any region is updated this day.
    float previous[MAX_REGIONS];


    // i: Zero-based index used to visit each item in this loop.
    for (int i = 0; i < MAX_REGIONS; i++)
        previous[i] = gs->regions[i].infected;

    // i: Zero-based index used to visit each item in this loop.
    for (int i = 0; i < MAX_REGIONS; i++) {
        // Pointer to the region currently being processed.
        Region *r = &gs->regions[i];


        // Susceptible fraction: original population minus infected, dead, and protected fractions.
        float healthy = clamp(
            1.0f - previous[i] - r->dead - r->vaccinated,
            0.0f, 1.0f
        );

        // Effective regional healthcare score after local research, limited to 0..1.
        float healthcare = effective_healthcare(r);

        // Effective border strength; forced to 1 for a player-closed border.
        float border = r->bordersClosed ? 1.0f : r->borderControl;
        border = clamp(border, 0.0f, 1.0f);

        // Spread multiplier: 1 normally, 1.15 when an active adaptation matches the region climate.
        float climate = 1.0f;

        if (r->climate == CLIMATE_COLD &&
            virus_has_trait(v, TRAIT_COLD_ADAPTED))
            climate = 1.15f;

        if (r->climate == CLIMATE_HOT &&
            virus_has_trait(v, TRAIT_HOT_ADAPTED))
            climate = 1.15f;

        // Infected people in other open-border regions, in millions, using the daily snapshot.
        float sourceCases = 0.0f;
        // Original population of all other regions in millions, including closed-border regions.
        float sourcePopulation = 0.0f;

        // j: Index of another region, used to calculate imported infection pressure.
        for (int j = 0; j < MAX_REGIONS; j++) {
            if (j == i) continue;

            // Read-only pointer to another region contributing to imported infection pressure.
            const Region *source = &gs->regions[j];

            // All other regions count in the denominator; only open sources add infected people below.
            sourcePopulation += source->population;

            if (!source->bordersClosed)
                sourceCases += previous[j] * source->population;
        }


        // Population-weighted infection pressure from other regions; closed sources add no cases.
        float imported = sourcePopulation > 0.0f
            ? sourceCases / sourcePopulation
            : 0.0f;

        // Cross-region transmission factor; Fast Spread increases it.
        float mixing = GLOBAL_MIXING_RATE;

        if (virus_has_trait(v, TRAIT_FAST_SPREAD))
            mixing *= 1.5f;

        // Spread multiplier remaining after healthcare prevention; a smaller value is safer.
        float prevention = 1.0f - healthcare * 0.30f;

        if (virus_has_trait(v, TRAIT_STEALTH))
            prevention = 1.0f - healthcare * 0.15f;


        // Combined local and imported infection pressure after climate, healthcare, and border effects.
        float exposure = v->infectivity * climate * prevention *
            (previous[i] * (1.0f - border * 0.5f)
             + mixing * imported * (1.0f - border));


        // Newly infected fraction of original region population, capped to susceptible people.
        float newCases = clamp(exposure * healthy, 0.0f, healthy);

        // Daily fraction of infected people dying after healthcare and overload adjustments.
        float deathRate = v->severity * (1.0f - healthcare * 0.5f);

        if (virus_hospital_load(r) > 1.0f)
            // Overload doubles the adjusted daily death rate.
            deathRate *= 2.0f;

        // This day deaths as a fraction of original region population.
        float deaths = clamp(
            previous[i] * deathRate,
            0.0f, previous[i]
        );


        // This day recoveries, capped so no person is counted as both dead and recovered.
        float recoveries = clamp(
            previous[i] * v->recoveryRate,
            0.0f, previous[i] - deaths
        );

        // New cases enter infection; deaths and recoveries leave it. Recovered people rejoin the
        // susceptible remainder.
        r->infected = previous[i] + newCases - deaths - recoveries;
        r->dead += deaths;

        if (virus_hospital_load(r) > 1.0f)
            r->overloadedDays++;
        else
            // Recovery of hospital capacity breaks the consecutive overload streak.
            r->overloadedDays = 0;
    }
}
