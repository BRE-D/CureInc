#include "raylib.h"
#include "types.h"
#include "virus.h"

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CureInc");
    SetTargetFPS(60);
 
    GameState state = {0};
    state.screen    = SCREEN_MENU;
    state.dayLength = DEFAULT_DAY_LENGTH;
    state.gameSpeed = 1;

    Virus_Init(&state.virus);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime() * state.gameSpeed;

        if (!state.paused)
        {
            state.dayTimer += dt;

            if (state.dayTimer >= state.dayLength)
            {
                state.dayTimer -= state.dayLength;
                state.day++;

                Virus_Update(&state.virus, 1.0f);
                Virus_TryMutate(&state.virus, 1.0f);
            }
        }

        BeginDrawing();
            ClearBackground((Color){ 15, 15, 25, 255 });

            DrawText("CureInc", 100, 100, 60, WHITE);

            DrawText(TextFormat("Day: %d", state.day),
                     100, 200, 30, WHITE);

            DrawText(TextFormat("Infected: %.2f%%", state.virus.globalInfected * 100.0f),
                     100, 240, 24, RED);

            DrawText(TextFormat("Dead: %.2f%%", state.virus.globalDead * 100.0f),
                     100, 270, 24, GRAY);

            DrawText("Press ESC to exit", 100, 340, 20,
                     (Color){ 150, 150, 150, 255 });
        EndDrawing();
    }

    CloseWindow();
    return 0;
}