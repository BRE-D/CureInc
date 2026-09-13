// Header guard: include these declarations only once per compiled source file.
#ifndef UI_H
#define UI_H

#include "raylib.h"
#include <stdbool.h>
#include "types.h"


// Enum naming a button request. The caller uses it to decide which game action to perform.
typedef enum {
    // No requested action.
    UI_NONE = 0,
    // Start a fresh game.
    UI_START_GAME,
    // Return to the menu.
    UI_MAIN_MENU,
    // Close the game.
    UI_EXIT,

    // Resume the paused game.
    UI_RESUME,
    // Use normal speed.
    UI_SPEED_1,
    // Use double speed.
    UI_SPEED_2,
    // Freeze the simulation.
    UI_PAUSE,

    // Request one scientist purchase.
    UI_HIRE_SCIENTIST,
    // Request the next lab upgrade.
    UI_UPGRADE_LAB,
    // Request the next production upgrade.
    UI_INCREASE_PRODUCTION
} UIAction;


// Struct containing a HUD display snapshot; changing it does not change the simulation.
typedef struct GameStats {
    // HUD percentage across all four cure stages, each given one quarter of the bar.
    float cureProgress;
    // HUD infection percentage of original world population (0..100).
    float globalInfection;
    // HUD cumulative death percentage of original world population (0..100).
    float globalDeaths;
    // Funding converted to a whole-number amount for display.
    int   budget;
    // Game-day number shown in the HUD.
    int   dayCount;
    // HUD speed: 0 for paused, otherwise 1 or 2.
    int   gameSpeed;
    // Current money earned per game day for display.
    float fundingRate;
    // Actual research points per game day after all multipliers.
    float researchRate;
    // Cure stability (0..1); mutations lower it and it scales research/effectiveness.
    float stability;
    // Current phase used to select the HUD label and color.
    ResearchPhase curePhase;
} GameStats;


// Struct containing an editable selected-region UI copy. Population/counts are whole people.
typedef struct RegionData {
    // Read-only region name displayed in cards and details.
    const char *name;
    // Original region population converted to a whole-number person count.
    int   population;
    // Whole-number infected count displayed in the selected region panel.
    int   infectedCount;
    // Local research points (0..100); boost global research and this region healthcare.
    float cureResearch;
    // Whether player-ordered closure is active (1/true closed, 0/false open).
    bool  bordersClosed;
    // Whether the selected-region details panel should be visible.
    bool  isSelected;
} RegionData;

// Reset gameplay UI: close the region panel, choose Lab, restore normal speed backup, and enable buttons.
// Called when starting a new game; returns nothing.
void UI_ResetGameplayState(void);

// Draw a button with centered text and hover color. Return true only for a left click inside an enabled
// button; used by all screens.
// bounds: rectangle position and size in screen pixels.
// text: text to draw or measure.
// baseColor: normal button color.
// hoverColor: button color while the pointer is over it.
bool DrawUIButton(Rectangle bounds, const char *text, Color baseColor, Color hoverColor);
// Draw a panel background, border, subtle shadow, and top accent inside/around its rectangle. Shared
// drawing helper; returns nothing.
// bounds: rectangle position and size in screen pixels.
// background: panel background color.
// border: panel outline color.
// borderWidth: outline thickness in pixels.
void DrawUIPanel(Rectangle bounds, Color background, Color border, float borderWidth);
// Clamp a percentage to 0..100, draw its filled bar, and center a formatted label. Drawing helper only;
// returns nothing.
// bounds: rectangle position and size in screen pixels.
// percentage: bar value from 0 to 100.
// barColor: filled part color.
// bgColor: empty part color.
// label: text displayed on the button or bar.
void DrawProgressBar(Rectangle bounds, float percentage, Color barColor, Color bgColor, const char *label);

// Draw the menu or How to Play page. Return Start, Exit, or no action to main; the Help and Back buttons
// change the local help-page flag.
// currentState: menu API argument; intentionally unused here.
UIAction UI_DrawMainMenu(GameScreen currentState);
// Draw the top resource bars and speed controls using a display snapshot. Return the requested speed/pause
// action; the caller changes game state.
// stats: read-only snapshot of values formatted for the HUD.
UIAction UI_DrawGameplayHUD(const GameStats *stats);
// Draw the dimmed pause dialog. Return Resume, Main Menu, or no action; the gameplay screen applies the
// result.
UIAction UI_DrawPauseOverlay(void);
// Draw selected-region information and hospital load. Handle local research and border purchases using
// global funding; edit the UI region copy for the caller to copy back. Returns nothing.
// bounds: rectangle position and size in screen pixels.
// region: editable UI copy of the selected region; changes are copied back by the caller.
// source: read-only simulation region used to calculate hospital load.
// cure: actual global cure data and funding wallet used for regional purchases.
void     UI_DrawRegionPanel(Rectangle bounds, RegionData *region, const Region *source, CureState *cure);

// Draw up to three active notifications, choosing the largest remaining timers first (newest first except
// ties). Keep all text in the bottom update area; returns nothing.
// gs: shared game state; const means this function only reads it.
void     UI_DrawEventLog(const GameState *gs);
// Draw region cards, guidance, tabs, HUD, and region details; apply clicked purchases and speed actions.
// Disable gameplay buttons while paused and handle the pause overlay. Called once per drawn gameplay
// frame; returns nothing.
// gs: shared game state; const means this function only reads it.
void     UI_DrawGameplay(GameState *gs);
// Start a short black fade whenever the screen changes and reduce opacity using real frame time. Called
// after drawing each screen; returns nothing.
// currentScreen: screen currently being displayed, used to detect a transition.
void     UI_DrawTransition(GameScreen currentScreen);
// Draw victory/defeat statistics, survivor infection percentage, and the day Distribution opened. Return
// Main Menu if clicked, otherwise no action.
// gs: shared game state; const means this function only reads it.
UIAction UI_DrawEndScreen(const GameState *gs);
// Draw Lab/Virus/Cure tabs and the selected body. Return the Lab purchase action, or no action for
// informational tabs.
// gs: shared game state; const means this function only reads it.
UIAction UI_DrawInfoPanel(GameState *gs);

#endif
