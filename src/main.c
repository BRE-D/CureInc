#include "raylib.h"
#include "types.h"

int main(void) {
    // Window setup
    InitWindow(1280, 720, "Cure Inc.");
    SetTargetFPS(60);

    // Initialize game state
    Virus virus = {0.6f, 0.4f, 0.3f, 0.1f};
    Cure cure = {0.0f, 1.0f, 0.0f};
    Resources resources = {1000, 10, 50};

    Region regions[3] = {
        {"Dhaka",    0.1f, 0.8f, 0.3f, 1000000, 100000},
        {"Khulna",   0.05f, 0.9f, 0.2f, 500000,  25000},
        {"Chittagong", 0.08f, 0.7f, 0.4f, 700000,  56000},
    };

    // Main game loop
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        DrawText("Cure Inc. - Running!", 400, 340, 24, GREEN);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}