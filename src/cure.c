#include "cure.h"
#include "events.h"

void cure_update(GameState *gs) {
  if (!gs)
    return;

  // 1. Daily Funding & RP updates
  gs->cure.funding += gs->cure.fundingPerTick;
  gs->cure.researchPoints += gs->cure.rpPerTick;

  // 2. Cure Research Progress & Phase Transitions
  if (gs->cure.phase < PHASE_DISTRIBUTION) {
    float research_speed_mod = 0.0f;
    for (int i = 0; i < gs->skillCount; i++) {
      if (gs->skills[i].unlocked) {
        research_speed_mod += gs->skills[i].researchMod;
      }
    }

    // Base progress rate is 2.5% per day
    float base_speed = 2.5f;
    float modifier = 1.0f + research_speed_mod;

    // Virus resistance dampens research speed
    float resistance_penalty = 1.0f - gs->virus.resistance;
    if (resistance_penalty < 0.1f)
      resistance_penalty = 0.1f; // minimum 10% research speed

    float daily_progress = base_speed * modifier * resistance_penalty;
    gs->cure.researchProgress += daily_progress;

    if (gs->cure.researchProgress >= 100.0f) {
      gs->cure.researchProgress = 0.0f;
      gs->cure.phase = (ResearchPhase)(gs->cure.phase + 1);

      const char *title = "Cure Milestone";
      const char *desc = "The cure research enters the next phase.";

      if (gs->cure.phase == PHASE_TRIALS) {
        title = "Trials Commenced";
        desc = "Clinical trials are underway testing vaccine safety.";
      } else if (gs->cure.phase == PHASE_PRODUCTION) {
        title = "Mass Production";
        desc = "Vaccine manufacturing plants are actively producing doses.";
        gs->cure.productionRate =
            0.05f; // 5% global population coverage per day
      } else if (gs->cure.phase == PHASE_DISTRIBUTION) {
        title = "Vaccine Rollout";
        desc = "Global distribution has begun! Citizens are being vaccinated.";
        gs->cure.effectiveness = 1.0f - gs->virus.resistance;
        if (gs->cure.effectiveness < 0.1f)
          gs->cure.effectiveness = 0.1f;
      }

      event_log_add(gs, title, desc);
    }
  }

  // 3. Vaccine Distribution (PHASE_DISTRIBUTION)
  if (gs->cure.phase == PHASE_DISTRIBUTION) {
    float dist_mod = 0.0f;
    for (int i = 0; i < gs->skillCount; i++) {
      if (gs->skills[i].unlocked) {
        dist_mod += gs->skills[i].distributionMod;
      }
    }

    // Base vaccine doses available per day (fraction of global population
    // covered)
    float total_daily_doses = gs->cure.productionRate * (1.0f + dist_mod);

    // Sum total population
    float total_pop = 0.0f;
    for (int i = 0; i < MAX_REGIONS; i++) {
      total_pop += gs->regions[i].population;
    }
    if (total_pop <= 0.0f)
      total_pop = 1.0f;

    // Distribute doses to regions proportionally to population size
    for (int i = 0; i < MAX_REGIONS; i++) {
      Region *r = &gs->regions[i];

      // Dose allocated to this region
      float regional_doses = total_daily_doses * (r->population / total_pop);

      // Vaccination rate depends on trust and effectiveness
      float vaccination_rate =
          regional_doses * r->publicTrust * gs->cure.effectiveness;

      r->vaccinated += vaccination_rate;
      if (r->vaccinated > 1.0f) {
        r->vaccinated = 1.0f;
      }

      // Vaccine squeezes out the virus
      if (r->infected > 1.0f - r->vaccinated) {
        r->infected = 1.0f - r->vaccinated;
      }
    }

    // Recalculate global distributed stats
    float total_vaccinated_pop = 0.0f;
    for (int i = 0; i < MAX_REGIONS; i++) {
      total_vaccinated_pop +=
          (gs->regions[i].vaccinated * gs->regions[i].population);
    }
    gs->cure.globalDistributed = total_vaccinated_pop / total_pop;
    if (gs->cure.globalDistributed > 1.0f) {
      gs->cure.globalDistributed = 1.0f;
    }
  }
}
