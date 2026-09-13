#include "events.h"
#include "raylib.h"

#define POOL_SIZE 14

static Event eventPool[POOL_SIZE] =
{
    {"Outbreak Reported", "A cluster of new cases has emerged--", 0, 0},
    {"Funding Surge",  "Emergency relief package approved -- research budget increased--", 0, 0},
    {"Mutation Watch", "Labs are monitoring the pathogen for new changes.", 0, 0},
    {"Public Panic",  "Panic reduces border control by 5 points in every region.", 0, 0},
    {"Border Lockdown", "Governments seal transit corridors to slow inter-region spread", 0, 0},
    {"Lab Breakthrough", "A promising compound has cleared preliminary safety screening--", 0, 0},
    {"Budget cuts", "Political deadlock freezes a quarter of the research allocation--", 0, 0},
    {"Volunteer Surge", "Volunteers add +0.05 base research points per day.", 0, 0},
    {"Supply Disruption", "Cold-chain failure delays vaccine shipments to eastern zones--", 0, 0},
    {"WHO Alert", "Global health authority raises threat level to High--", 0, 0},
    {"Supply Chain Collapse", "Port closures destroy half of the vaccine stockpile", 0, 0},
    {"Political Infighting", "Member nations prioritize hoarding; global solidarity dissolves--", 0, 0},
    {"Medical Miracle", "Research gains 5 progress points and +0.10 base points/day.", 0, 0},
    {"Winter is coming", "Hospitals are preparing for colder weather.", 0, 0}
};

// খালি জায়গায় খবর রাখি; সব ভরা হলে সবচেয়ে আগে শেষ হবে এমন খবরটি সরাই।
void events_add(GameState *gs, const char *title, const char *description)
{
    int slot = 0;
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

// নতুন খেলার জন্য পুরোনো খবর ও আগের event মুছে দিই।
void events_init(GameState *gs)
{

    for (int i = 0; i < MAX_EVENTS; i++)
    {
        gs->eventLog[i].active = 0;
        gs->eventLog[i].timer = 0;
    }
    gs->eventCount = 0;
    gs->lastEventIndex = -1;
}

// একটি random event বেছে তার নির্দিষ্ট প্রভাব প্রয়োগ করি।
void events_trigger_random(GameState *gs)
{
    int pick;
    int attempts = 0;

    do {
        pick = GetRandomValue(0, POOL_SIZE - 1);
        attempts++;
    } while (pick == gs->lastEventIndex && attempts < 5); // একই খবর এড়াতে সর্বোচ্চ ৫ চেষ্টা।

    gs->lastEventIndex = pick;

    // Research শেষ হলে তার bonus বা progress আর বদলাব না।
    if (gs->cure.phase >= PHASE_PRODUCTION && (pick == 5 || pick == 7 || pick == 12)) {
        events_add(gs, eventPool[pick].title, "Research is complete; no research bonus applied.");
        return;
    }
    events_add(gs, eventPool[pick].title, eventPool[pick].description);

    switch (pick)
    {
        case 1: // Funding Surge: প্রতিদিনের আয় বাড়ে।
            gs->cure.fundingPerTick += 2.0f;
            break;

        case 5:  // Lab Breakthrough ও Medical Miracle-এর একই progress bonus।
        case 12:
            gs->cure.researchProgress += 5.0f;
            if (gs->cure.researchProgress > 100) gs->cure.researchProgress = 100;
            if (pick == 12) gs->cure.rpPerTick += 0.10f;
            break;

        case 6: // Budget cuts: আয় কমে, কিন্তু নির্ধারিত সর্বনিম্নের নিচে নয়।
            gs->cure.fundingPerTick -= 1.5f;
            if (gs->cure.fundingPerTick < 0.5f) gs->cure.fundingPerTick = 0.5f;
            break;

        case 7: // Volunteer: মূল research/day অল্প বাড়ে।
            gs->cure.rpPerTick += 0.05f;
            break;

        case 8: // মজুত vaccine-এর ১০% নষ্ট হয়।
            gs->cure.vaccineStockpile *= 0.90f;
            break;

        case 10: // অর্ধেক মজুত নষ্ট এবং আয় কমে।
            gs->cure.vaccineStockpile *= 0.50f;

            gs->cure.fundingPerTick -= 2.0f;

            if (gs->cure.fundingPerTick < 0.5f)
                gs->cure.fundingPerTick = 0.5f;

            break;

        case 11: // বর্তমান টাকা থেকে ৫০ কাটা হয়।
            gs->cure.funding -= 50.0f;
            if (gs->cure.funding < 0.0f) gs->cure.funding = 0.0f;
            break;

        case 2:
        case 13:
            break;

        case 3: // Panic-এ সীমান্ত নিয়ন্ত্রণ কমে; এটি সময় শেষে নিজে থেকে ফেরে না।
            for (int r = 0; r < MAX_REGIONS; r++) {
                gs->regions[r].borderControl -= 0.05f;
                if (gs->regions[r].borderControl < 0) gs->regions[r].borderControl = 0;
            }
            break;

        case 4: // সব অঞ্চলে সীমান্ত নিয়ন্ত্রণ কিছুটা বাড়ে।
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

// খবরের সময় কমাই; সময় শেষ হলে খবর লুকাই।
void events_update(GameState *gs, float delta)
{
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
