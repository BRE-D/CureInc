#ifndef UI_H
#define UI_H

#include "raylib.h"
#include <stdbool.h>
#include "types.h"

// Button click-এর কাজের নাম; এই enum-এ game data রাখা হয় না।
typedef enum {
    UI_NONE = 0,
    UI_START_GAME,
    UI_MAIN_MENU,
    UI_EXIT,

    UI_RESUME,
    UI_SPEED_1,
    UI_SPEED_2,
    UI_PAUSE,

    UI_HIRE_SCIENTIST,
    UI_UPGRADE_LAB,
    UI_INCREASE_PRODUCTION
} UIAction;

// HUD-তে দেখানোর copy; আসল simulation state নয়।
typedef struct GameStats {
    float cureProgress;
    float globalInfection;
    float globalDeaths; // মোট মৃত্যুর শতাংশ; HUD-তে সব সময় দেখা যাবে।
    int   budget;
    int   dayCount;
    int   gameSpeed;
    float fundingRate;
    float researchRate;
    float stability;
    ResearchPhase curePhase;
} GameStats;

// নির্বাচিত অঞ্চলের UI copy; সংখ্যা মানুষের এককে।
typedef struct RegionData {
    const char *name;
    int   population;
    int   infectedCount;
    float cureResearch;
    bool  bordersClosed;
    bool  isSelected;
} RegionData;

void UI_ResetGameplayState(void); // নতুন খেলায় Lab tab খুলে region panel বন্ধ রাখি।

bool DrawUIButton(Rectangle bounds, const char *text, Color baseColor, Color hoverColor); // Button আঁকি; চালু থাকলে mouse click হয়েছে কি না ফেরত দিই।
void DrawUIPanel(Rectangle bounds, Color background, Color border, float borderWidth); // একটি আয়তাকার panel ও তার border আঁকি।
void DrawProgressBar(Rectangle bounds, float percentage, Color barColor, Color bgColor, const char *label); // শতাংশ অনুযায়ী bar ভরি এবং মাঝখানে লেখা বসাই।

UIAction UI_DrawMainMenu(GameScreen currentState);
UIAction UI_DrawGameplayHUD(const GameStats *stats); // উপরের bar-এ টাকা, দিন, research, মৃত্যু ও speed দেখাই।
UIAction UI_DrawPauseOverlay(void); // Pause-এর পর্দায় শুধু Resume ও Main Menu কাজ করে।
void     UI_DrawRegionPanel(Rectangle bounds, RegionData *region, const Region *source, CureState *cure); // নির্বাচিত অঞ্চলের তথ্য, hospital এবং দুটি action দেখাই।

void     UI_DrawEventLog(const GameState *gs); // সক্রিয় খবরগুলো নিচ থেকে ওপরে দেখাই।
void     UI_DrawGameplay(GameState *gs); // Map, panel ও HUD দেখাই; click-এর কাজ game state-এ প্রয়োগ করি।
void     UI_DrawTransition(GameScreen currentScreen); // Screen বদলালে অল্প সময়ের fade দেখাই।
UIAction UI_DrawEndScreen(const GameState *gs);
UIAction UI_DrawInfoPanel(GameState *gs); // যে tab খোলা আছে শুধু তার ভিতরের তথ্য দেখাই।

#endif
