#include "raylib.h"
#include "ui.h"

int main(void) {
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Cure Inc.");
    SetTargetFPS(60);

    InitUI();
    GameState currentState = STATE_MAIN_MENU;

    // Mock simulation stats for testing UI display
    GameStats stats = {
        .cureProgress = 15.0f,
        .globalInfection = 2.5f,
        .budget = 50000,
        .dayCount = 1,
        .gameSpeed = 1
    };

    float timer = 0.0f;

    while (!WindowShouldClose()) {
        // --- SIMULATION MOCK UPDATE (Simulating time & progress) ---
        if (currentState == STATE_GAMEPLAY && stats.gameSpeed > 0) {
            timer += GetFrameTime() * stats.gameSpeed;
            if (timer >= 1.0f) { // Every 1 second
                timer = 0.0f;
                stats.dayCount++;
                stats.cureProgress += 0.5f;     // Slowly advance cure
                stats.globalInfection += 0.3f; // Slowly advance infection
                stats.budget += 100;

                // Keep stats within valid ranges
                if (stats.cureProgress > 100.0f) stats.cureProgress = 100.0f;
                if (stats.globalInfection > 100.0f) stats.globalInfection = 100.0f;
            }
        }

        // --- DRAWING ---
        BeginDrawing();
            ClearBackground(RAYWHITE);

            switch (currentState) {
                case STATE_MAIN_MENU:
                    DrawMainMenu(&currentState);
                    break;

                case STATE_GAMEPLAY:
                    DrawText("[ Interactive World Map Placeholder ]", screenWidth / 2 - 200, screenHeight / 2, 20, LIGHTGRAY);
                    
                    // Draw HUD on top of map
                    DrawGameplayHUD(&currentState, &stats);
                    break;

                case STATE_PAUSED:
                    DrawText("[ Interactive World Map Placeholder ]", screenWidth / 2 - 200, screenHeight / 2, 20, LIGHTGRAY);
                    
                    // Draw HUD underneath pause
                    DrawGameplayHUD(&currentState, &stats);
                    
                    // Overlay pause menu
                    DrawPauseOverlay(&currentState);
                    break;

                default:
                    break;
            }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}