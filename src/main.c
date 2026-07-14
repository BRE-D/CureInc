#include "raylib.h"
#include "types.h"
#include "virus.h"
#include "region.h"
#include "cure.h"
#include "ui.h"
#include "events.h"
#include <stdio.h>


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

/* ---------------- screen-state translation layer ----------------
   These two functions are the ONLY place GameScreen and AppScreen
   ever touch each other. AppScreen is never stored persistently —
   it's rebuilt from GameState every frame, so the two can never
   drift out of sync the way manually-copied variables can. */

static AppScreen derive_app_screen(const GameState *gs)
{
    if (gs->screen == SCREEN_WIN || gs->screen == SCREEN_LOSE) return STATE_GAME_OVER;
    if (gs->screen == SCREEN_MENU) return STATE_MAIN_MENU;
    return gs->paused ? STATE_PAUSED : STATE_GAMEPLAY;
}

static void apply_app_screen(GameState *gs, AppScreen appScreen)
{
    switch (appScreen)
    {
        case STATE_MAIN_MENU:
            gs->screen = SCREEN_MENU;
            gs->paused = 0;
            break;
        case STATE_GAMEPLAY:
            if (gs->screen != SCREEN_GAME) { gs->screen = SCREEN_GAME; reset_game(gs); }
            gs->paused = 0;
            break;
        case STATE_PAUSED:
            gs->paused = 1;
            break;
        case STATE_GAME_OVER:
            /* nothing to write back — state.screen is already WIN/LOSE */
            break;
    }
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

    GameStats stats         = {0};
    RegionData activeRegion = {0};
    int savedSpeed          = 1;

    Rectangle regionClickTarget = { 400, 300, 200, 40 };

    while (!WindowShouldClose())
    {
        AppScreen appScreen = derive_app_screen(&state);
        float dt = GetFrameTime() * state.gameSpeed;

        if (appScreen == STATE_GAMEPLAY)
        {
            state.dayTimer += dt;
            if (state.dayTimer >= state.dayLength)
            {
                state.dayTimer -= state.dayLength;
                state.day++;
                day_tick(&state, 1.0f);

                if (state.day % 7 == 0)
                    events_trigger_random(&state);
            }
            events_update(&state, dt);
        }

        Region *sel = &state.regions[state.selectedRegionIndex];

        stats.cureProgress    = state.cure.researchProgress;
        stats.globalInfection = state.virus.globalInfected * 100.0f;
        stats.budget          = (int)state.cure.funding;
        stats.dayCount        = state.day;

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
                case STATE_MAIN_MENU:
                    DrawMainMenu(&appScreen);
                    if (appScreen == STATE_GAMEPLAY)
                        activeRegion.isSelected = false;
                    break;

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
                        appScreen = STATE_MAIN_MENU;
                    break;
                }

                default: break;
            }
        EndDrawing();

        state.cure.funding = (float)stats.budget;
        sel->cureResearch  = activeRegion.cureResearch;
        sel->bordersClosed = activeRegion.bordersClosed;

        if (stats.gameSpeed > 0)
        {
            savedSpeed      = stats.gameSpeed;
            state.gameSpeed = stats.gameSpeed;
        }
        else if (!state.paused)
        {
            state.gameSpeed = savedSpeed;
        }

        apply_app_screen(&state, appScreen);
    }

    CloseWindow();
    return 0;
}