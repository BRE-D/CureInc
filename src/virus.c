#include "virus.h"
#include "events.h"
#include "raylib.h"
#include <string.h>

/*
 * get_climate_factor - Returns a multiplier for virus spread in a region
 *                      based on climate type and mutation adaptation traits.
 */
static float get_climate_factor(const Region *r, int active_traits)
{
    // Cold regions: The North, Beyond the Wall, The Vale
    if (strcmp(r->name, "The North") == 0 ||
        strcmp(r->name, "Beyond the Wall") == 0 ||
        strcmp(r->name, "The Vale") == 0)
    {
        if (active_traits & TRAIT_COLD_ADAPTED) {
            return 1.5f;
        } else {
            // Beyond the wall is extremely cold and isolated
            return (strcmp(r->name, "Beyond the Wall") == 0) ? 0.1f : 0.6f;
        }
    }
    // Hot regions: Dorne, The Dothraki Sea
    else if (strcmp(r->name, "Dorne") == 0 ||
             strcmp(r->name, "The Dothraki Sea") == 0)
    {
        if (active_traits & TRAIT_HOT_ADAPTED) {
            return 1.5f;
        } else {
            return 0.7f;
        }
    }

    // Default temperate regions
    return 1.0f;
}

/*
 * virus_update - Executes the daily simulation step for the pathogen.
 */
