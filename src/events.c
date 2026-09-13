#include "events.h"
#include "raylib.h"

// Number of possible random event templates.
#define POOL_SIZE 14

// Array of 14 possible event templates. Entries contain static text and initially inactive timers.
static Event eventPool[POOL_SIZE] =
{
    {"Outbreak Reported", "Fresh clusters add a small number of infections across the regions.", 0, 0},
    {"Funding Surge",  "Emergency relief package approved -- research budget increased--", 0, 0},
    {"Mutation Watch", "Genome surveillance improves vaccine stability by 2%.", 0, 0},
    {"Public Panic",  "Panic reduces border control by 5 points in every region.", 0, 0},
    {"Border Lockdown", "Governments seal transit corridors to slow inter-region spread", 0, 0},
    {"Lab Breakthrough", "A promising compound has cleared preliminary safety screening--", 0, 0},
    {"Budget cuts", "Political deadlock freezes a quarter of the research allocation--", 0, 0},
    {"Volunteer Surge", "Volunteers add +0.05 base research points per day.", 0, 0},
    {"Supply Disruption", "Cold-chain failure delays vaccine shipments to eastern zones--", 0, 0},
    {"WHO Alert", "Global alert unlocks +$1 emergency funding per day.", 0, 0},
    {"Supply Chain Collapse", "Port closures destroy half of the vaccine stockpile", 0, 0},
    {"Political Infighting", "Member nations prioritize hoarding; global solidarity dissolves--", 0, 0},
    {"Medical Miracle", "Research gains 5 progress points and +0.10 base points/day.", 0, 0},
    {"Winter is coming", "Cold regions experience a small rise in infections.", 0, 0}
};


// Store an 8-second notification in the first free slot, or replace the active slot with least time
// remaining. Increase active count only for a free slot. Text pointers must remain valid; returns nothing.
// gs: shared game state; const means this function only reads it.
// title: read-only notification title text.
// description: read-only notification detail text.
void events_add(GameState *gs, const char *title, const char *description)
{
    // Notification slot selected for insertion or replacement.
    int slot = 0;
    // i: Zero-based index used to visit each item in this loop.
    for (int i = 0; i < MAX_EVENTS; i++) {
        if (!gs->eventLog[i].active) {
            slot = i;
            break;
        }
        if (gs->eventLog[i].timer < gs->eventLog[slot].timer) slot = i;
    }

    if (!gs->eventLog[slot].active) gs->eventCount++;
    gs->eventLog[slot] = (Event){title, description, 1, 8.0f};
}


// Clear active notifications and timers, reset their count, and forget the previous random event. Called
// by reset_game; returns nothing.
// gs: shared game state; const means this function only reads it.
void events_init(GameState *gs)
{

    // i: Zero-based index used to visit each item in this loop.
    for (int i = 0; i < MAX_EVENTS; i++)
    {
        gs->eventLog[i].active = 0;
        gs->eventLog[i].timer = 0;
    }
    gs->eventCount = 0;
    gs->lastEventIndex = -1;
}


