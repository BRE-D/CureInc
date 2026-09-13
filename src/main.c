#include "raylib.h"
#include "types.h"
#include "virus.h"
#include "region.h"
#include "cure.h"
#include "ui.h"
#include "events.h"
#include <stdlib.h>
#include <time.h>

// আগে হারার শর্ত দেখি; একই দিনে দুই শর্ত পূরণ হলেও আগে পরাজয় হবে।
static void check_win_lose(GameState *gs) {
    int collapsed = 0;
    for (int i = 0; i < MAX_REGIONS; i++)
        if (gs->regions[i].overloadedDays >= 30) collapsed++;
    if (gs->virus.globalDead >= LOSS_DEATH_SHARE) {
        gs->screen = SCREEN_LOSE;
        gs->endReason = "Deaths reached the allowed limit.";
    } else if (collapsed == MAX_REGIONS) {
        gs->screen = SCREEN_LOSE;
        gs->endReason = "Every region has been overloaded for 30 days.";
    } else {
        float living = 1.0f - gs->virus.globalDead; // জীবিতদের ৯০% সুরক্ষিত, ৫%-এর কম আক্রান্ত চাই।
        if (living > 0 && gs->cure.phase == PHASE_DISTRIBUTION &&
            gs->cure.globalDistributed >= .90f && gs->virus.globalInfected / living < .05f) {
            gs->screen = SCREEN_WIN;
            gs->endReason = "Vaccination reached 90% and infection fell below 5%.";
        }
    }
}

// একদিন: event → mutation → সংক্রমণ → cure → মোট হিসাব → জয়/হার।
static void day_tick(GameState *gs) {
    if (gs->day % 7 == 0) events_trigger_random(gs); // প্রতি ৭ দিনে খবর।
    if (virus_try_mutate(&gs->virus, gs->day)) {
        gs->cure.stability -= .03f;
        if (gs->cure.stability < .60f) gs->cure.stability = .60f;
        events_add(gs, "Virus Mutated", virus_trait_name(gs->virus.lastMutation));
    }
    ResearchPhase oldPhase = gs->cure.phase;
    virus_update(gs);
    cure_update(gs, 1);
    virus_refresh_totals(gs);
    region_update_states(gs);
    if (oldPhase != gs->cure.phase) // নতুন ধাপ খুললে player-কে জানাই।
        events_add(gs, "Phase Complete", "Next cure stage unlocked. Follow the guidance above the regions.");
    check_win_lose(gs);
}

// পুরোনো খেলার সব তথ্য মুছে নতুন করে শুরু করি।
static void reset_game(GameState *gs) {
    GameScreen screen = gs->screen;
    *gs = (GameState){.screen=screen, .dayLength=DEFAULT_DAY_LENGTH, .gameSpeed=1, .selectedRegionIndex=2};
    virus_init(&gs->virus);
    region_init(gs);
    cure_init(&gs->cure);
    events_init(gs);
    virus_refresh_totals(gs);
    UI_ResetGameplayState();
}

// Frame-এ সময় জমাই, বর্তমান screen আঁকি, button-এর নির্দেশ পালন করি।
int main(void) {
    srand((unsigned)time(NULL));
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Cure Inc. - Challenge");
    SetTargetFPS(60);
    GameState state = {.screen=SCREEN_MENU};
    bool exitRequested = false;
    while (!WindowShouldClose() && !exitRequested) {
        float dt = GetFrameTime();
        if (state.screen == SCREEN_GAME) {
            state.dayTimer += dt * state.gameSpeed;
            while (state.dayTimer >= state.dayLength && state.screen == SCREEN_GAME) {
                state.dayTimer -= state.dayLength; // Frame দেরি হলেও কোনো পূর্ণ দিন বাদ যায় না।
                state.day++;
                day_tick(&state);
            }
            events_update(&state, dt);
        }
        BeginDrawing();
        ClearBackground((Color){232, 239, 246, 255});
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
