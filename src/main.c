#include "raylib.h"
#include "types.h"
#include "virus.h"
#include "region.h"
#include "cure.h"
#include "ui.h"
#include "events.h"
#include <stdlib.h>
#include <time.h>


// Check the daily result. Deaths or 30 overloaded days in every region cause defeat first. Otherwise,
// Distribution needs 90% of survivors protected and fewer than 5% infected. Updates the screen and reason;
// returns nothing.
// gs: shared game state; const means this function only reads it.
static void check_win_lose(GameState *gs) {
    // Count of regions with at least 30 consecutive overloaded days.
    int collapsed = 0;
    // i: Zero-based index used to visit each item in this loop.
    for (int i = 0; i < MAX_REGIONS; i++)
        if (gs->regions[i].overloadedDays >= 30) collapsed++;
    if (gs->virus.globalDead >= LOSS_DEATH_SHARE) {
        gs->screen = SCREEN_LOSE;
        gs->endReason = "Deaths reached the allowed limit.";
    } else if (collapsed == MAX_REGIONS) {
        gs->screen = SCREEN_LOSE;
        gs->endReason = "Every region has been overloaded for 30 days.";
    } else {
        // Surviving fraction of original world population; used as the win-target denominator.
        float living = 1.0f - gs->virus.globalDead;
        if (living > 0 && gs->cure.phase == PHASE_DISTRIBUTION &&
            gs->cure.globalDistributed >= .90f && gs->virus.globalInfected / living < .05f) {
            gs->screen = SCREEN_WIN;
            gs->endReason = "Vaccination reached 90% and infection fell below 5%.";
        }
    }
}


// Run one game day: weekly event, mutation, virus, cure, global totals, region states, phase notice, then
// win/lose check. Called by main after the day counter increases; returns nothing.
// gs: shared game state; const means this function only reads it.
static void day_tick(GameState *gs) {
    // Trigger an event every seven game days before the simulation update.
    if (gs->day % 7 == 0) events_trigger_random(gs);
    if (virus_try_mutate(&gs->virus, gs->day)) {
        // A mutation removes 3 stability percentage points; the next line keeps at least 60%.
        gs->cure.stability -= .03f;
        if (gs->cure.stability < .60f) gs->cure.stability = .60f;
        events_add(gs, "Virus Mutated", virus_trait_name(gs->virus.lastMutation));
    }
    // Phase before the daily updates, saved to detect and announce a transition.
    ResearchPhase oldPhase = gs->cure.phase;
    virus_update(gs);
    cure_update(gs, 1);
    virus_refresh_totals(gs);
    region_update_states(gs);
    if (oldPhase != gs->cure.phase)
        events_add(gs, "Phase Complete", "Next cure stage unlocked. Follow the guidance above the regions.");
    check_win_lose(gs);
}


// Clear the previous game while keeping the chosen screen, then initialize every system and reset the UI.
// Called when Play is clicked; returns nothing.
// gs: shared game state; const means this function only reads it.
static void reset_game(GameState *gs) {
    // Saved screen choice retained while all other game data is reset.
    GameScreen screen = gs->screen;
    // Named fields set the new defaults; all omitted fields are zeroed.
    *gs = (GameState){.screen=screen, .dayLength=DEFAULT_DAY_LENGTH, .gameSpeed=1, .selectedRegionIndex=2};
    virus_init(&gs->virus);
    region_init(gs);
    cure_init(&gs->cure);
    events_init(gs);
    virus_refresh_totals(gs);
    UI_ResetGameplayState();
}


// Program entry point. Open the window, collect frame time, run complete game days, draw the current
// screen, and handle menu actions. Close the window on exit; return 0 for normal completion.
int main(void) {
    srand((unsigned)time(NULL));
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Cure Inc. - Challenge");
    SetTargetFPS(60);
    // Single shared game state; initially only the menu screen is set and other fields are zero.
    GameState state = {.screen=SCREEN_MENU};
    // True after the Exit button asks the main loop to stop.
    bool exitRequested = false;
    while (!WindowShouldClose() && !exitRequested) {
        // Real seconds since the previous rendered frame.
        float dt = GetFrameTime();
        if (state.screen == SCREEN_GAME) {
            state.dayTimer += dt * state.gameSpeed;
            while (state.dayTimer >= state.dayLength && state.screen == SCREEN_GAME) {
                // Keep leftover time so a slow frame does not skip any complete game days.
                state.dayTimer -= state.dayLength;
                state.day++;
                day_tick(&state);
            }
            events_update(&state, dt);
        }
        BeginDrawing();
        ClearBackground((Color){232, 239, 246, 255});
        // Button request to return or apply; UI_NONE means no request.
        UIAction action = UI_NONE;
        if (state.screen == SCREEN_MENU) action = UI_DrawMainMenu(state.screen);
        else if (state.screen == SCREEN_GAME || state.screen == SCREEN_PAUSED) UI_DrawGameplay(&state);
        else action = UI_DrawEndScreen(&state);
        if (action == UI_START_GAME) { state.screen = SCREEN_GAME; reset_game(&state); }
        if (action == UI_MAIN_MENU) state.screen = SCREEN_MENU;
        if (action == UI_EXIT) exitRequested = true;
        UI_DrawTransition(state.screen);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
