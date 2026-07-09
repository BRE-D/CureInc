#include "raylib.h"
#include "types.h"

int main(void)
{
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "CureInc");
    SetTargetFPS(60);

    
    GameState state = {0};
    state.screen    = SCREEN_MENU;
    state.dayLength = DEFAULT_DAY_LENGTH;
    state.gameSpeed = 1;
    (void)state;

    
    while (!WindowShouldClose())
    {
        
        BeginDrawing();
            ClearBackground((Color){ 15, 15, 25, 255 });

            DrawText("CureInc", 100, 100, 60, WHITE);
            DrawText("Basic Screen", 100, 180, 24,
                     (Color){ 100, 220, 100, 255 });

            DrawText("Press ESC to exit", 100, 240, 20,
                     (Color){ 150, 150, 150, 255 });
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
