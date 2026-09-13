#include "region.h"


// Set the eight starting regions. Named initializers specify chosen fields; omitted fields become zero.
// Seed infections in Westeros and Essos, then update region states; returns nothing.
// gs: shared game state; const means this function only reads it.
void region_init(GameState *gs) {
    gs->regions[0] = (Region){.name="The North", .population=.25f,
        .healthcareCapacity=.85f, .borderControl=.65f, .climate=CLIMATE_COLD};
    gs->regions[1] = (Region){.name="Dorne", .population=.30f,
        .healthcareCapacity=.68f, .borderControl=.48f, .climate=CLIMATE_HOT};
    gs->regions[2] = (Region){.name="Westeros", .population=.30f, .infected=.01f,
        .healthcareCapacity=.80f, .borderControl=.65f, .climate=CLIMATE_TEMPERATE};
    gs->regions[3] = (Region){.name="The Vale", .population=.15f,
        .healthcareCapacity=.90f, .borderControl=.85f, .climate=CLIMATE_TEMPERATE};
    gs->regions[4] = (Region){.name="Essos", .population=1.00f, .infected=.02f,
        .healthcareCapacity=.65f, .borderControl=.45f, .climate=CLIMATE_HOT};
    gs->regions[5] = (Region){.name="The Iron Islands", .population=.10f,
        .healthcareCapacity=.60f, .borderControl=.80f, .climate=CLIMATE_TEMPERATE};
    gs->regions[6] = (Region){.name="Beyond the Wall", .population=.01f,
        .healthcareCapacity=.90f, .borderControl=.95f, .climate=CLIMATE_COLD};
    gs->regions[7] = (Region){.name="The Dothraki Sea", .population=.70f,
        .healthcareCapacity=.20f, .borderControl=.10f, .climate=CLIMATE_HOT};
    region_update_states(gs);
}


// Set each region state from its infected fraction: clean up to 0.000001, infected below 0.30, critical
// below 0.60, otherwise devastated. UI card colors use separate thresholds; returns nothing.
// gs: shared game state; const means this function only reads it.
void region_update_states(GameState *gs) {
    // i: Zero-based index used to visit each item in this loop.
    for (int i = 0; i < MAX_REGIONS; i++) {
        // Currently infected fraction of original regional population (0..1).
        float infected = gs->regions[i].infected;
        if (infected <= .000001f) gs->regions[i].state = REGION_CLEAN;
        else if (infected < .30f) gs->regions[i].state = REGION_INFECTED;
        else if (infected < .60f) gs->regions[i].state = REGION_CRITICAL;
        else gs->regions[i].state = REGION_DEVASTATED;
    }
}
