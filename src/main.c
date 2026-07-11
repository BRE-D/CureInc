#include "raylib.h"
#include "types.h"
#include "events.h"

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CureInc");
    SetTargetFPS(60);
    SetWindowPosition(0, 0);

    GameState state = {0};
    state.screen = SCREEN_MENU;
    state.dayLength = DEFAULT_DAY_LENGTH;
    state.gameSpeed = 1;

    events_init(&state);
    float eventTimer = 0.0f;//This triggers the next event

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        eventTimer += dt;
        if (eventTimer >= 5.0f)
        {
            events_trigger_random(&state);
            eventTimer = 0.0f;
        }

        events_update(&state, dt);

        BeginDrawing();
            ClearBackground(BLACK);
            DrawText("CureInc", 100, 100, 60, WHITE);
            DrawText("Basic Window", 100, 180, 24, MAROON);
            DrawText("Press ESC to exit", 100, 240, 20, MAROON);

            int ypos = 320;
            for (int i = 0; i < MAX_EVENTS; i++)
            {
                if(state.eventLog[i].active)
                {
                    DrawText(state.eventLog[i].title, 100, ypos, 20, YELLOW);
                    DrawText(state.eventLog[i].description, 100, ypos + 24, 16, YELLOW);
                    ypos += 64;
                }
            }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}