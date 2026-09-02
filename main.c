#include "raylib.h"
#include "ui.h"

int main(void) {
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Cure Inc.");
    SetTargetFPS(60);

    InitUI();
    GameState currentState = STATE_MAIN_MENU;

    // Global Stats Mock
    GameStats stats = {
        .cureProgress = 15.0f,
        .globalInfection = 5.0f,
        .budget = 5000,
        .dayCount = 1,
        .gameSpeed = 1
    };

    // Sample Region Mock Data
    RegionData activeRegion = {
        .name = "North America",
        .population = 500000,
        .infectedCount = 25000,
        .cureResearch = 12.0f,
        .bordersClosed = false,
        .isSelected = false
    };

    // Interactive clickable node representing region on world map placeholder
    Rectangle regionClickTarget = { 400, 300, 120, 40 };

    float timer = 0.0f;

    while (!WindowShouldClose()) {
        // --- SIMULATION TICK ---
        if (currentState == STATE_GAMEPLAY && stats.gameSpeed > 0) {
            timer += GetFrameTime() * stats.gameSpeed;
            if (timer >= 1.0f) {
                timer = 0.0f;
                stats.dayCount++;
                stats.cureProgress += 0.2f;

                // Slowly increase infection if borders open
                if (!activeRegion.bordersClosed && activeRegion.infectedCount < activeRegion.population) {
                    activeRegion.infectedCount += 500;
                }
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
                case STATE_PAUSED:
                    // 1. World Map Visual Placeholder
                    DrawText("[ Interactive World Map Placeholder ]", 350, 200, 20, LIGHTGRAY);

                    // 2. Clickable Region Target (Interactive Node)
                    bool hovered = CheckCollisionPointRec(GetMousePosition(), regionClickTarget);
                    DrawRectangleRec(regionClickTarget, hovered ? SKYBLUE : BLUE);
                    DrawRectangleLinesEx(regionClickTarget, 2, DARKBLUE);
                    DrawText("N. America", (int)regionClickTarget.x + 10, (int)regionClickTarget.y + 10, 18, WHITE);

                    // Click detection on region node
                    if (currentState == STATE_GAMEPLAY && hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        activeRegion.isSelected = true; // Open side panel
                    }

                    // 3. Draw Top Header HUD
                    DrawGameplayHUD(&currentState, &stats);

                    // 4. Draw Region Side Panel (if active)
                    Rectangle sidePanelBounds = { (float)screenWidth - 320, 70, 300, 350 };
                    DrawRegionPanel(sidePanelBounds, &activeRegion, &stats);

                    // 5. Draw Pause Overlay if paused
                    if (currentState == STATE_PAUSED) {
                        DrawPauseOverlay(&currentState);
                    }
                    break;

                default:
                    break;
            }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}