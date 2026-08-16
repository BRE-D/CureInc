#ifndef UI_H
#define UI_H

#include "raylib.h"
#include <stdbool.h>
#include "types.h"


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
    const char *name;
    int population;
    int infectedCount;
    float cureResearch;
    bool bordersClosed;
    bool isSelected;
    /*When the player clicks on a region, 
     this flag flips to true on that country,
     signaling the drawing engine to pop open 
     its custom detail menu panel on the screen.
    */
} RegionData;

// FUNCTION DECLARATIONS

void InitUI(void);

bool DrawUIButton(Rectangle bounds, const char *text, Color baseColor, Color hoverColor);
void DrawUIPanel(Rectangle bounds, Color background, Color border, float borderWidth);
void DrawProgressBar(Rectangle bounds, float percentage, Color barColor, Color bgColor, const char *label);

void DrawMainMenu(GameScreen *currentState);
void DrawGameplayHUD(GameScreen *currentState, GameStats *stats);
void DrawPauseOverlay(GameScreen *currentState);

void DrawRegionPanel(Rectangle bounds, RegionData *region, GameStats *stats);

#endif 