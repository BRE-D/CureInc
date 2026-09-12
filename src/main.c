#include "raylib.h"
#include "types.h"
#include "virus.h"
#include "region.h"
#include "cure.h"
#include "ui.h"
#include "events.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// virus-system simulation

static void check_win_lose(GameState *gs)
{
    int collapsed = 0;
    for (int i = 0; i < MAX_REGIONS; i++)
        if (gs->regions[i].overloadedDays >= 30) collapsed++;

    if (gs->virus.globalDead >= 0.40f) {
        gs->screen = SCREEN_LOSE;
        gs->endReason = "Deaths reached 40% of the original population.";
    } else if (collapsed == MAX_REGIONS) {
        gs->screen = SCREEN_LOSE;
        gs->endReason = "Every region has been overloaded for 30 days.";
    } else {
        float living = 1.0f - gs->virus.globalDead;
        if (living > 0.0f && gs->cure.phase == PHASE_DISTRIBUTION &&
            gs->cure.globalDistributed >= 0.90f &&
            gs->virus.globalInfected / living < 0.05f) {
            gs->screen = SCREEN_WIN;
            gs->endReason = "Vaccination reached 90% and infection fell below 5%.";
        }
    }
}

static void day_tick(GameState *gs)
{
    if (gs->day % 7 == 0) events_trigger_random(gs);

    if (virus_try_mutate(&gs->virus, gs->day)) {
        gs->cure.stability -= 0.03f;
        if (gs->cure.stability < 0.60f) gs->cure.stability = 0.60f;
        events_add(gs, "Virus Mutated", virus_trait_name(gs->virus.lastMutation));
    }

    virus_update(gs);
    cure_update(gs, 1.0f);
    virus_refresh_totals(gs);
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
    virus_refresh_totals(gs);
    region_update_states(gs);
    UI_ResetGameplayState();
}

// main

int main(void)
{
    srand((unsigned int)time(NULL));
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Cure Inc.");
    SetTargetFPS(60);
    InitUI();

    GameState state = {0};
    state.screen              = SCREEN_MENU;
    state.selectedRegionIndex = 2;

    Rectangle regionNode = { 400, 300, 200, 40 };

    while (!WindowShouldClose())
    {
        float frameTime = GetFrameTime();
        float dt = frameTime * state.gameSpeed;

        if (state.screen == SCREEN_GAME)
        {
            state.dayTimer += dt;
            while (state.dayTimer >= state.dayLength &&
                   state.screen == SCREEN_GAME)
            {
                state.dayTimer -= state.dayLength;
                state.day++;
                day_tick(&state);
            }
            events_update(&state, frameTime);
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