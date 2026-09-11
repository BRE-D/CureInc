#include "events.h"
#include "raylib.h"

//events that can happen
#define POOL_SIZE 14

static Event eventPool[POOL_SIZE] = 
{
    {
        "Outbreak Reported", "A cluster of new cases has emerged--", 0, 0 
    },
    {
        "Funding Surge",  "Emergency relief package approved -- research budget increased--", 0, 0 
    },
    {
        "Mutation Detected", "Analysts warn the pathogen has developed a new protein marker--", 0, 0 
    },
    {
        "Public Panic",  "Social media fuels mass hysteria, clinic queues double overnight--", 0, 0   
    },
    {
        "Border Lockdown", "Governments seal transit corridors to slow inter-region spread", 0, 0 
    },
    {
        "Lab Breakthrough", "A promising compound has cleared preliminary safety screening--", 0, 0 
    },
    {
        "Budget cuts", "Political deadlock freezes a quarter of the research allocation--", 0, 0 
    },
    {
        "Volunteer Surge", "Thousands sign up for lcinical trials following a news segment--", 0, 0 
    },
    {
        "Supply Disruption", "Cold-chain failure delays vaccine shipments to eastern zones--", 0, 0 
    },
    {
        "WHO Alert", "Global health authority raises threat level to High--", 0, 0 
    },
    {
        "Supply Chain Collapse", "Major ports shut down; vaccine distribution halts indefinitely--", 0, 0 
    },
    {
        "Political Infighting", "Member nations prioritize hoarding; global solidarity dissolves--", 0, 0
    },
    {
        "Medical Miracle", "AI-driven drug discovery accelerates trial timelines by 30%--", 0, 0 
    },
    {
        "Winter is coming", "A resistant, cold strain has quietly spread north for weeks--", 0, 0 
    }
};

void events_init(GameState *gs)
{
    //clear log before the game
    for (int i = 0; i < MAX_EVENTS; i++)
    {
        gs->eventLog[i].active = 0;
        gs->eventLog[i].timer = 0;
    }
    gs->eventCount = 0;
    gs->lastEventIndex = -1; /* No previous event */
}

void events_trigger_random(GameState *gs)
{
    int pick;
    int attempts = 0;
    
    /* Try to pick a different event than the last one */
    do {
        pick = GetRandomValue(0, POOL_SIZE - 1);
        attempts++;
    } while (pick == gs->lastEventIndex && attempts < 5);
    
    gs->lastEventIndex = pick; /* Remember this event */
    
    //find free slot and put event
    for (int i = 0; i < MAX_EVENTS; i++)
    {
        if(gs->eventLog[i].active == 0)
        {
            gs->eventLog[i].title = eventPool[pick].title;
            gs->eventLog[i].description = eventPool[pick].description;
            gs->eventLog[i].active = 1;
            gs->eventLog[i].timer = 8.0f;
            gs->eventCount++;
            break;
        }
    }

    /* Apply mechanical effects based on event type */
    switch (pick)
    {
        case 1: /* "Funding Surge" */
            gs->cure.fundingPerTick += 2.0f;
            break;

        case 5: /* "Lab Breakthrough" */
            gs->cure.researchProgress += 15.0f;
            break;

        case 6: /* "Budget cuts" */
            gs->cure.fundingPerTick -= 1.5f;
            if (gs->cure.fundingPerTick < 1.0f) gs->cure.fundingPerTick = 1.0f;
            break;

        case 7: /* "Volunteer Surge" */
            gs->cure.rpPerTick += 0.3f;
            break;

        case 8: /* "Supply Disruption" */
            gs->cure.globalDistributed -= 0.05f;
            if (gs->cure.globalDistributed < 0.0f) gs->cure.globalDistributed = 0.0f;
            break;

        case 10: /* "Supply Chain Collapse" */
            gs->cure.productionRate *= 0.5f;
            gs->cure.fundingPerTick -= 2.0f;
            if (gs->cure.fundingPerTick < 0.5f) gs->cure.fundingPerTick = 0.5f;
            break;

        case 11: /* "Political Infighting" */
            gs->cure.funding -= 50.0f;
            if (gs->cure.funding < 0.0f) gs->cure.funding = 0.0f;
            break;

        case 12: /* "Medical Miracle" */
            gs->cure.researchProgress += 25.0f;
            gs->cure.rpPerTick += 0.5f;
            break;

        case 2: /* "Mutation Detected" */
            gs->cure.stability -= 0.10f;
            if (gs->cure.stability < 0.3f) gs->cure.stability = 0.3f;
            break;

        case 3: /* "Public Panic" */
            for (int r = 0; r < MAX_REGIONS; r++)
            {
                gs->regions[r].publicTrust -= 0.05f;
                if (gs->regions[r].publicTrust < 0.1f) gs->regions[r].publicTrust = 0.1f;
            }
            break;

        case 4: /* "Border Lockdown" */
            for (int r = 0; r < MAX_REGIONS; r++)
            {
                gs->regions[r].borderControl += 0.10f;
                if (gs->regions[r].borderControl > 1.0f) gs->regions[r].borderControl = 1.0f;
            }
            break;

        case 13: /* "Winter is coming" - cold-adapted strain boosts infection in cold regions */
            gs->virus.activeTraits |= TRAIT_COLD_ADAPTED;
            gs->virus.infectivity += 0.08f;
            /* Boost infection in cold climate regions */
            for (int r = 0; r < MAX_REGIONS; r++)
            {
                if (gs->regions[r].climate == CLIMATE_COLD)
                {
                    gs->regions[r].infected += 0.03f;
                    if (gs->regions[r].infected > 1.0f) gs->regions[r].infected = 1.0f;
                }
            }
            break;

        default:
            /* Events 0, 9 are informational only */
            break;
    }
}

void events_update(GameState *gs, float delta)
{
    for (int i = 0; i < MAX_EVENTS; i++)
    {
        if (gs->eventLog[i].active) // Only update remaining showtime of the active elements in the eventlog
        {
            gs->eventLog[i].timer -= delta; // Subtract a microscopic slice of time (e.g., 0.016s)
            
            if (gs->eventLog[i].timer <= 0)  // Has the countdown reached 0?
            {
                gs->eventLog[i].active = 0; // Turn it OFF (stops drawing)
                gs->eventCount--;           // Subtract from active event count
            }
        }
    }
}
