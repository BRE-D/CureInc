#include "raylib.h"
#include "types.h"
#include "virus.h"
#include "region.h"
#include "cure.h"
#include "ui.h"
#include "events.h"
#include <stdio.h>

// virus-system simulation

static float region_effective_healthcare(const Region *r)
{
    float eh = r->healthcareCapacity + (r->cureResearch / 100.0f) * 0.2f;
    return (eh > 1.0f) ? 1.0f : eh;
}

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
        if (r->climate == CLIMATE_COLD && virus_has_trait(v, TRAIT_COLD_ADAPTED)) climateBonus = 0.10f;
        if (r->climate == CLIMATE_HOT && virus_has_trait(v, TRAIT_HOT_ADAPTED)) climateBonus = 0.10f;

        float effectiveBorder = r->bordersClosed ? 1.0f : r->borderControl;
        float dampening = 1.0f - (region_effective_healthcare(r) * 0.3f + r->vaccinated * 0.5f);
        if (dampening < 0.1f) dampening = 0.1f;

        float localInfectivity = (v->infectivity + climateBonus) * (1.0f - effectiveBorder * 0.5f);
        float localGrowth = localInfectivity * prevInfected[i] * (1.0f - prevInfected[i]) * dampening * dtDays;

        float incoming = 0.0f;
        for (int j = 0; j < MAX_REGIONS; j++)
            if (j != i) incoming += prevInfected[j] * gs->regions[j].population;

        float importPressure = GLOBAL_MIXING_RATE * (v->infectivity + climateBonus) * incoming
                                * (1.0f - effectiveBorder) * dtDays;

        r->infected += localGrowth + importPressure;
        if (r->infected > 1.0f) r->infected = 1.0f;
        if (r->infected < 0.0f) r->infected = 0.0f;
    }
}

static void apply_deaths(GameState *gs, float dtDays)
{
    Virus *v = &gs->virus;
    float totalPop = 0.0f, weightedDeaths = 0.0f;

    for (int i = 0; i < MAX_REGIONS; i++)
    {
        Region *r = &gs->regions[i];
        float localSeverity = v->severity * (1.0f - region_effective_healthcare(r) * 0.5f);
        totalPop       += r->population;
        weightedDeaths += r->population * (localSeverity * r->infected * dtDays);
    }

    if (totalPop > 0.0f)
    {
        v->globalDead += weightedDeaths / totalPop;
        if (v->globalDead > v->globalInfected) v->globalDead = v->globalInfected;
    }
}

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
    if (gs->virus.globalDead >= 0.50f) gs->screen = SCREEN_LOSE;
    else if (gs->cure.globalDistributed >= 1.0f) gs->screen = SCREEN_WIN;
}

static void day_tick(GameState *gs, float dtDays)
{
    spread_infection(gs, dtDays);
    apply_deaths(gs, dtDays);
    aggregate_global_stats(gs);
    virus_try_mutate(&gs->virus, dtDays);
    cure_update(&gs->cure, dtDays);
    region_update_states(gs);
    check_win_lose(gs);
}

static void reset_game(GameState *gs)
{
    GameScreen keepScreen = gs->screen;
    *gs = (GameState){0};
    gs->screen              = keepScreen;
    gs->dayLength           = DEFAULT_DAY_LENGTH;
    gs->gameSpeed           = 1;
    gs->selectedRegionIndex = 2;

    virus_init(&gs->virus);
    region_init(gs);
    cure_init(&gs->cure);
    events_init(gs);
    UI_ResetGameplayState();
}

// main

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Cure Inc.");
    SetTargetFPS(60);
    InitUI();

    GameState state = {0};
    state.screen              = SCREEN_MENU;
    state.selectedRegionIndex = 2;

    Rectangle regionNode = { 400, 300, 200, 40 };

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime() * state.gameSpeed;

        if (state.screen == SCREEN_GAME)
        {
            state.dayTimer += dt;
            if (state.dayTimer >= state.dayLength)
            {
                state.dayTimer -= state.dayLength;
                state.day++;
                day_tick(&state, 1.0f);
                if (state.day % 7 == 0) events_trigger_random(&state);
            }
            events_update(&state, dt);
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);
            switch (state.screen)
            {
                case SCREEN_MENU: {
                    UIAction action = UI_DrawMainMenu(state.screen);
                    if (action == UI_START_GAME) {
                        state.screen = SCREEN_GAME;
                        reset_game(&state);
                    }
                    break;
                }
                case SCREEN_GAME:
                case SCREEN_PAUSED:
                    UI_DrawGameplay(&state, regionNode);
                    break;
                case SCREEN_WIN:
                case SCREEN_LOSE: {
                    UIAction action = UI_DrawEndScreen(state.screen);
                    if (action == UI_MAIN_MENU) {
                        state.screen = SCREEN_MENU;
                    }
                    break;
                }
                default: break;
            }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}