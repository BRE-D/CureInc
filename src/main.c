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

//UI <-> sim data bridge 

static void pull_ui_snapshot(const GameState *gs, GameStats *stats, RegionData *rd)
{
    const Region *sel = &gs->regions[gs->selectedRegionIndex];
    bool gameplayActive = (gs->screen == SCREEN_GAME);

    stats->cureProgress    = gs->cure.researchProgress;
    stats->globalInfection = gs->virus.globalInfected * 100.0f;
    stats->budget          = (int)gs->cure.funding;
    stats->dayCount        = gs->day;
    stats->gameSpeed       = (gs->screen == SCREEN_PAUSED) ? 0
                              : gameplayActive ? gs->gameSpeed : stats->gameSpeed;

    rd->name          = sel->name;
    rd->population    = (int)(sel->population * 1000000.0f);
    rd->infectedCount = (int)(sel->infected * rd->population);
    rd->cureResearch  = sel->cureResearch;
    rd->bordersClosed = sel->bordersClosed;
}

static void push_ui_actions(GameState *gs, const GameStats *stats, const RegionData *rd, int *savedSpeed)
{
    Region *sel = &gs->regions[gs->selectedRegionIndex];

    gs->cure.funding   = (float)stats->budget;
    sel->cureResearch  = rd->cureResearch;
    sel->bordersClosed = rd->bordersClosed;

    if (stats->gameSpeed > 0) { *savedSpeed = stats->gameSpeed; gs->gameSpeed = stats->gameSpeed; }
    else if (gs->screen != SCREEN_PAUSED) { gs->gameSpeed = *savedSpeed; }
}

//drawing helpers

static void draw_event_log(const GameState *gs)
{
    int y = SCREEN_HEIGHT - 80;
    for (int i = 0; i < MAX_EVENTS; i++)
    {
        if (!gs->eventLog[i].active) continue;

        Rectangle box = { 20, (float)y, 1000, 46 };
        DrawRectangleRec(box, Fade(DARKBLUE, 0.85f));
        DrawRectangleLinesEx(box, 1.5f, BLUE);

        char text[256];
        snprintf(text, sizeof(text), "[!] %s: %s", gs->eventLog[i].title, gs->eventLog[i].description);
        DrawText(text, 28, y + 13, 20, WHITE);
        y -= 54;
    }
}

static void draw_gameplay(GameState *gs, GameStats *stats, RegionData *rd, Rectangle regionNode)
{
    DrawText("[ Interactive World Map Placeholder ]", 350, 200, 20, LIGHTGRAY);

    bool hovered = CheckCollisionPointRec(GetMousePosition(), regionNode);
    DrawRectangleRec(regionNode, hovered ? SKYBLUE : BLUE);
    DrawRectangleLinesEx(regionNode, 2, DARKBLUE);
    DrawText(rd->name, (int)regionNode.x + 10, (int)regionNode.y + 10, 18, WHITE);

    if (gs->screen == SCREEN_GAME)
    {
        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) rd->isSelected = true;
        if (IsKeyPressed(KEY_TAB)) gs->selectedRegionIndex = (gs->selectedRegionIndex + 1) % MAX_REGIONS;
    }

    draw_event_log(gs);

    DrawGameplayHUD(&gs->screen, stats);

    Rectangle panel = { (float)SCREEN_WIDTH - 320, 70, 300, 350 };
    DrawRegionPanel(panel, rd, stats);

    if (gs->screen == SCREEN_PAUSED) DrawPauseOverlay(&gs->screen);
}

static void draw_game_over(GameState *gs)
{
    DrawText(gs->screen == SCREEN_WIN ? "CURE DISTRIBUTED - HUMANITY SAVED" : "HUMANITY HAS FALLEN",
              gs->screen == SCREEN_WIN ? 300 : 420, 320, gs->screen == SCREEN_WIN ? 34 : 40,
              gs->screen == SCREEN_WIN ? DARKGREEN : RED);

    Rectangle menuBtn = { (float)(SCREEN_WIDTH - 200) / 2, 420, 200, 50 };
    if (DrawUIButton(menuBtn, "MAIN MENU", BLUE, SKYBLUE))
        gs->screen = SCREEN_MENU;
}

//main

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
    Rectangle regionNode     = { 400, 300, 200, 40 };

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

        pull_ui_snapshot(&state, &stats, &activeRegion);

        BeginDrawing();
            ClearBackground(RAYWHITE);
            switch (state.screen)
            {
                case SCREEN_MENU: {
                    GameScreen before = state.screen;
                    DrawMainMenu(&state.screen);
                    if (before == SCREEN_MENU && state.screen == SCREEN_GAME)
                    {
                        reset_game(&state);
                        activeRegion.isSelected = false;
                    }
                    break;
                }
                case SCREEN_GAME:
                case SCREEN_PAUSED:
                    draw_gameplay(&state, &stats, &activeRegion, regionNode);
                    break;
                case SCREEN_WIN:
                case SCREEN_LOSE:
                    draw_game_over(&state);
                    break;
                default: break;
            }
        EndDrawing();

        push_ui_actions(&state, &stats, &activeRegion, &savedSpeed);
    }

    CloseWindow();
    return 0;
}