#include "raylib.h"
#include "types.h"
#include "virus.h"
#include "region.h"
#include "cure.h"
#include "events.h"

/* ----------------------------------------------------------------
   Daily simulation step. Each function below does exactly one job;
   day_tick() just runs them in the right order.
   ---------------------------------------------------------------- */

/* Spreads infection two ways:
   1. Within-region growth (logistic curve, needs existing infection)
   2. Cross-region import (lets a region at 0% catch the virus from
      other infected regions, dampened by its own border control)
   Also applies a climate bonus if the virus has mutated a matching
   COLD_ADAPTED / HOT_ADAPTED trait for that region's climate zone. */
static void spread_infection(GameState *gs, float dtDays)
{
    Virus *v = &gs->virus;

    float prevInfected[MAX_REGIONS];
    for (int i = 0; i < MAX_REGIONS; i++)
        prevInfected[i] = gs->regions[i].infected;

    for (int i = 0; i < MAX_REGIONS; i++)
    {
        Region *r = &gs->regions[i];

        float climateBonus = 0.0f;
        if (r->climate == CLIMATE_COLD && virus_has_trait(v, TRAIT_COLD_ADAPTED))
            climateBonus = 0.10f;
        if (r->climate == CLIMATE_HOT && virus_has_trait(v, TRAIT_HOT_ADAPTED))
            climateBonus = 0.10f;

        float localInfectivity = (v->infectivity + climateBonus) * (1.0f - r->borderControl * 0.5f);
        float dampening = 1.0f - (r->healthcareCapacity * 0.3f + r->vaccinated * 0.5f);
        if (dampening < 0.1f) dampening = 0.1f;

        float localGrowth = localInfectivity * prevInfected[i] * (1.0f - prevInfected[i])
                             * dampening * dtDays;

        float incoming = 0.0f;
        for (int j = 0; j < MAX_REGIONS; j++)
        {
            if (j == i) continue;
            incoming += prevInfected[j] * gs->regions[j].population;
        }
        float importPressure = GLOBAL_MIXING_RATE * (v->infectivity + climateBonus) * incoming
                                * (1.0f - r->borderControl) * dtDays;

        r->infected += localGrowth + importPressure;
        if (r->infected > 1.0f) r->infected = 1.0f;
        if (r->infected < 0.0f) r->infected = 0.0f;
    }
}

/* Deaths accrue per region based on severity and local healthcare
   quality, then roll up into one global death fraction. */
static void apply_deaths(GameState *gs, float dtDays)
{
    Virus *v = &gs->virus;
    float totalPop = 0.0f, weightedDeaths = 0.0f;

    for (int i = 0; i < MAX_REGIONS; i++)
    {
        Region *r = &gs->regions[i];
        float localSeverity = v->severity * (1.0f - r->healthcareCapacity * 0.5f);
        float deaths = localSeverity * r->infected * dtDays;

        totalPop       += r->population;
        weightedDeaths += r->population * deaths;
    }

    if (totalPop > 0.0f)
    {
        v->globalDead += weightedDeaths / totalPop;
        if (v->globalDead > v->globalInfected) v->globalDead = v->globalInfected;
    }
}

/* Population calculation: globalInfected is a population-weighted
   average across all regions, not an independently simulated number. */
static void aggregate_global_stats(GameState *gs)
{
    float totalPop = 0.0f, weightedInfected = 0.0f;

    for (int i = 0; i < MAX_REGIONS; i++)
    {
        totalPop         += gs->regions[i].population;
        weightedInfected += gs->regions[i].population * gs->regions[i].infected;
    }

    if (totalPop > 0.0f)
        gs->virus.globalInfected = weightedInfected / totalPop;
}

static void check_win_lose(GameState *gs)
{
    if (gs->virus.globalDead >= 0.50f)
        gs->screen = SCREEN_LOSE;
    else if (gs->cure.globalDistributed >= 1.0f)
        gs->screen = SCREEN_WIN;
}

/* Runs once per simulated day. This is the entire "System Reacts"
   step from the design doc. */
static void day_tick(GameState *gs, float dtDays)
{
    spread_infection(gs, dtDays);
    virus_try_mutate(&gs->virus, dtDays);
    apply_deaths(gs, dtDays);
    aggregate_global_stats(gs);
    cure_update(&gs->cure, dtDays);
    region_update_states(gs);
    check_win_lose(gs);
}

/* Resets all simulation state for a fresh run, keeping whatever
   screen the caller already set (so callers control the transition,
   this function only handles the data reset). */
