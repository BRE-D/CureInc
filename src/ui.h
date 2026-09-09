#ifndef UI_H
#define UI_H

#include "raylib.h"
#include <stdbool.h>
#include "types.h"

typedef enum { //etar step onujayi shb chole
    UI_NONE = 0,
    UI_START_GAME,
    UI_MAIN_MENU,
    UI_RESUME,
    UI_SPEED_1,
    UI_SPEED_2,
    UI_PAUSE,
    UI_FUND_RESEARCH,
    UI_TOGGLE_BORDERS,
    UI_UNLOCK_SKILL
} UIAction;

typedef struct GameStats {
    float cureProgress;     // 0.0f to 100.0f
    float globalInfection;  // 0.0f to 100.0f
    int   budget;           // in-game currency
    int   dayCount;         // days elapsed
    int   gameSpeed;        // 0 = paused, 1 = normal, 2 = fast
} GameStats;

typedef struct RegionData {
    const char *name;
    int   population;
    int   infectedCount;
    float cureResearch;
    bool  bordersClosed;
    bool  isSelected;
} RegionData;

// Setup / lifecycle
void InitUI(void);
void UI_ResetGameplayState(void);

// Immediate-mode primitives
bool DrawUIButton(Rectangle bounds, const char *text, Color baseColor, Color hoverColor);
void DrawUIPanel(Rectangle bounds, Color background, Color border, float borderWidth);
void DrawProgressBar(Rectangle bounds, float percentage, Color barColor, Color bgColor, const char *label);

// Screen-level widgets (Day 1)
UIAction UI_DrawMainMenu(GameScreen currentState);
UIAction UI_DrawGameplayHUD(const GameStats *stats);
UIAction UI_DrawPauseOverlay(void);
void     UI_DrawRegionPanel(Rectangle bounds, RegionData *region, GameStats *stats);

// Full-screen coordinators (Day 2)
void     UI_DrawEventLog(const GameState *gs);
void     UI_DrawGameplay(GameState *gs, Rectangle regionNode);
UIAction UI_DrawEndScreen(GameScreen screen);

#endif