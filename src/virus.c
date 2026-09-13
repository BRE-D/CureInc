#include "virus.h"
#include <stdlib.h>


#define MUTATION_MIN_DAYS 20
                                //virus cannot mutate again until at least 20 days after the prev mutation
#define MUTATION_MAX_DAYS 30
                                //mutation is guaranteed within 30 days after prev mutation
#define HOSPITAL_BED_SHARE 0.05f
                                // maximum hospital-bed capacity is based on 5% of the original regional population,multiplied by healthcare quality.
#define HOSPITAL_CASE_SHARE 0.10f
                                //10% of infected people need hospital beds.

static float clamp(float value, float low, float high)  //This function simply prevents a value from going outside a range.
{
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

static float effective_healthcare(const Region *r)      //calculates the real healthcare strength of a region.
{
    return clamp(r->healthcareCapacity + r->cureResearch * 0.002f,0.0f, 1.0f);              //cureResearch=Local research=100$=15
}



void virus_init(Virus *v)
{
    *v = (Virus){0};

    v->infectivity = 0.13f;         //controls infection spread
    v->severity = 0.005f;           //the base daily death rate among infected people
    v->recoveryRate = 0.030f;       //3% of the starting infected population recovers each game day
    v->mutationRate = 0.12f;        //on eligible mutation days there is 12% daily chance of mutation
    v->lastMutation = TRAIT_NONE;   //the virus hasn't acquired a mutation yet.
}



int virus_has_trait(const Virus *v, MutationTrait t)    //It checks whether a particular mutation is active using bitwise AND
{
    return (v->activeTraits & t) != 0;
}


const char *virus_trait_name(MutationTrait t)   //This simply converts the mutation enum into readable text
{                                             //mainly useful for displaying the mutation to the player
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

int virus_try_mutate(Virus *v, int day)         //It tries to give the virus a new mutation trait
{
    int elapsed = day - v->lastMutationDay;     //calculates how many days have passed since the last mutation

    if (elapsed < MUTATION_MIN_DAYS) return 0;  //If fewer than 20 days passed,no mutation is allowed[1-20]

    if (elapsed < MUTATION_MAX_DAYS) {          //If more than 20 but fewer than 30 days passed, mutation is possible [21-29]
    
        double roll = (double)rand() / ((double)RAND_MAX + 1.0);    //generates a random number roughly inside the range [0.0 and 1.0)

        if (roll >= v->mutationRate) return 0;    //filtering out the 88% failure case;indicating no mutation occurred.
    }

                                                                     // One randomly selected mutation bit; it may already be present.
    MutationTrait chosen = (MutationTrait)(1 << (rand() % 8));      //dividing by 8 because only 8 mutation types

                                // Bitwise OR adds the chosen flag without clearing previously acquired traits.
    v->activeTraits |= chosen;  //Saving the mutation
    v->lastMutation = chosen;   //N.B.code allows repeated mutations to strengthen the virus. Even if the trait already exists, strength and resistance still increase
    v->lastMutationDay = day;   //remembers what mutation happened and when.

    // Increase relative spread strength by 3%; resistance below rises by 5 percentage points
    v->infectivity *= 1.03f;
    v->resistance += 0.05f;

    //again some traits have additional effects
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

        default:            //If the value of chosen does not match any of the explicit case labels listed above
            break;
    }

    v->infectivity = clamp(v->infectivity, 0.0f, 0.35f);
    v->severity = clamp(v->severity, 0.0f, 0.02f);
    v->resistance = clamp(v->resistance, 0.0f, 0.80f);

    //prevents the virus stats from becoming unreasonably high.

    return 1;
}


float virus_hospital_load(const Region *r)  //This calculates whether hospitals have enough capacity
{
    // Available beds
    float beds = effective_healthcare(r) * HOSPITAL_BED_SHARE;
    // people requiring hospital beds.
    float demand = r->infected * HOSPITAL_CASE_SHARE;

    if (beds < 0.0001f) beds = 0.0001f;

    return demand / beds;   //load=deman/capacity
}
//Hospital load < 1 → capacity available
//Hospital load = 1 → exactly full
//Hospital load > 1 → overloaded

void virus_refresh_totals(GameState *gs)        //calculates global statistics from all 8 regions
{
    // Total original world population in millions.
    float population = 0.0f;
    //currently infected, in millions.
    float infected = 0.0f;
    // total deaths, in millions.
    float dead = 0.0f;
    //  successfully protected population, in millions.
    float vaccinated = 0.0f;

    // i: Zero-based index used to visit each region in this loop.
    for (int i = 0; i < MAX_REGIONS; i++) {
        // Pointer to the region currently being processed.
        const Region *r = &gs->regions[i];

        population += r->population;                //total globalpopulation
        infected += r->population * r->infected;    //global infected population
        dead += r->population * r->dead;            //global deaths
        vaccinated += r->population * r->vaccinated;//global total vaccinated
    }

    if (population <= 0.0f) return;

    gs->virus.globalInfected = infected / population;   //world infection percentage.
    gs->virus.globalDead = dead / population;           //world death percentage.

    // Surviving world population in millions; denominator for vaccine coverage.
    float living = population - dead;

    //vaccine distribution
    gs->cure.globalDistributed = living > 0.0f? clamp(vaccinated / living, 0.0f, 1.0f) : 0.0f;  //"Is the alive population greater than zero?" If yes, it performs the calculation.
                                                                                                // If no (meaning everyone has died), it safely bypasses the math and assigns 0.0f, 
                                                                                                //preventing a fatal division-by-zero error
}


void virus_update(GameState *gs)    //performs one full day's virus simulation
{                                   //runs once per game day, not every frame
    
    Virus *v = &gs->virus;
    
    float previous[MAX_REGIONS];    // Array storing each region infected fraction before any region is updated this day


    // i: Zero-based index used to visit each region in this loop
    for (int i = 0; i < MAX_REGIONS; i++)
        previous[i] = gs->regions[i].infected;

    //i: Zero-based index used to visit each region in this loop
    for (int i = 0; i < MAX_REGIONS; i++) {
        Region *r = &gs->regions[i];        // Pointer to the region currently being processed.

        /*1.Calculating healthy or susceptible people*/
        float healthy = clamp( 1.0f - previous[i] - r->dead - r->vaccinated, 0.0f, 1.0f);   
        float healthcare = effective_healthcare(r); //// Effective regional healthcare score after local research, limited to 0..1

        /*Border protection*/
        float border = r->bordersClosed ? 1.0f : r->borderControl;  // Effective border strength; forced to 1 when player closed the border.
        border = clamp(border, 0.0f, 1.0f);               //keeps it between 0 and 1
                                                          //recovery does not give permanent immunity in this model.

        /*2.Climate mutations*/
        float climate = 1.0f;      //If it were initialized to 0.0f instead,it would accidentally wipe out all infection spread entirely
        if (r->climate == CLIMATE_COLD && virus_has_trait(v, TRAIT_COLD_ADAPTED) )  climate = 1.15f;
        if (r->climate == CLIMATE_HOT && virus_has_trait(v, TRAIT_HOT_ADAPTED) )    climate = 1.15f;


        /*3.Imported infection*/
        float sourceCases = 0.0f;                          // Infected people in other open-border regions, in millions, using the daily snapshot.
        float sourcePopulation = 0.0f;                     //Original population of all other regions in millions, including closed-border regions.

                // j: Index of another region, used to calculate imported infection pressure.
        for (int j = 0; j < MAX_REGIONS; j++) {
            if (j == i) continue;
            const Region *source = &gs->regions[j];     // Read-only pointer to another region contributing to imported infection pressure.
            sourcePopulation += source->population;

            if (!source->bordersClosed)                //close bordered infected people doesnt count here
                sourceCases += previous[j] * source->population;
        }
        float imported = sourcePopulation > 0.0f? sourceCases / sourcePopulation: 0.0f;//the overall infection pressure from other regions

        /*4.Fast Spread mutation*/
    
        float mixing = GLOBAL_MIXING_RATE;          //Cross-region transmission factor; Fast Spread increases it.

        if (virus_has_trait(v, TRAIT_FAST_SPREAD))  mixing *= 1.5f;

        /* 5.Healthcare reduces spread*/

        float prevention = 1.0f - healthcare * 0.30f;

        if (virus_has_trait(v, TRAIT_STEALTH))   prevention = 1.0f - healthcare * 0.15f; 
                //healthcare now provides only half as much spread reduction.
                //So Stealth makes healthcare prevention less effective.

        /*6.*THE MAIN INFECTION FORMULA**/
        float exposure = v->infectivity * climate * prevention *(previous[i] * (1.0f - border * 0.5f)+ mixing * imported * (1.0f - border));
            //Exposure=VirusStrength×Climate×HealthcareEffect×(LocalSpread+ImportedSpread)

        /*7.Calculate new infections*/
            // Newly infected fraction of original region population, capped to susceptible people.
        float newCases = clamp(exposure * healthy, 0.0f, healthy);

        /*8.Calculate death rate*/
        // Daily fraction of infected people dying after healthcare and overload adjustments.
        float deathRate = v->severity * (1.0f - healthcare * 0.5f);
        //So healthcare lowers the effective death rate.


        /*9.Hospital overload doubles mortality*/
        if (virus_hospital_load(r) > 1.0f)  deathRate *= 2.0f;  // fixed formula

        /*10. Calculate deaths*/
        float deaths = clamp(previous[i] * deathRate,0.0f, previous[i]);    //fixed formula


        /*11. Calculate recoveries*/
        float recoveries = clamp(previous[i] * v->recoveryRate,0.0f, previous[i] - deaths); //fixed formula

        /*12. Update infection and deaths*/
        r->infected = previous[i] + newCases - deaths - recoveries;
        r->dead += deaths;

        /*12. Track hospital overload days*/
        if (virus_hospital_load(r) > 1.0f) r->overloadedDays++;
        else r->overloadedDays = 0;
    }
}
