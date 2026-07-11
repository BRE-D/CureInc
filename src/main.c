
#include "raylib.h"
#include "region.h"
#include "types.h"


/* ----------------------------------------------------------------
 * game_init - Zero-initialise GameState and seed all starting values.
 *
 * Sets virus biology parameters, time settings, and delegates
 * region setup to region_init().
 *
 * Parameters:
 *   gs - pointer to a GameState that has already been zero-filled.
 * ---------------------------------------------------------------- */
static void game_init(GameState *gs) {
  /* Scene */
  gs->screen = SCREEN_MENU;

  /* Time */
  gs->day = 1;
  gs->dayTimer = 0.0f;
  gs->dayLength = DEFAULT_DAY_LENGTH; /* 3.0 real seconds per day    */
  gs->gameSpeed = 1;
  gs->paused = 0;

  /* Virus — starting biology values */
  gs->virus.infectivity = 0.03f;  /* 3% base spread per day         */
  gs->virus.severity = 0.02f;     /* low initial mortality impact    */
  gs->virus.resistance = 0.10f;   /* 10% resistance to cure          */
  gs->virus.mutationRate = 0.05f; /* 5% chance of new trait per day  */
  gs->virus.activeTraits = TRAIT_NONE;
  gs->virus.globalInfected = 0.0f;
  gs->virus.globalDead = 0.0f;

  /* Cure — bootstrap funding */
  gs->cure.phase = PHASE_DISCOVERY;
  gs->cure.funding = 500.0f;
  gs->cure.fundingPerTick = 10.0f;
  gs->cure.stability = 1.0f;
  gs->cure.effectiveness = 0.0f;

  /* Populate all 8 regions */
  region_init(gs);
}

/* ----------------------------------------------------------------
 * main - Entry point. Initialises window, seeds game state,
 *        then runs the core game loop.
 * ---------------------------------------------------------------- */
int main(void) {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CureInc");
  SetTargetFPS(60);

  GameState state = {0};
  game_init(&state);

  while (!WindowShouldClose()) {
    /* ---- INPUT & TIME UPDATE ------------------------------- */

    /* Speed controls */
    if (IsKeyPressed(KEY_ONE))
      state.gameSpeed = 1;
    if (IsKeyPressed(KEY_TWO))
      state.gameSpeed = 2;
    if (IsKeyPressed(KEY_THREE))
      state.gameSpeed = 3;

    /* Pause toggle */
    if (IsKeyPressed(KEY_SPACE))
      state.paused = !state.paused;

    /* Advance simulated time when not paused */
    if (!state.paused) {
      state.dayTimer += GetFrameTime() * (float)state.gameSpeed;

      if (state.dayTimer >= state.dayLength) {
        state.dayTimer -= state.dayLength;
        state.day++;
      }
    }

    /* ---- DRAW ---------------------------------------------- */
    BeginDrawing();
    ClearBackground((Color){15, 15, 25, 255});

    DrawText("CureInc", 100, 100, 60, WHITE);

    DrawText(TextFormat("Day: %d", state.day), 100, 180, 28,
             (Color){100, 220, 100, 255});

    DrawText(state.paused ? "PAUSED" : "RUNNING", 100, 220, 22,
             (Color){220, 180, 60, 255});

    DrawText(TextFormat("Speed: x%d", state.gameSpeed), 100, 255, 20,
             (Color){150, 150, 150, 255});

    DrawText("SPACE=Pause   1/2/3=Speed   ESC=Quit", 100, 290, 18,
             (Color){100, 100, 100, 255});

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
