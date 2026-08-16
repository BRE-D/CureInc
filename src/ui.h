#ifndef UI_H
#define UI_H

#include "raylib.h"
#include <stdbool.h>  //for true false.we need boool cuz DrawUIButton returns true false not number
#include "types.h" //for gamescreen enum

typedef enum UIAction { //An enum (short for enumeration) is a feature in C that lets you create a custom variable type with a fixed list of named choices. Under the hood, C assigns whole numbers ($0, 1, 2...$) to each name, but you get to write readable words instead of confusing numbers.
    UI_NONE = 0, //nothing was clicked,automatically equals 0
    UI_START_GAME, //player clicked start,..1
    UI_MAIN_MENU, // .. main,..2
    UI_RESUME, // .. resume
    UI_SPEED_1,
    UI_SPEED_2,
    UI_PAUSE,
    UI_FUND_RESEARCH,
    UI_TOGGLE_BORDERS,//.. close/open borders
    UI_UNLOCK_SKILL,
} UIAction;

typedef struct GameStats {
    float cureProgress;     // 0.0f to 100.0f
    float globalInfection;  // 0.0f to 100.0f
    int   budget;           // in-game currency
    int   dayCount;         // days elapsed
    int   gameSpeed;        // 0 = paused, 1 = normal, 2 = fast
} GameStats;

void InitUI(void); //(Initialization function)


bool DrawUIButton(Rectangle bounds, const char *text, Color baseColor, Color hoverColor);
void DrawUIPanel(Rectangle bounds, Color background, Color border, float borderWidth);
void DrawProgressBar(Rectangle bounds, float percentage, Color barColor, Color bgColor, const char *label);

UIAction UI_DrawMainMenu(GameScreen currentState);
UIAction UI_DrawGameplayHUD(const GameStats *stats);
UIAction UI_DrawPauseOverlay(void);

#endif 