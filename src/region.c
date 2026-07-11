#include "region.h"

/*
 * region_init - Fills all 8 region slots in gs->regions[] with
 *               starting values for the simulation.
 *
 * Regions are named after Game of Thrones locations and mapped to
 * the 7 real-world continents plus Nescafe Island (fictional).
 *
 * The Seven Kingdoms (Europe) and Essos (Asia) begin with a small
 * infection seed. All other regions start clean.
 *
 * Parameters:
 *   gs - pointer to the active GameState; must not be NULL.
 *
 * Side effects:
 *   Writes to gs->regions[0..MAX_REGIONS-1].
 */
void region_init(GameState *gs) {
/* Shorthand to avoid repeating gs->regions[i] on every line */
#define R(i) gs->regions[i]

  /* --- The North (Scotland, northern England, and parts of Scandinavia (cold
   * climate, clans, castles).) --- */
  R(0).name = "The North";
  R(0).population = 0.25f; /* sparse, harsh cold terrain      */
  R(0).infected = 0.00f;
  R(0).vaccinated = 0.00f;
  R(0).healthcareCapacity = 0.85f; /* Scandinavia = top-tier care     */
  R(0).publicTrust = 0.80f;        /* Nordic nations trust highly     */
  R(0).borderControl = 0.65f;      /* strong Nordic border governance */
  R(0).state = REGION_CLEAN;

  /* --- Dorne (Spain ;especially Andalusia and Moorish Iberia, with some Middle
   * Eastern influence.) --- */
  R(1).name = "Dorne";
  R(1).population = 0.30f;
  R(1).infected = 0.00f;
  R(1).vaccinated = 0.00f;
  R(1).healthcareCapacity = 0.68f; /* Spain ranks among Europe's best */
  R(1).publicTrust = 0.52f;        /* moderate, varies by region      */
  R(1).borderControl = 0.48f;      /* EU + Mediterranean crossings    */
  R(1).state = REGION_CLEAN;

  /* Westeros (Primarily Britain (Great Britain), medieval Western Europe ) */
  R(2).name = "Westeros";
  R(2).population = 0.30f;
  R(2).infected = 0.01f; /* simulation starts here          */
  R(2).vaccinated = 0.00f;
  R(2).healthcareCapacity = 0.80f;
  R(2).publicTrust = 0.75f;
  R(2).borderControl = 0.65f;
  R(2).state = REGION_INFECTED;

  /* --- The Vale (Alpine regions such as Switzerland and Austria (mountains and
   * isolated valleys).) --- */
  R(3).name = "The Vale";
  R(3).population =
      0.15f; /* small Alpine nations, mountains limit settlement */
  R(3).infected = 0.00f;
  R(3).vaccinated = 0.00f;
  R(3).healthcareCapacity =
      0.90f; /* Switzerland = world's best healthcare system    */
  R(3).publicTrust = 0.85f; /* Swiss have extremely high institutional trust */
  R(3).borderControl = 0.85f; /* mountains = near-impregnable natural barrier */
  R(3).state = REGION_CLEAN;

  /* --- Essos (Asia — secondary outbreak, largest pop) --- */
  R(4).name = "Essos";
  R(4).population = 1.00f; /* baseline: most populous         */
  R(4).infected = 0.02f;   /* dense cities accelerate spread  */
  R(4).vaccinated = 0.00f;
  R(4).healthcareCapacity = 0.65f;
  R(4).publicTrust = 0.55f;
  R(4).borderControl = 0.45f;
  R(4).state = REGION_INFECTED;

  /* --- The Iron Islands (Australia/Oceania — isolated.Vikings from Scandinavia
   * and the Norse-Gaels of the British Isles.) --- */
  R(5).name = "The Iron Islands";
  R(5).population = 0.10f; /* small, sea-faring communities   */
  R(5).infected = 0.00f;
  R(5).vaccinated = 0.00f;
  R(5).healthcareCapacity = 0.60f; /* rugged, war-culture, basic care */
  R(5).publicTrust = 0.55f;        /* clan loyalty, distrust outsiders*/
  R(5).borderControl = 0.80f;      /* sea = natural moat, hard to breach */
  R(5).state = REGION_CLEAN;

  /* --- Beyond the Wall (Antarctica — research stations only) --- */
  R(6).name = "Beyond the Wall";
  R(6).population = 0.01f; /* Night's Watch rangers only      */
  R(6).infected = 0.00f;
  R(6).vaccinated = 0.00f;
  R(6).healthcareCapacity = 0.90f; /* controlled, isolated camps      */
  R(6).publicTrust = 0.95f;
  R(6).borderControl = 0.95f; /* The Wall is a very good border  */
  R(6).state = REGION_CLEAN;

  /* --- The Dothraki Sea (Eurasian Steppe (Mongols, Huns, Turkic nomads).) ---
   */
  R(7).name = "The Dothraki Sea ";
  R(7).population = 0.70f; /* vast steppe, nomadic spread     */
  R(7).infected = 0.00f;
  R(7).vaccinated = 0.00f;
  R(7).healthcareCapacity = 0.20f; /* nomadic tribes, no hospitals    */
  R(7).publicTrust = 0.28f;        /* tribal loyalty, not institutional */
  R(7).borderControl = 0.10f;      /* open steppe = essentially no border */
  R(7).state = REGION_CLEAN;

#undef R
}

/*
 * region_update_states - Reclassifies each region's RegionState
 *                        based on its current infected fraction.
 *
 * Thresholds:
 *   < 0.05  -> REGION_CLEAN
 *   < 0.30  -> REGION_INFECTED
 *   < 0.60  -> REGION_CRITICAL
 *   >= 0.60 -> REGION_DEVASTATED
 *
 * Parameters:
 *   gs - pointer to the active GameState; must not be NULL.
 *
 * Side effects:
 *   Updates gs->regions[i].state for all i in [0, MAX_REGIONS).
 */
void region_update_states(GameState *gs) {
  for (int i = 0; i < MAX_REGIONS; i++) {
    float inf = gs->regions[i].infected;

    if (inf < 0.05f)
      gs->regions[i].state = REGION_CLEAN;
    else if (inf < 0.30f)
      gs->regions[i].state = REGION_INFECTED;
    else if (inf < 0.60f)
      gs->regions[i].state = REGION_CRITICAL;
    else
      gs->regions[i].state = REGION_DEVASTATED;
  }
}