// Choose one of 14 events, try up to five times to avoid an immediate repeat, then log and apply it.
// Research bonuses stop after Trials. Called every seven game days; returns nothing.
// gs: shared game state; const means this function only reads it.
void events_trigger_random(GameState *gs)
{
    // Index of the random event selected from eventPool.
    int pick;
    // Number of random draws so far; retries stop at five.
    int attempts = 0;

    do {
        pick = GetRandomValue(0, POOL_SIZE - 1);
        attempts++;
    } while (pick == gs->lastEventIndex && attempts < 5);

    gs->lastEventIndex = pick;


    if (gs->cure.phase >= PHASE_PRODUCTION && (pick == 5 || pick == 7 || pick == 12)) {
        events_add(gs, eventPool[pick].title, "Research is complete; no research bonus applied.");
        return;
    }
    events_add(gs, eventPool[pick].title, eventPool[pick].description);

    switch (pick)
    {
        // Outbreak: add up to 0.5 percentage points of infection in every region.
        case 0:
            // r: region index (0..7) for applying this event to each region.
            for (int r = 0; r < MAX_REGIONS; r++) {
                // Susceptible fraction: original population minus infected, dead, and protected fractions.
                float healthy = 1.0f - gs->regions[r].infected
                              - gs->regions[r].dead
                              - gs->regions[r].vaccinated;
                if (healthy < 0.0f) healthy = 0.0f;

                // Extra infected fraction from this outbreak, capped by available susceptible people.
                float added = healthy < 0.005f ? healthy : 0.005f;
                gs->regions[r].infected += added;
            }
            break;

        // Funding Surge: add $2 to recurring daily income.
        case 1:
            gs->cure.fundingPerTick += 2.0f;
            break;

        // Breakthrough: share the 5-point research bonus with Medical Miracle.
        case 5:
        // Medical Miracle also adds 0.10 base research points/day.
        case 12:
            gs->cure.researchProgress += 5.0f;
            if (gs->cure.researchProgress > 100) gs->cure.researchProgress = 100;
            if (pick == 12) gs->cure.rpPerTick += 0.10f;
            break;

        // Budget cuts: subtract $1.50/day, keeping income at least $0.50/day.
        case 6:
            gs->cure.fundingPerTick -= 1.5f;
            if (gs->cure.fundingPerTick < 0.5f) gs->cure.fundingPerTick = 0.5f;
            break;

        // Volunteers: add 0.05 base research points/day.
        case 7:
            gs->cure.rpPerTick += 0.05f;
            break;

        // Supply disruption: destroy 10% of stored vaccine units.
        case 8:
            gs->cure.vaccineStockpile *= 0.90f;
            break;

        // Supply collapse: halve vaccine stock and subtract $2/day, with a $0.50/day minimum.
        case 10:
            gs->cure.vaccineStockpile *= 0.50f;

            gs->cure.fundingPerTick -= 2.0f;

            if (gs->cure.fundingPerTick < 0.5f)
                gs->cure.fundingPerTick = 0.5f;

            break;

        // Political infighting: remove up to $50 from current funding.
        case 11:
            gs->cure.funding -= 50.0f;
            if (gs->cure.funding < 0.0f) gs->cure.funding = 0.0f;
            break;

        // Mutation Watch: restore 2 stability percentage points, capped at 100%.
        case 2:
            gs->cure.stability += 0.02f;
            if (gs->cure.stability > 1.0f) gs->cure.stability = 1.0f;
            break;

        // WHO Alert: add $1 to recurring daily income.
        case 9:
            gs->cure.fundingPerTick += 1.0f;
            break;

        // Winter: add up to 1 percentage point of infection only in cold regions.
        case 13:
            // r: region index (0..7) for applying this event to each region.
            for (int r = 0; r < MAX_REGIONS; r++) {
                if (gs->regions[r].climate != CLIMATE_COLD) continue;

                // Susceptible fraction: original population minus infected, dead, and protected fractions.
                float healthy = 1.0f - gs->regions[r].infected
                              - gs->regions[r].dead
                              - gs->regions[r].vaccinated;
                if (healthy < 0.0f) healthy = 0.0f;

                // Extra infected fraction from this outbreak, capped by available susceptible people.
                float added = healthy < 0.01f ? healthy : 0.01f;
                gs->regions[r].infected += added;
            }
            break;

        // Panic: reduce every region baseline border control by 5 points; this persists.
        case 3:
            // r: region index (0..7) for applying this event to each region.
            for (int r = 0; r < MAX_REGIONS; r++) {
                gs->regions[r].borderControl -= 0.05f;
                if (gs->regions[r].borderControl < 0) gs->regions[r].borderControl = 0;
            }
            break;

        // Lockdown: raise every region baseline border control by 10 points, capped at 100%.
        case 4:
            // r: region index (0..7) for applying this event to each region.
            for (int r = 0; r < MAX_REGIONS; r++)
            {
                gs->regions[r].borderControl += 0.10f;
                if (gs->regions[r].borderControl > 1.0f) gs->regions[r].borderControl = 1.0f;
            }
            break;

        default:

            break;
    }
}


// Subtract real frame time from active notification timers and hide expired entries. Called only during
// gameplay, so pausing also freezes notices; returns nothing.
// gs: shared game state; const means this function only reads it.
// delta: elapsed real seconds since the previous frame.
void events_update(GameState *gs, float delta)
{
    // i: Zero-based index used to visit each item in this loop.
    for (int i = 0; i < MAX_EVENTS; i++)
    {
        if (gs->eventLog[i].active)
        {
            gs->eventLog[i].timer -= delta;

            if (gs->eventLog[i].timer <= 0)
            {
                gs->eventLog[i].active = 0;
                gs->eventCount--;
            }
        }
    }
}
