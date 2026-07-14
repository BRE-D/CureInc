#ifndef UI_H
#define UI_H

#include "raylib.h"
#include <stdbool.h>

// UI STATES & GAME DATA

typedef enum AppScreen {
    STATE_MAIN_MENU,    //STATE_MAIN_MENU (0): Draws the title text and the "Press Enter" splash screen.
    STATE_GAMEPLAY,     //STATE_GAMEPLAY (1): Draws the map interface and updates timers.
    STATE_PAUSED,       //STATE_PAUSED (2): Darkens the screen layout and halts the simulation clock.
    STATE_GAME_OVER     //STATE_GAME_OVER (3): Draws the final victory or defeat popup message.
} AppScreen;

// Global game statistics structure
typedef struct GameStats {
    float cureProgress;     // Percentage (0.0f to 100.0f)
    float globalInfection;  // Percentage (0.0f to 100.0f)
    int budget;             // Game currency / research points
    int dayCount;           // Number of days elapsed
    int gameSpeed;          // 0 = Paused, 1 = Normal, 2 = Fast
} GameStats;

// Region/Country details structure for side panel display
typedef struct RegionData {
    const char *name;       // Region Name (e.g., "North America")
    int population;         // Total population
    int infectedCount;      // Number of infected citizens
    float cureResearch;     // Regional cure research progress %
    bool bordersClosed;     // Status flag: borders open/closed
    bool isSelected;        // Is this region currently open in the UI?
} RegionData;

// -----------------------------------------------------------------------------
// FUNCTION DECLARATIONS
// -----------------------------------------------------------------------------
void InitUI(void);

bool DrawUIButton(Rectangle bounds, const char *text, Color baseColor, Color hoverColor);
void DrawUIPanel(Rectangle bounds, Color background, Color border, float borderWidth);
void DrawProgressBar(Rectangle bounds, float percentage, Color barColor, Color bgColor, const char *label);

void DrawMainMenu(AppScreen *currentState);
void DrawGameplayHUD(AppScreen *currentState, GameStats *stats);
void DrawPauseOverlay(AppScreen *currentState);

void DrawRegionPanel(Rectangle bounds, RegionData *region, GameStats *stats);

#endif // UI_H