void virus_update(GameState *gs)
{
    if (!gs) return;

    // 1. Gather global weights and sum populations
    float total_pop = 0.0f;
    for (int i = 0; i < MAX_REGIONS; i++) {
        total_pop += gs->regions[i].population;
    }
    if (total_pop <= 0.0f) total_pop = 1.0f;

    // 2. Daily Local Growth, Healthcare Degradation, and Local Mortality
    float daily_total_deaths = 0.0f;

    for (int i = 0; i < MAX_REGIONS; i++) {
        Region *r = &gs->regions[i];
        
        if (r->infected > 0.0f) {
            // Local growth calculation
            float climate_factor = get_climate_factor(r, gs->virus.activeTraits);
            float growth_rate = gs->virus.infectivity * climate_factor;

            if (gs->virus.activeTraits & TRAIT_AIRBORNE) {
                growth_rate *= 1.4f;
            }

            // Logistic growth model: growth is limited by remaining uninfected and unvaccinated population
            float growth = r->infected * (1.0f - r->infected - r->vaccinated) * growth_rate;
            if (growth < 0.0f) growth = 0.0f;

            r->infected += growth;
            if (r->infected > 1.0f - r->vaccinated) {
                r->infected = 1.0f - r->vaccinated;
            }
            if (r->infected < 0.0f) {
                r->infected = 0.0f;
            }

            // Healthcare degradation
            float degradation = r->infected * gs->virus.severity * 0.1f;
            r->healthcareCapacity -= degradation;
            if (r->healthcareCapacity < 0.0f) {
                r->healthcareCapacity = 0.0f;
            }

            // Mortality rate calculation
            float base_death_rate = gs->virus.severity * 0.02f;
            if (gs->virus.activeTraits & TRAIT_LETHAL) {
                base_death_rate *= 2.0f;
            }

            // Healthcare capacity dampens or elevates death rates
            float healthcare_multiplier = 1.0f + (1.0f - r->healthcareCapacity) * 3.0f;
            float local_deaths = r->infected * base_death_rate * healthcare_multiplier;

            // Accumulate weighted deaths contribution
            daily_total_deaths += (local_deaths * r->population);

            // Deduct deaths from active infected pool
            r->infected -= local_deaths;
            if (r->infected < 0.0f) {
                r->infected = 0.0f;
            }
        } else {
            // Gradual healthcare recovery when there is no active infection
            r->healthcareCapacity += 0.02f;
            if (r->healthcareCapacity > 1.0f) {
                r->healthcareCapacity = 1.0f;
            }
        }
    }

    // Accumulate to global dead count
    gs->virus.globalDead += (daily_total_deaths / total_pop);
    if (gs->virus.globalDead > 1.0f) {
        gs->virus.globalDead = 1.0f;
    }

    // 3. Inter-Region Transmission (Border Crossing Spread)
    float base_transmission = 0.015f;
    if (gs->virus.activeTraits & TRAIT_FAST_SPREAD) {
        base_transmission *= 2.0f;
    }
    if (gs->virus.activeTraits & TRAIT_AIRBORNE) {
        base_transmission *= 1.5f;
    }

    // Double buffer for synchronous/order-independent transmission
    float temp_infected[MAX_REGIONS];
    for (int k = 0; k < MAX_REGIONS; k++) {
        temp_infected[k] = gs->regions[k].infected;
    }

    for (int i = 0; i < MAX_REGIONS; i++) {
        if (gs->regions[i].infected > 0.01f) {
            for (int j = 0; j < MAX_REGIONS; j++) {
                if (i == j) continue;
                
                const Region *orig_target = &gs->regions[j];
                float target_avail = 1.0f - orig_target->infected - orig_target->vaccinated;
                
                if (target_avail > 0.01f) {
                    // Spread is inhibited by border control of target region, boosted by source population size
                    float spread_chance = gs->regions[i].infected * base_transmission * 
                                         (1.0f - orig_target->borderControl) * gs->regions[i].population;

                    if (temp_infected[j] == 0.0f) {
                        // Seed infection if crossing criteria met (threshold lowered to 0.0001f to prevent lockout)
                        if (spread_chance > 0.0001f) {
                            temp_infected[j] = 0.005f;
                        }
                    } else {
                        temp_infected[j] += spread_chance;
                    }

                    if (temp_infected[j] > 1.0f - orig_target->vaccinated) {
                        temp_infected[j] = 1.0f - orig_target->vaccinated;
                    }
                }
            }
        }
    }

    // Write temp buffer back to regions
    for (int k = 0; k < MAX_REGIONS; k++) {
        gs->regions[k].infected = temp_infected[k];
    }

    // 4. Update Global Infected Stats
    float total_infected_pop = 0.0f;
    for (int i = 0; i < MAX_REGIONS; i++) {
        total_infected_pop += (gs->regions[i].infected * gs->regions[i].population);
    }
    gs->virus.globalInfected = total_infected_pop / total_pop;
    if (gs->virus.globalInfected > 1.0f) {
        gs->virus.globalInfected = 1.0f;
    }

    // 5. Daily Mutation Check
    float mutation_roll = (float)GetRandomValue(0, 10000) / 10000.0f;
    if (mutation_roll < gs->virus.mutationRate) {
        // Collect unacquired traits
        MutationTrait all_traits[] = {
            TRAIT_AIRBORNE,
            TRAIT_DRUG_RESISTANT,
            TRAIT_STEALTH,
            TRAIT_LETHAL,
            TRAIT_FAST_SPREAD,
            TRAIT_COLD_ADAPTED,
            TRAIT_HOT_ADAPTED,
            TRAIT_LONG_INCUBATION
        };
        int num_traits = 8;
        MutationTrait unacquired_traits[8];
        int unacquired_count = 0;

        for (int k = 0; k < num_traits; k++) {
            if (!(gs->virus.activeTraits & all_traits[k])) {
                unacquired_traits[unacquired_count++] = all_traits[k];
            }
        }

        if (unacquired_count > 0) {
            int idx = GetRandomValue(0, unacquired_count - 1);
            MutationTrait selected = unacquired_traits[idx];

            gs->virus.activeTraits |= selected;

            // Apply modifiers and queue event notifications
            const char *title = "Mutation";
            const char *desc = "The virus has mutated.";

            switch (selected) {
                case TRAIT_AIRBORNE:
                    gs->virus.infectivity += 0.02f;
                    title = "Airborne Transmission";
                    desc = "The pathogen can now spread through the air, increasing infectivity.";
                    break;
                case TRAIT_DRUG_RESISTANT:
                    gs->virus.resistance += 0.15f;
                    if (gs->virus.resistance > 1.0f) gs->virus.resistance = 1.0f;
                    title = "Drug Resistance";
                    desc = "The pathogen shows increased resistance to vaccine research.";
                    break;
                case TRAIT_STEALTH:
                    gs->virus.mutationRate += 0.02f;
                    title = "Stealth Adaptation";
                    desc = "The virus replicates with minimal signs, raising mutation probability.";
                    break;
                case TRAIT_LETHAL:
                    gs->virus.severity += 0.03f;
                    title = "Lethal Mutation";
                    desc = "Pathogen severity has increased, significantly rising the mortality rate.";
                    break;
                case TRAIT_FAST_SPREAD:
                    // Handled in transmission logic
                    title = "Fast Spread";
                    desc = "Incubators spread faster, increasing transmission between region borders.";
                    break;
                case TRAIT_COLD_ADAPTED:
                    // Handled in climate logic
                    title = "Cold Adaptation";
                    desc = "The virus survives cold climates better, boosting spread in Northern regions.";
                    break;
                case TRAIT_HOT_ADAPTED:
                    // Handled in climate logic
                    title = "Heat Adaptation";
                    desc = "The virus survives heat climates better, boosting spread in tropical regions.";
                    break;
                case TRAIT_LONG_INCUBATION:
                    gs->virus.mutationRate += 0.03f;
                    title = "Long Incubation";
                    desc = "A longer incubation cycle allows more mutation trials per day.";
                    break;
                default:
                    break;
            }

            event_log_add(gs, title, desc);
        }
    }
}
