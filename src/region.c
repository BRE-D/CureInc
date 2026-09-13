#include "region.h"

// .name-এর মতো নাম ধরে মান বসাই; না লেখা field নিজে থেকেই ০ হয়।
void region_init(GameState *gs) {
    gs->regions[0] = (Region){.name="The North", .population=.25f,
        .healthcareCapacity=.85f, .publicTrust=.80f, .borderControl=.65f, .climate=CLIMATE_COLD};
    gs->regions[1] = (Region){.name="Dorne", .population=.30f,
        .healthcareCapacity=.68f, .publicTrust=.52f, .borderControl=.48f, .climate=CLIMATE_HOT};
    gs->regions[2] = (Region){.name="Westeros", .population=.30f, .infected=.01f,
        .healthcareCapacity=.80f, .publicTrust=.75f, .borderControl=.65f, .climate=CLIMATE_TEMPERATE};
    gs->regions[3] = (Region){.name="The Vale", .population=.15f,
        .healthcareCapacity=.90f, .publicTrust=.85f, .borderControl=.85f, .climate=CLIMATE_TEMPERATE};
    gs->regions[4] = (Region){.name="Essos", .population=1.00f, .infected=.02f,
        .healthcareCapacity=.65f, .publicTrust=.55f, .borderControl=.45f, .climate=CLIMATE_HOT};
    gs->regions[5] = (Region){.name="The Iron Islands", .population=.10f,
        .healthcareCapacity=.60f, .publicTrust=.55f, .borderControl=.80f, .climate=CLIMATE_TEMPERATE};
    gs->regions[6] = (Region){.name="Beyond the Wall", .population=.01f,
        .healthcareCapacity=.90f, .publicTrust=.95f, .borderControl=.95f, .climate=CLIMATE_COLD};
    gs->regions[7] = (Region){.name="The Dothraki Sea", .population=.70f,
        .healthcareCapacity=.20f, .publicTrust=.28f, .borderControl=.10f, .climate=CLIMATE_HOT};
    region_update_states(gs); // শুরুর আক্রান্তের হার অনুযায়ী অবস্থাও ঠিক করি।
}

// আক্রান্তের পরিমাণ দেখে এলাকার অবস্থা ঠিক হয়; এটি মৃত্যুর হার নয়।
void region_update_states(GameState *gs) {
    for (int i = 0; i < MAX_REGIONS; i++) {
        float infected = gs->regions[i].infected;
        if (infected <= .000001f) gs->regions[i].state = REGION_CLEAN;
        else if (infected < .30f) gs->regions[i].state = REGION_INFECTED;
        else if (infected < .60f) gs->regions[i].state = REGION_CRITICAL;
        else gs->regions[i].state = REGION_DEVASTATED;
    }
}