static void reset_game(GameState *gs)
{
    GameScreen keepScreen = gs->screen;
    *gs = (GameState){0};
    gs->screen    = keepScreen;
    gs->dayLength = DEFAULT_DAY_LENGTH;
    gs->gameSpeed = 1;

    virus_init(&gs->virus);
    region_init(gs);
    cure_init(&gs->cure);
    events_init(gs); // Initialize events subsystem
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CureInc");
    SetTargetFPS(60);
    SetWindowPosition(0, 0);

    GameState state = {0};
    state.screen = SCREEN_MENU;

    float eventTimer = 0.0f; // Triggers the next random event

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime() * state.gameSpeed;

        switch (state.screen)
        {
            case SCREEN_MENU:
                if (IsKeyPressed(KEY_ENTER))
                {
                    state.screen = SCREEN_GAME;
                    reset_game(&state);
                }
                break;

            case SCREEN_GAME:
                if (!state.paused)
                {
                    state.dayTimer += dt;
                    if (state.dayTimer >= state.dayLength)
                    {
                        state.dayTimer -= state.dayLength;
                        state.day++;
                        day_tick(&state, 1.0f);
                    }

                    // Ticks down the event timer and spawns events in real-world seconds
                    float realDt = GetFrameTime();
                    eventTimer += realDt;
                    if (eventTimer >= 5.0f)
                    {
                        events_trigger_random(&state);
                        eventTimer = 0.0f;
                    }
                    events_update(&state, realDt);
                }
                if (IsKeyPressed(KEY_P)) state.paused = !state.paused;
                break;

            case SCREEN_WIN:
            case SCREEN_LOSE:
                if (IsKeyPressed(KEY_R))
                    state.screen = SCREEN_MENU;
                break;
        }

        BeginDrawing();
            ClearBackground((Color){ 15, 15, 25, 255 });

            if (state.screen == SCREEN_MENU)
            {
                DrawText("CureInc", 100, 300, 60, WHITE);
                DrawText("Press ENTER to start", 100, 380, 24, (Color){ 160, 170, 185, 255 });
            }
            else
            {
                // Global Info Group (Left Column)
                DrawText(TextFormat("Day: %d", state.day), 100, 60, 32, WHITE);
                DrawText(TextFormat("Global Infected: %.2f%%", state.virus.globalInfected * 100.0f),
                         100, 105, 22, (Color){ 240, 70, 70, 255 }); // Soft, vibrant red
                DrawText(TextFormat("Global Dead: %.2f%%", state.virus.globalDead * 100.0f),
                         100, 135, 22, (Color){ 160, 170, 185, 255 }); // Slate gray/blue
                DrawText(TextFormat("Cure: %.0f%% distributed (phase %d)",
                                     state.cure.globalDistributed * 100.0f, state.cure.phase),
                         100, 165, 22, (Color){ 90, 200, 250, 255 }); // Sky blue

                // Regional Header
                DrawText("Regional Infected:", 100, 210, 24, (Color){ 50, 230, 120, 255 });

                // Region List
                for (int i = 0; i < MAX_REGIONS; i++) {
                    DrawText(TextFormat("%s: %.1f%%", state.regions[i].name,
                                         state.regions[i].infected * 100.0f),
                             100, 250 + i * 26, 18, (Color){ 220, 225, 235, 255 }); // Off-white/Silver
                }

                // Event Log (Right Column)
                DrawText("Latest Events:", 750, 210, 24, YELLOW);
                int ypos = 250;
                for (int i = 0; i < MAX_EVENTS; i++)
                {
                    if (state.eventLog[i].active)
                    {
                        DrawText(state.eventLog[i].title, 750, ypos, 20, YELLOW);
                        DrawText(state.eventLog[i].description, 750, ypos + 24, 16, (Color){ 200, 200, 200, 255 });
                        ypos += 64;
                    }
                }

                if (state.paused)
                    DrawText("PAUSED (P to resume)", 100, 480, 22, YELLOW);

                if (state.screen == SCREEN_LOSE)
                {
                    DrawText("HUMANITY HAS FALLEN", 100, 540, 40, (Color){ 255, 30, 30, 255 });
                    DrawText("Press R to return to menu", 100, 590, 20, (Color){ 160, 170, 185, 255 });
                }
                else if (state.screen == SCREEN_WIN)
                {
                    DrawText("CURE DISTRIBUTED - HUMANITY SAVED", 100, 540, 40, (Color){ 50, 230, 120, 255 });
                    DrawText("Press R to return to menu", 100, 590, 20, (Color){ 160, 170, 185, 255 });
                }

                DrawText("Press ESC to exit", 100, 1000, 20, (Color){ 90, 95, 110, 255 }); // Faded status
            }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}