#include "raylib.h"
#include "types.h"
#include "virus.h"
#include "region.h"
#include "cure.h"
#include "ui.h"
#include "events.h"
#include <stdio.h>

/* ---------------- virus-system simulation (unchanged) ---------------- */

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
        float effectiveHealthcare = r->healthcareCapacity + (r->cureResearch / 100.0f) * 0.2f;
        if (effectiveHealthcare > 1.0f) effectiveHealthcare = 1.0f;

        float localInfectivity = (v->infectivity + climateBonus) * (1.0f - effectiveBorder * 0.5f);
        float dampening = 1.0f - (effectiveHealthcare * 0.3f + r->vaccinated * 0.5f);
        if (dampening < 0.1f) dampening = 0.1f;

        float localGrowth = localInfectivity * prevInfected[i] * (1.0f - prevInfected[i]) * dampening * dtDays;

        float incoming = 0.0f;
        for (int j = 0; j < MAX_REGIONS; j++)
        {
            if (j == i) continue;
            incoming += prevInfected[j] * gs->regions[j].population;
        }
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
        float effectiveHealthcare = r->healthcareCapacity + (r->cureResearch / 100.0f) * 0.2f;
        if (effectiveHealthcare > 1.0f) effectiveHealthcare = 1.0f;

        float localSeverity = v->severity * (1.0f - effectiveHealthcare * 0.5f);
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
}

/* ---------------- main ---------------- */

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Cure Inc.");
    SetTargetFPS(60);
    InitUI();

    GameState state = {0};
    state.screen              = SCREEN_MENU;
    state.selectedRegionIndex = 2;

    AppScreen appScreen  = STATE_MAIN_MENU;
    GameStats stats      = {0};
    RegionData activeRegion = {0};
    int savedSpeed       = 1;   /* remembers speed across pause/unpause */

    Rectangle regionClickTarget = { 400, 300, 200, 40 };

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime() * state.gameSpeed;
        state.paused = (appScreen == STATE_PAUSED);

        if (appScreen == STATE_GAMEPLAY && !state.paused)
        {
            state.dayTimer += dt;
            if (state.dayTimer >= state.dayLength)
            {
                state.dayTimer -= state.dayLength;
                state.day++;
                day_tick(&state, 1.0f);

                /* trigger a random event every 7 days */
                if (state.day % 7 == 0)
                    events_trigger_random(&state);
            }
            events_update(&state, dt);
        }

        /* mirror real sim data -> UI-facing structs */
        Region *sel = &state.regions[state.selectedRegionIndex];

        stats.cureProgress    = state.cure.researchProgress;
        stats.globalInfection = state.virus.globalInfected * 100.0f;
        stats.budget          = (int)state.cure.funding;
        stats.dayCount        = state.day;

        /* speed: show 0 when paused, otherwise show real speed */
        if (state.paused)
            stats.gameSpeed = 0;
        else if (appScreen == STATE_GAMEPLAY)
            stats.gameSpeed = state.gameSpeed;

        activeRegion.name          = sel->name;
        activeRegion.population    = (int)(sel->population * 1000000.0f);
        activeRegion.infectedCount = (int)(sel->infected * activeRegion.population);
        activeRegion.cureResearch  = sel->cureResearch;
        activeRegion.bordersClosed = sel->bordersClosed;

        BeginDrawing();
            ClearBackground(RAYWHITE);

            switch (appScreen)
            {
                case STATE_MAIN_MENU: {
                    AppScreen before = appScreen;
                    DrawMainMenu(&appScreen);
                    if (before == STATE_MAIN_MENU && appScreen == STATE_GAMEPLAY)
                    {
                        state.screen = SCREEN_GAME;
                        reset_game(&state);
                        savedSpeed = 1;
                        activeRegion.isSelected = false;
                    }
                    break;
                }

                case STATE_GAMEPLAY:
                case STATE_PAUSED: {
                    DrawText("[ Interactive World Map Placeholder ]", 350, 200, 20, LIGHTGRAY);

                    bool hovered = CheckCollisionPointRec(GetMousePosition(), regionClickTarget);
                    DrawRectangleRec(regionClickTarget, hovered ? SKYBLUE : BLUE);
                    DrawRectangleLinesEx(regionClickTarget, 2, DARKBLUE);
                    DrawText(activeRegion.name, (int)regionClickTarget.x + 10, (int)regionClickTarget.y + 10, 18, WHITE);

                    if (appScreen == STATE_GAMEPLAY && hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                        activeRegion.isSelected = true;

                    if (appScreen == STATE_GAMEPLAY && IsKeyPressed(KEY_TAB))
                        state.selectedRegionIndex = (state.selectedRegionIndex + 1) % MAX_REGIONS;

                    /* draw active event notifications at bottom of screen */
                    int eventY = SCREEN_HEIGHT - 80;
                    for (int i = 0; i < MAX_EVENTS; i++)
                    {
                        if (state.eventLog[i].active)
                        {
                            Rectangle evtBox = { 20, (float)eventY, 1000, 46 };
                            DrawRectangleRec(evtBox, Fade(DARKBLUE, 0.85f));
                            DrawRectangleLinesEx(evtBox, 1.5f, BLUE);

                            char evtText[256];
                            snprintf(evtText, sizeof(evtText), "[!] %s: %s",
                                    state.eventLog[i].title,
                                    state.eventLog[i].description);
                            DrawText(evtText, 28, eventY + 13, 20, WHITE);
                            eventY -= 54;
                        }
                    }

                    DrawGameplayHUD(&appScreen, &stats);

                    Rectangle sidePanelBounds = { (float)SCREEN_WIDTH - 320, 70, 300, 350 };
                    DrawRegionPanel(sidePanelBounds, &activeRegion, &stats);

                    if (appScreen == STATE_PAUSED)
                        DrawPauseOverlay(&appScreen);
                    break;
                }

                case STATE_GAME_OVER: {
                    if (state.screen == SCREEN_WIN)
                        DrawText("CURE DISTRIBUTED - HUMANITY SAVED", 300, 320, 34, DARKGREEN);
                    else
                        DrawText("HUMANITY HAS FALLEN", 420, 320, 40, RED);

                    Rectangle menuBtn = { (float)(SCREEN_WIDTH - 200) / 2, 420, 200, 50 };
                    if (DrawUIButton(menuBtn, "MAIN MENU", BLUE, SKYBLUE))
                    {
                        appScreen    = STATE_MAIN_MENU;
                        state.screen = SCREEN_MENU;
                    }
                    break;
                }

                default: break;
            }
        EndDrawing();

        /* write player actions back into the real sim */
        state.cure.funding = (float)stats.budget;
        sel->cureResearch  = activeRegion.cureResearch;
        sel->bordersClosed = activeRegion.bordersClosed;

        /* sync speed: save it when > 0, restore it when unpausing */
        if (stats.gameSpeed > 0)
        {
            savedSpeed      = stats.gameSpeed;
            state.gameSpeed = stats.gameSpeed;
        }
        else if (!state.paused)
        {
            state.gameSpeed = savedSpeed;
            stats.gameSpeed = savedSpeed;
        }

        /* sync sim win/lose into UI flow */
        if ((state.screen == SCREEN_WIN || state.screen == SCREEN_LOSE) && appScreen != STATE_GAME_OVER)
            appScreen = STATE_GAME_OVER;
    }

    CloseWindow();
    return 0;
}