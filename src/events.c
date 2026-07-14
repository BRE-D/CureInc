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
}

void events_trigger_random(GameState *gs)
{
    int pick = GetRandomValue(0, POOL_SIZE - 1);
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

