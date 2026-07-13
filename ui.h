#ifndef UI_H
#define UI_H

#include "raylib.h"

// -----------------------------------------------------------------------------
// UI STATES & GAME DATA
// -----------------------------------------------------------------------------
typedef enum GameState {
    STATE_MAIN_MENU,
    STATE_GAMEPLAY,
    STATE_PAUSED,
    STATE_GAME_OVER
} GameState;

// Simulation data struct passed into UI drawing functions.
// Your teammates will update these values from their simulation system.
typedef struct GameStats {
    float cureProgress;     // Percentage (0.0f to 100.0f)
    float globalInfection;  // Percentage (0.0f to 100.0f)
    int budget;             // Game currency / research points
    int dayCount;           // Number of days elapsed
    int gameSpeed;          // 0 = Paused, 1 = Normal, 2 = Fast
} GameStats;

// -----------------------------------------------------------------------------
// FUNCTION DECLARATIONS
// -----------------------------------------------------------------------------
void InitUI(void);

// Immediate-Mode UI Primitives
bool DrawUIButton(Rectangle bounds, const char *text, Color baseColor, Color hoverColor);
void DrawUIPanel(Rectangle bounds, Color background, Color border, float borderWidth);
void DrawProgressBar(Rectangle bounds, float percentage, Color barColor, Color bgColor, const char *label);

// Screen-level Drawing Functions
void DrawMainMenu(GameState *currentState);
void DrawGameplayHUD(GameState *currentState, GameStats *stats);
void DrawPauseOverlay(GameState *currentState);

#endif // UI_H