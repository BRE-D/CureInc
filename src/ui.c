define UI_H

#include "raylib.h"
#include <stdbool.h>
#include "types.h"
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


// Global game statistics structure
typedef struct GameStats {
    float cureProgress;     // Percentage (0.0f to 100.0f)
    float globalInfection;  // Percentage (0.0f to 100.0f)
    int budget;             // Game currency / research points
    int dayCount;           // Number of days elapsed
    int gameSpeed;          // 0 = Paused, 1 = Normal, 2 = Fast
    float cureProgress;     // 0.0f to 100.0f
    float globalInfection;  // 0.0f to 100.0f
    int   budget;           // in-game currency
    int   dayCount;         // days elapsed
    int   gameSpeed;        // 0 = paused, 1 = normal, 2 = fast
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
void InitUI(void); //(Initialization function)


bool DrawUIButton(Rectangle bounds, const char *text, Color baseColor, Color hoverColor);
void DrawUIPanel(Rectangle bounds, Color background, Color border, float borderWidth);
void DrawProgressBar(Rectangle bounds, float percentage, Color barColor, Color bgColor, const char *label);

void DrawMainMenu(GameScreen *currentState);
void DrawGameplayHUD(GameScreen *currentState, GameStats *stats);
void DrawPauseOverlay(GameScreen *currentState);

void DrawRegionPanel(Rectangle bounds, RegionData *region, GameStats *stats);
UIAction UI_DrawMainMenu(GameScreen currentState);
UIAction UI_DrawGameplayHUD(const GameStats *stats);
UIAction UI_DrawPauseOverlay(void);

#endif 