#include "ui.h"
#include <stdio.h>
#include "virus.h"
#include "cure.h"
#include <string.h>

static bool gButtonsEnabled = true;
static bool gRegionPanelOpen   = false;
static int  gPausedSpeedBackup = 1;
static bool gShowHowToPlay = false;
typedef enum { INFO_TAB_LAB = 0, INFO_TAB_VIRUS, INFO_TAB_RESEARCH } InfoTab;
static InfoTab gActiveInfoTab = INFO_TAB_LAB;
static const char *phaseNames[] = {"Discovery", "Trials", "Production", "Distribution"};
#define FADE_DURATION 0.35f
static GameScreen gLastScreen = SCREEN_MENU;
static float      gFadeAlpha  = 0.0f;

// নতুন খেলায় Lab tab খুলে region panel বন্ধ রাখি।
void UI_ResetGameplayState(void) {
    gRegionPanelOpen   = false;
    gPausedSpeedBackup = 1;
    gActiveInfoTab = INFO_TAB_LAB;
    gButtonsEnabled = true;
}

// একটি আয়তাকার panel ও তার border আঁকি।
void DrawUIPanel(Rectangle bounds, Color background, Color border, float borderWidth) {
    // হালকা shadow ও উপরের accent panel-কে আলাদা করে দেখায়।
    DrawRectangle((int)bounds.x + 3, (int)bounds.y + 4, (int)bounds.width, (int)bounds.height, (Color){210,220,230,255});
    DrawRectangleRec(bounds, background);
    DrawRectangleLinesEx(bounds, borderWidth, border);
    DrawRectangle((int)bounds.x, (int)bounds.y, (int)bounds.width, 3, (Color){28,111,139,255});
}

// লেখা বড় হলে font ছোট করি, যেন button-এর বাইরে না যায়।
static int TextSizeToFit(const char *text, int preferred, float width)
{
    while (preferred > 10 && MeasureText(text, preferred) > width)
        preferred--;
    return preferred;
}

// Button আঁকি; চালু থাকলে mouse click হয়েছে কি না ফেরত দিই।
bool DrawUIButton(Rectangle bounds, const char *text, Color baseColor, Color hoverColor) {
    Vector2 mousePos = GetMousePosition();
    // Pause-এ gButtonsEnabled false থাকায় নিচের gameplay button কাজ করে না।
    bool isHovered = gButtonsEnabled && CheckCollisionPointRec(mousePos, bounds);
    Color activeColor = isHovered ? hoverColor : baseColor;
    DrawRectangleRec(bounds, activeColor);
    DrawRectangleLinesEx(bounds, 2.0f, DARKGRAY);

    int fontSize = TextSizeToFit(text, 18, bounds.width - 20.0f);
    int textWidth = MeasureText(text, fontSize);
    float textX = bounds.x + (bounds.width - textWidth) / 2.0f;
    float textY = bounds.y + (bounds.height - fontSize) / 2.0f;
    DrawText(text, (int)textX, (int)textY, fontSize, WHITE);

    return (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT));
}

// কেনা সম্ভব না হলে ম্লান button; global pause protection-ও বজায় থাকে।
static bool DrawPurchase(Rectangle bounds, const char *label, bool canBuy) {
    bool enabled = gButtonsEnabled;
    gButtonsEnabled = enabled && canBuy;
    bool clicked = DrawUIButton(bounds, label, canBuy ? (Color){28,91,124,255} : (Color){139,151,165,255}, DARKBLUE);
    gButtonsEnabled = enabled;
    return clicked;
}

// শতাংশ অনুযায়ী bar ভরি এবং মাঝখানে লেখা বসাই।
void DrawProgressBar(Rectangle bounds, float percentage, Color barColor, Color bgColor, const char *label) {
    if (percentage < 0.0f) percentage = 0.0f;
    if (percentage > 100.0f) percentage = 100.0f;
    DrawRectangleRec(bounds, bgColor);

    float filledWidth = bounds.width * (percentage / 100.0f);
    Rectangle fillArea = { bounds.x, bounds.y, filledWidth, bounds.height };
    DrawRectangleRec(fillArea, barColor);
    DrawRectangleLinesEx(bounds, 1.5f, DARKGRAY);

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%s: %.1f%%", label, percentage);

    int fontSize = 14;
    int textWidth = MeasureText(buffer, fontSize);
    float textX = bounds.x + (bounds.width - textWidth) / 2.0f;
    float textY = bounds.y + (bounds.height - fontSize) / 2.0f;
    DrawText(buffer, (int)textX + 1, (int)textY + 1, fontSize, BLACK);
    DrawText(buffer, (int)textX, (int)textY, fontSize, WHITE);
}

// Menu-র button কোন কাজ চায় তা main.c-কে ফেরত দিই।
UIAction UI_DrawMainMenu(GameScreen currentState) {
    (void)currentState;
    int width = GetScreenWidth();
    DrawRectangle(0, 0, width, 68, (Color){20,40,65,255});
    DrawText("GLOBAL RESPONSE / CURE INC.", 32, 24, 20, WHITE);
    DrawText("A strategy game about protecting people", 32, SCREEN_HEIGHT - 40, 16, DARKGRAY);
    if (gShowHowToPlay) {
        DrawText("HOW TO PLAY", (width - MeasureText("HOW TO PLAY", 42)) / 2, 100, 42, DARKBLUE);
        Rectangle panel = {(float)(width - 700) / 2, 180, 700, 380};
        DrawUIPanel(panel, RAYWHITE, DARKGRAY, 2);
        DrawText("GOAL", (int)panel.x + 30, 210, 24, DARKGREEN);
        DrawText("Develop and distribute a vaccine before humanity collapses.", (int)panel.x + 30, 245, 18, BLACK);
        DrawText("HOW TO PLAY", (int)panel.x + 30, 295, 24, DARKBLUE);
        const char *steps[] = {
            "- Start by hiring scientists to speed up research.",
            "- Then upgrade your lab to finish Discovery and Trials.",
            "-After Trials, vaccines are produced. Distribution starts when the stockpile is ready.",
            "- Upgrade production; pause any time to read and plan.",
            "- Protect 90% of survivors; keep their infections below 5%."
        };
        for (int i = 0; i < 5; i++) // একই drawing code পাঁচবার না লিখে loop।
            DrawText(steps[i], (int)panel.x + 40, 335 + i * 30, 18, BLACK);
        char lossRule[96];
        snprintf(lossRule, sizeof(lossRule), "- Lose at %.0f%% deaths or if all hospitals collapse.", LOSS_DEATH_SHARE * 100);
        DrawText(lossRule, (int)panel.x + 40, 485, 18, MAROON);
        Rectangle back = {(float)(width - 200) / 2, 600, 200, 50};
        if (DrawUIButton(back, "BACK", BLUE, SKYBLUE)) gShowHowToPlay = false;
        return UI_NONE;
    }
    DrawUIPanel((Rectangle){(float)(width - 480) / 2, 95, 480, 460}, RAYWHITE, LIGHTGRAY, 1);
    DrawRectangle(width / 2 - 5, 112, 10, 26, (Color){28,111,139,255});
    DrawRectangle(width / 2 - 13, 120, 26, 10, (Color){28,111,139,255});
    DrawText("CURE INC.", (width - MeasureText("CURE INC.", 50)) / 2, 150, 50, DARKBLUE);
    char mode[80];
    snprintf(mode, sizeof(mode), "CHALLENGE - death limit %.0f%%", LOSS_DEATH_SHARE * 100);
    DrawText(mode, (width - MeasureText(mode, 18)) / 2, 220, 18, MAROON);
    DrawText("Research. Produce. Protect.", (width - MeasureText("Research. Produce. Protect.", 18)) / 2, 256, 18, DARKGRAY);
    Rectangle play = {(float)(width - 220) / 2, 300, 220, 50};
    Rectangle help = {(float)(width - 220) / 2, 370, 220, 50};
    Rectangle quit = {(float)(width - 220) / 2, 440, 220, 50};
    if (DrawUIButton(play, "PLAY", DARKGREEN, GREEN)) return UI_START_GAME;
    if (DrawUIButton(help, "HOW TO PLAY", BLUE, SKYBLUE)) gShowHowToPlay = true;
    if (DrawUIButton(quit, "EXIT", MAROON, RED)) return UI_EXIT;
    return UI_NONE;
}

// উপরের bar-এ টাকা, দিন, research, মৃত্যু ও speed দেখাই।
UIAction UI_DrawGameplayHUD(const GameStats *stats) {
    int screenWidth = GetScreenWidth();

    Rectangle headerBar = { 0, 0, (float)screenWidth, 72 };
    DrawUIPanel(headerBar, RAYWHITE, LIGHTGRAY, 1.0f);

    Rectangle cureBarBounds = { 20, 10, 220, 28 };
    DrawProgressBar(cureBarBounds, stats->cureProgress, BLUE, DARKGRAY, "Cure");

    const char *currentPhase = phaseNames[stats->curePhase];
    Color phaseColors[] = { DARKBLUE, BLUE, SKYBLUE, DARKGREEN };
    Color phaseColor = phaseColors[stats->curePhase];

    Rectangle phaseBadge = { 20, 42, 220, 18 };
    DrawRectangleRec(phaseBadge, Fade(phaseColor, 0.3f));
    DrawRectangleLinesEx(phaseBadge, 1.0f, phaseColor);

    int phaseTextSize = 11;
    char phaseText[32];
    snprintf(phaseText, sizeof(phaseText), "Phase: %s", currentPhase);
    int phaseTextWidth = MeasureText(phaseText, phaseTextSize);
    DrawText(phaseText, (int)(phaseBadge.x + (phaseBadge.width - phaseTextWidth) / 2),
             (int)phaseBadge.y + 3, phaseTextSize, phaseColor);

    Rectangle infectBarBounds = { 260, 10, 220, 28 };
    DrawProgressBar(infectBarBounds, stats->globalInfection, RED, DARKGRAY, "Infected");

    // আক্রান্ত মানে মৃত নয়; মৃত্যু জমতে থাকে, তাই সীমাটি সব সময় দেখাই।
    char deathsText[64];
    snprintf(deathsText, sizeof(deathsText), "Deaths: %.1f%% / %.0f%% limit",
             stats->globalDeaths, LOSS_DEATH_SHARE * 100.0f);
    DrawText(deathsText, 260, 44, TextSizeToFit(deathsText, 14, 220), MAROON);

    char budgetText[32];
    snprintf(budgetText, sizeof(budgetText), "Budget: $%d", stats->budget);
    DrawText(budgetText, 510, 20, 20, DARKGREEN);

    char rateText[64];
    snprintf(rateText, sizeof(rateText), "+$%.1f/day", stats->fundingRate);
    DrawText(rateText, 510, 38, 14, DARKGREEN);

    char dayText[32];
    snprintf(dayText, sizeof(dayText), "Day %d", stats->dayCount);
    DrawText(dayText, 700, 20, 20, BLACK);

    char researchText[64];
    snprintf(researchText, sizeof(researchText), "Research: +%.2f/day", stats->researchRate);
    DrawText(researchText, 850, 20, 16, DARKBLUE);

    char stabilityText[64];
    snprintf(stabilityText, sizeof(stabilityText), "Stability: %.0f%%", stats->stability * 100.0f);
    Color stabColor = stats->stability >= 0.7f ? DARKGREEN : (stats->stability >= 0.5f ? ORANGE : RED);
    DrawText(stabilityText, 850, 38, 16, stabColor);

    Rectangle btn1x    = { (float)screenWidth - 240, 15, 40, 30 };
    Rectangle btn2x    = { (float)screenWidth - 190, 15, 40, 30 };
    Rectangle btnPause = { (float)screenWidth - 140, 15, 60, 30 };

    UIAction action = UI_NONE;
    if (DrawUIButton(btn1x, "1x", stats->gameSpeed == 1 ? DARKBLUE : GRAY, BLUE)) {
        action = UI_SPEED_1;
    }
    if (DrawUIButton(btn2x, "2x", stats->gameSpeed == 2 ? DARKBLUE : GRAY, BLUE)) {
        action = UI_SPEED_2;
    }
    if (DrawUIButton(btnPause, "||", stats->gameSpeed == 0 ? MAROON : GRAY, RED)) {
        action = UI_PAUSE;
    }

    return action;
}

// Pause-এর পর্দায় শুধু Resume ও Main Menu কাজ করে।
UIAction UI_DrawPauseOverlay(void) {
    int screenWidth  = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.5f));

    Rectangle panel = { (float)(screenWidth - 300) / 2, (float)(screenHeight - 200) / 2, 300, 200 };
    DrawUIPanel(panel, RAYWHITE, DARKGRAY, 2.0f);
    DrawText("PAUSED", (int)panel.x + 110, (int)panel.y + 20, 20, BLACK);

    UIAction action = UI_NONE;

    Rectangle resumeBtn = { panel.x + 50, panel.y + 70, 200, 40 };
    if (DrawUIButton(resumeBtn, "RESUME", GREEN, LIME)) {
        action = UI_RESUME;
    }

    Rectangle menuBtn = { panel.x + 50, panel.y + 120, 200, 40 };
    if (DrawUIButton(menuBtn, "MAIN MENU", RED, MAROON)) {
        action = UI_MAIN_MENU;
    }

    return action;
}

// নির্বাচিত অঞ্চলের তথ্য, hospital এবং দুটি action দেখাই।
void UI_DrawRegionPanel(Rectangle bounds, RegionData *region, const Region *source, CureState *cure) {
    if (!region->isSelected) return;
    DrawUIPanel(bounds, RAYWHITE, DARKGRAY, 2.0f);
    DrawText(region->name, (int)bounds.x + 15, (int)bounds.y + 15,
             TextSizeToFit(region->name, 22, bounds.width - 65), DARKBLUE);

    Rectangle closeBtn = { bounds.x + bounds.width - 35, bounds.y + 10, 25, 25 };
    if (DrawUIButton(closeBtn, "X", RED, MAROON)) {
        region->isSelected = false;
    }

    DrawLine((int)bounds.x + 10, (int)bounds.y + 45, (int)(bounds.x + bounds.width - 10), (int)bounds.y + 45, GRAY);

    char popBuf[64];
    snprintf(popBuf, sizeof(popBuf), "Population: %d", region->population);
    DrawText(popBuf, (int)bounds.x + 15, (int)bounds.y + 60, 16, BLACK);

    char infBuf[64];
    snprintf(infBuf, sizeof(infBuf), "Infected: %d", region->infectedCount);
    DrawText(infBuf, (int)bounds.x + 15, (int)bounds.y + 85, 16, RED);

    float infectionPercent = 0.0f;
    if (region->population > 0) {
        infectionPercent = ((float)region->infectedCount / (float)region->population) * 100.0f;
    }
    Rectangle regInfectBar = { bounds.x + 15, bounds.y + 115, bounds.width - 30, 22 };
    DrawProgressBar(regInfectBar, infectionPercent, RED, LIGHTGRAY, "Infection");

    Rectangle regCureBar = { bounds.x + 15, bounds.y + 150, bounds.width - 30, 22 };
    DrawProgressBar(regCureBar, region->cureResearch, BLUE, LIGHTGRAY, "Local Research");

    const char *borderStatus = region->bordersClosed ? "Borders: CLOSED" : "Borders: OPEN";
    Color statusColor = region->bordersClosed ? RED : DARKGREEN;
    DrawText(borderStatus, (int)bounds.x + 15, (int)bounds.y + 190, 16, statusColor);

    float load = virus_hospital_load(source);
    Color loadColor = load > 1.0f ? MAROON : DARKGREEN;
    char hospitalText[80];
    snprintf(hospitalText, sizeof(hospitalText), "Hospital load: %.0f%%", load * 100.0f);
    DrawText(hospitalText, (int)bounds.x + 15, (int)bounds.y + 222, 17, loadColor);
    Rectangle loadTrack = {bounds.x + 15, bounds.y + 246, bounds.width - 30, 12};
    DrawRectangleRec(loadTrack, LIGHTGRAY);
    float fill = load > 1.0f ? 1.0f : load;
    if (fill < 0.0f) fill = 0.0f;
    loadTrack.width *= fill;
    DrawRectangleRec(loadTrack, loadColor);
    if (source->overloadedDays >= 30)
        snprintf(hospitalText, sizeof(hospitalText), "COLLAPSED: %d overloaded days", source->overloadedDays);
    else if (load > 1.0f)
        snprintf(hospitalText, sizeof(hospitalText), "Overloaded: %d / 30 days", source->overloadedDays);
    else
        snprintf(hospitalText, sizeof(hospitalText), "Within hospital capacity");
    DrawText(hospitalText, (int)bounds.x + 15, (int)bounds.y + 270,
             TextSizeToFit(hospitalText, 14, bounds.width - 30), loadColor);

    Rectangle fundBtn = { bounds.x + 15, bounds.y + 300, bounds.width - 30, 35 };
    if (DrawPurchase(fundBtn, "Fund Local Research ($100)", cure->funding >= 100 && region->cureResearch < 100)) {
        if (cure->funding >= 100 && region->cureResearch < 100.0f) {
            cure->funding -= 100;
            region->cureResearch += 15.0f;
            if (region->cureResearch > 100.0f) region->cureResearch = 100.0f;
        }
    }
    DrawText(region->cureResearch >= 100.0f ? "Local research is full." : "Adds up to 15 local research points.",
             (int)bounds.x + 15, (int)bounds.y + 341, 12, DARKGRAY);
    DrawText("Faster research + stronger hospitals.",
             (int)bounds.x + 15, (int)bounds.y + 358, 12, DARKGRAY);

    const char *toggleLabel = region->bordersClosed ? "Reopen Borders" : "Close Borders ($100)";
    Rectangle borderBtn = { bounds.x + 15, bounds.y + 385, bounds.width - 30, 35 };
    if (DrawPurchase(borderBtn, toggleLabel, region->bordersClosed || cure->funding >= 100)) {
        if (!region->bordersClosed && cure->funding >= 100) {
            cure->funding -= 100;
            region->bordersClosed = true;
        } else if (region->bordersClosed) {
            region->bordersClosed = false;
        }
    }
    DrawText("Closing slows spread. Reopening is free.",
             (int)bounds.x + 15, (int)bounds.y + 429,
             TextSizeToFit("Closing slows spread. Reopening is free.", 12, bounds.width - 30), DARKGRAY);
}

// খবরের নাম অনুযায়ী রং বাছি; এটি শুধু দেখানোর জন্য।
static Color EventColor(const char *title) {
    if (strstr(title, "utat") || strstr(title, "utbreak")) return MAROON;
    if (strstr(title, "esearch") || strstr(title, "reakthrough")) return DARKGREEN;
    return DARKBLUE;
}

// সর্বশেষ তিনটি খবর নির্দিষ্ট জায়গায় রাখি, যাতে map বা button ঢাকা না পড়ে।
void UI_DrawEventLog(const GameState *gs) {
    DrawText("RESPONSE UPDATES", 32, 566, 16, DARKBLUE);
    bool shown[MAX_EVENTS] = {0};
    for (int row = 0; row < 3; row++) {
        int newest = -1;
        for (int i = 0; i < MAX_EVENTS; i++)
            if (gs->eventLog[i].active && !shown[i] &&
                (newest < 0 || gs->eventLog[i].timer > gs->eventLog[newest].timer)) newest = i;
        if (newest < 0) {
            if (row == 0) DrawText("World updates appear here. Use Pause to read and plan.", 32, 600, 16, DARKGRAY);
            break;
        }
        shown[newest] = true;
        const Event *event = &gs->eventLog[newest];
        Rectangle box = {20, 592 + row * 52, SCREEN_WIDTH - 40, 44};
        DrawRectangleRec(box, RAYWHITE);
        DrawRectangle(20, (int)box.y, 4, 44, EventColor(event->title));
        char text[256];
        snprintf(text, sizeof(text), "%s: %s", event->title, event->description);
        DrawText(text, 32, (int)box.y + 14, TextSizeToFit(text, 16, box.width - 24), DARKBLUE);
    }
}

// মানচিত্রের রং আক্রান্তের হার বোঝায়; মৃত মানুষের হার নয়।
static Color InfectionColor(float infectedFraction) {
    if (infectedFraction < 0.15f) return DARKGREEN;
    if (infectedFraction < 0.40f) return ORANGE;
    return MAROON;
}

// বর্তমান ধাপ দেখে এখন কী করতে হবে তার ছোট নির্দেশনা দেখাই।
static void DrawObjective(const GameState *gs) {
    Rectangle bounds = {310, 80, 725, 58};
    const char *next;
    char detail[128];
    const CureState *c = &gs->cure;

    if (c->phase == PHASE_DISCOVERY || c->phase == PHASE_TRIALS) {
        next = "Next: hire scientists and upgrade your lab.";
        snprintf(detail, sizeof(detail), "%s: %.0f%% complete. Finish both research phases to unlock vaccines.",
                 c->phase == PHASE_DISCOVERY ? "Discovery" : "Trials", cure_phase_progress(c));
    } else if (c->phase == PHASE_PRODUCTION) {
        next = "Next: upgrade production to build vaccine stock.";
        snprintf(detail, sizeof(detail), "Stockpile goal: %.0f%%. Distribution starts automatically at 100%%.",
                 cure_phase_progress(c));
    } else {
        next = c->globalDistributed >= 0.90f
            ? "Next: keep vaccination running while infections fall."
            : "Next: increase production to vaccinate more people.";
        float living = 1.0f - gs->virus.globalDead;
        float infected = living > 0.0f ? gs->virus.globalInfected / living * 100.0f : 0.0f;
        snprintf(detail, sizeof(detail), "Survivors: %.1f%% vaccinated (goal 90%%); %.1f%% infected (goal below 5%%).",
                 c->globalDistributed * 100.0f, infected);
    }
    DrawUIPanel(bounds, RAYWHITE, DARKBLUE, 1.0f);
    DrawText(next, 320, 88, TextSizeToFit(next, 17, bounds.width - 20), DARKBLUE);
    DrawText(detail, 320, 112, TextSizeToFit(detail, 14, bounds.width - 20), DARKGRAY);
}

// Map, panel ও HUD দেখাই; click-এর কাজ game state-এ প্রয়োগ করি।
void UI_DrawGameplay(GameState *gs) {
    gButtonsEnabled = (gs->screen == SCREEN_GAME);
    DrawObjective(gs);
    DrawText("REGIONAL RESPONSE", 310, 149, 17, DARKBLUE);
    DrawText("Click a region to manage it", 792, 151, 14, DARKGRAY);

    const int   MAP_COLS = 4;
    const float cellW = 170.0f, cellH = 136.0f, gap = 15.0f;
    const float mapX = 310.0f, mapY = 175.0f;

    for (int i = 0; i < MAX_REGIONS; i++) {
        int col = i % MAP_COLS;
        int row = i / MAP_COLS;
        Rectangle cell = { mapX + col * (cellW + gap), mapY + row * (cellH + gap), cellW, cellH };

        const Region *r = &gs->regions[i];
        Color status = InfectionColor(r->infected);
        bool clicked = DrawUIButton(cell, "", RAYWHITE, (Color){216,232,244,255});
        DrawRectangle((int)cell.x, (int)cell.y, (int)cell.width, 5, status);
        DrawText(r->name, (int)cell.x + 10, (int)cell.y + 15,
                 TextSizeToFit(r->name, 17, cell.width - 20), DARKBLUE);
        char text[80];
        snprintf(text, sizeof(text), "Infected  %.1f%%", r->infected * 100);
        DrawText(text, (int)cell.x + 10, (int)cell.y + 43, 14, MAROON);
        snprintf(text, sizeof(text), "Protected %.1f%%", r->vaccinated * 100);
        DrawText(text, (int)cell.x + 10, (int)cell.y + 64, 14, DARKGREEN);
        snprintf(text, sizeof(text), "Deaths    %.1f%%", r->dead * 100);
        DrawText(text, (int)cell.x + 10, (int)cell.y + 85, 14, DARKGRAY);
        DrawText(virus_hospital_load(r) > 1 ? "Hospital overloaded" : "Hospital within capacity",
                 (int)cell.x + 10, (int)cell.y + 111, 10, virus_hospital_load(r) > 1 ? MAROON : DARKGREEN);
        if (i == gs->selectedRegionIndex && gRegionPanelOpen)
            DrawRectangleLinesEx(cell, 3, DARKBLUE);

        if (gs->screen == SCREEN_GAME && clicked) {
            gs->selectedRegionIndex = i;
            gRegionPanelOpen = true;
        }
    }

    // ধাপের নাম ও রং দেখে পুরো cure journey বোঝা যায়।
    for (int i = 0; i < 4; i++) {
        Rectangle stage = {310 + i * 185, 480, 170, 38};
        Color fill = i == (int)gs->cure.phase ? (Color){28,91,124,255} : RAYWHITE;
        DrawRectangleRec(stage, fill);
        DrawText(phaseNames[i], (int)stage.x + 12, 492, 15, i == (int)gs->cure.phase ? WHITE : DARKGRAY);
    }
    DrawText("Infected can recover; deaths are permanent.", 310, 527, 12, DARKGRAY);
    DrawText("Green <15% | Orange 15-<40% | Red 40%+ infected", 310, 544, 12, DARKGRAY);
    DrawUIPanel((Rectangle){20, 440, 260, 105}, RAYWHITE, LIGHTGRAY, 1);
    char goal[64];
    DrawText("PROTECT SURVIVORS", 35, 454, 15, DARKBLUE);
    snprintf(goal, sizeof(goal), "Vaccinated: %.1f%% / 90%%", gs->cure.globalDistributed * 100);
    DrawText(goal, 35, 480, 15, DARKGREEN);
    float living = 1 - gs->virus.globalDead;
    snprintf(goal, sizeof(goal), "Infected: %.1f%% / below 5%%", living > 0 ? gs->virus.globalInfected / living * 100 : 0);
    DrawText(goal, 35, 506, TextSizeToFit(goal, 14, 230), MAROON);
    UI_DrawEventLog(gs);

    UIAction labAction = UI_DrawInfoPanel(gs);

    if (labAction == UI_HIRE_SCIENTIST) {
        cure_hire_scientist(&gs->cure);
    }
    else if (labAction == UI_UPGRADE_LAB) {
        cure_upgrade_lab(&gs->cure);
    }
    else if (labAction == UI_INCREASE_PRODUCTION) {
        cure_upgrade_production(&gs->cure);
    }

    GameStats stats = {0};

    // চারটি ধাপের প্রত্যেকটিকে মোট cure bar-এর ২৫% জায়গা দিই।
    float overallProgress = ((float)gs->cure.phase * 100 + cure_phase_progress(&gs->cure)) * .25f;

    stats.cureProgress    = overallProgress;
    stats.globalInfection = gs->virus.globalInfected * 100.0f;
    stats.globalDeaths = gs->virus.globalDead * 100.0f;
    stats.budget          = (int)gs->cure.funding;
    stats.dayCount        = gs->day;
    stats.gameSpeed       = (gs->screen == SCREEN_PAUSED) ? 0 : gs->gameSpeed;
    stats.fundingRate     = gs->cure.fundingPerTick;
    stats.researchRate    = cure_research_rate(gs);
    stats.stability       = gs->cure.stability;
    stats.curePhase       = gs->cure.phase;

    UIAction hudAction = UI_DrawGameplayHUD(&stats);
    if (hudAction == UI_SPEED_1)      { gs->gameSpeed = 1; gPausedSpeedBackup = 1; }
    else if (hudAction == UI_SPEED_2) { gs->gameSpeed = 2; gPausedSpeedBackup = 2; }
    else if (hudAction == UI_PAUSE)   { gs->screen = SCREEN_PAUSED; }

    Region *sel = &gs->regions[gs->selectedRegionIndex];
    RegionData rd = {0};
    rd.name          = sel->name;
    // Simulation-এ population million এককে, UI-তে মানুষের পূর্ণ সংখ্যা।
    rd.population    = (int)(sel->population * 1000000.0f);
    rd.infectedCount = (int)(sel->infected * rd.population);
    rd.cureResearch  = sel->cureResearch;
    rd.bordersClosed = sel->bordersClosed;
    rd.isSelected    = gRegionPanelOpen;

    Rectangle panel = { (float)SCREEN_WIDTH - 320, 80, 300, 455 };
    if (!gRegionPanelOpen) {
        DrawUIPanel(panel, RAYWHITE, LIGHTGRAY, 1);
        DrawText("REGION DETAILS", (int)panel.x + 20, 105, 20, DARKBLUE);
        DrawText("Select any region card.", (int)panel.x + 20, 154, 18, DARKGRAY);
        DrawText("Check hospital pressure.", (int)panel.x + 20, 196, 16, DARKGRAY);
        DrawText("Fund local research for support.", (int)panel.x + 20, 226, 14, DARKGRAY);
        DrawText("Close borders to slow spread.", (int)panel.x + 20, 256, 14, DARKGRAY);
    }
    gButtonsEnabled = gs->screen == SCREEN_GAME; // Pause-এ তথ্য পড়া যাবে, কেনাকাটা নয়।
    UI_DrawRegionPanel(panel, &rd, sel, &gs->cure);

    gRegionPanelOpen   = rd.isSelected;
    sel->cureResearch  = rd.cureResearch;
    sel->bordersClosed = rd.bordersClosed;

    gButtonsEnabled = true; // এবার শুধু overlay-র button click নিতে পারবে।
    if (gs->screen == SCREEN_PAUSED) {
        UIAction pauseAction = UI_DrawPauseOverlay();
        if (pauseAction == UI_RESUME) {
            gs->screen    = SCREEN_GAME;
            gs->gameSpeed = gPausedSpeedBackup;
        } else if (pauseAction == UI_MAIN_MENU) {
            gs->screen = SCREEN_MENU;
        }
    }
}

// খেলা শেষে জয়/হার, মৃত্যুর সীমা এবং শেষের হিসাব দেখাই।
UIAction UI_DrawEndScreen(const GameState *gs) {
    bool won = gs->screen == SCREEN_WIN;
    const char *title = won ? "VICTORY" : "DEFEAT";
    const char *subtitle = won ? "Humanity has contained the outbreak." : "Humanity could not contain the outbreak.";
    DrawText(title, (SCREEN_WIDTH - MeasureText(title, 48)) / 2, 100, 48, won ? DARKGREEN : RED);
    DrawText(subtitle, (SCREEN_WIDTH - MeasureText(subtitle, 22)) / 2, 165, 22, DARKGRAY);
    Rectangle panel = {(float)(SCREEN_WIDTH - 600) / 2, 220, 600, 340};
    DrawUIPanel(panel, RAYWHITE, DARKGRAY, 2);
    char text[128];
    int x = (int)panel.x + 50;
    snprintf(text, sizeof(text), "Days Survived: %d", gs->day);
    DrawText(text, x, 255, 22, BLACK);
    snprintf(text, sizeof(text), "Total Deaths: %.1f%% (limit %.0f%%)", gs->virus.globalDead * 100, LOSS_DEATH_SHARE * 100);
    DrawText(text, x, 300, TextSizeToFit(text, 22, 500), RED);
    snprintf(text, sizeof(text), "Global Vaccinated: %.1f%%", gs->cure.globalDistributed * 100);
    DrawText(text, x, 345, 22, DARKGREEN);
    
    float living = 1.0f - gs->virus.globalDead;
    float finalInfection =living > 0.0f? (gs->virus.globalInfected / living) * 100.0f: 0.0f;

    snprintf(text, sizeof(text), "Final Infection: %.1f%%", finalInfection);
    DrawText(text, x, 390, 22, MAROON);
    if (gs->cure.completionDay > 0)
        snprintf(text, sizeof(text), "Distribution Started: Day %d", gs->cure.completionDay);
    else
        snprintf(text, sizeof(text), "Distribution Started: No");
    DrawText(text, x, 435, 22, DARKBLUE);
    if (gs->endReason != NULL)
        DrawText(gs->endReason, (SCREEN_WIDTH - MeasureText(gs->endReason, 16)) / 2, 505, 16, DARKGRAY);
    Rectangle menu = {(float)(SCREEN_WIDTH - 220) / 2, 600, 220, 50};
    if (DrawUIButton(menu, "MAIN MENU", BLUE, SKYBLUE)) return UI_MAIN_MENU;
    return UI_NONE;
}

// Screen বদলালে অল্প সময়ের fade দেখাই।
void UI_DrawTransition(GameScreen currentScreen) {
    if (currentScreen != gLastScreen) {
        gFadeAlpha  = 1.0f;
        gLastScreen = currentScreen;
    }

    if (gFadeAlpha > 0.0f) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, gFadeAlpha));
        gFadeAlpha -= GetFrameTime() / FADE_DURATION;
        if (gFadeAlpha < 0.0f) gFadeAlpha = 0.0f;
    }
}

// কেনার লাভ দেখাতে game state-এর copy-তে upgrade দিয়ে rate-এর পার্থক্য বের করি।
static UIAction DrawLabBody(Rectangle area, const GameState *gs) {
    const CureState *c = &gs->cure;
    UIAction action = UI_NONE;
    float y = area.y;
    char buf[96];
    snprintf(buf, sizeof(buf), "Scientists: %d", c->scientistCount);
    DrawText(buf, (int)area.x, (int)y, 14, DARKGRAY);
    snprintf(buf, sizeof(buf), "Lab Level: %d/3", c->labLevel);
    DrawText(buf, (int)area.x, (int)y + 18, 14, DARKGRAY);
    snprintf(buf, sizeof(buf), "Production Level: %d/3", c->productionLevel);
    DrawText(buf, (int)area.x, (int)y + 36, 14, DARKGRAY);
    snprintf(buf, sizeof(buf), "Vaccine Stock: %.1f", c->vaccineStockpile);
    DrawText(buf, (int)area.x, (int)y + 54, 14, DARKGRAY);
    y += 80;

    Rectangle hireBtn = { area.x, y, area.width, 32 };
    Rectangle labBtn  = { area.x, y + 66, area.width, 32 };
    Rectangle prodBtn = { area.x, y + 132, area.width, 32 };
    float currentRate = cure_research_rate(gs); // কেনার আগের rate।
    GameState preview = *gs; // শুধু copy বদলাই; player-এর টাকা এখানে কাটে না।
    preview.cure.scientistCount++;
    snprintf(buf, sizeof(buf), "Hire Scientist ($%.0f)", SCIENTIST_COST);
    if (DrawPurchase(hireBtn, buf, c->funding >= SCIENTIST_COST))
        action = UI_HIRE_SCIENTIST;
    snprintf(buf, sizeof(buf), "+%.2f research/day now",
             cure_research_rate(&preview) - currentRate);
    DrawText(buf, (int)area.x, (int)y + 35, 13, DARKGRAY);
    snprintf(buf, sizeof(buf), "+%.1f vaccine units/day",
             cure_production_rate(&preview.cure) - cure_production_rate(c));
    DrawText(buf, (int)area.x, (int)y + 50, 13, DARKGRAY);

    if (c->labLevel >= 3) snprintf(buf, sizeof(buf), "Lab Maxed");
    else snprintf(buf, sizeof(buf), "Upgrade Lab ($%.0f)", LAB_BASE_COST * (c->labLevel + 1));
    if (DrawPurchase(labBtn, buf, c->phase < PHASE_PRODUCTION && c->labLevel < 3 && c->funding >= LAB_BASE_COST * (c->labLevel + 1)))
        action = UI_UPGRADE_LAB;
    preview.cure = *c;
    if (c->labLevel < 3) preview.cure.labLevel++;
    snprintf(buf, sizeof(buf), "+%.2f research/day now",
             cure_research_rate(&preview) - currentRate);
    if (c->phase >= PHASE_PRODUCTION)
        snprintf(buf, sizeof(buf), "Research is already complete.");
    else if (c->labLevel >= 3)
        snprintf(buf, sizeof(buf), "Maximum lab level reached.");
    DrawText(buf, (int)area.x, (int)y + 101,
             TextSizeToFit(buf, 13, area.width), DARKGRAY);

    if (c->productionLevel >= 3) snprintf(buf, sizeof(buf), "Production Maxed");
    else snprintf(buf, sizeof(buf), "Upgrade Production ($%.0f)", PRODUCTION_BASE_COST * (c->productionLevel + 1));
    if (DrawPurchase(prodBtn, buf, c->productionLevel < 3 && c->funding >= PRODUCTION_BASE_COST * (c->productionLevel + 1)))
        action = UI_INCREASE_PRODUCTION;
    preview.cure = *c;
    if (c->productionLevel < 3) preview.cure.productionLevel++;
    snprintf(buf, sizeof(buf), "+%.1f vaccine units/day",
             cure_production_rate(&preview.cure) - cure_production_rate(c));
    if (c->productionLevel >= 3)
        snprintf(buf, sizeof(buf), "Maximum production reached.");
    DrawText(buf, (int)area.x, (int)y + 167,
             TextSizeToFit(buf, 13, area.width), DARKGRAY);
    DrawText("Vaccine output starts after Trials.", (int)area.x, (int)y + 193,
             TextSizeToFit("Vaccine output starts after Trials.", 12, area.width), DARKGRAY);
    return action;
}

// Virus-এর হার এবং চালু mutation traits দেখাই।
static void DrawVirusBody(Rectangle area, const Virus *v) {
    float y = area.y;

    // এগুলো শক্তি ও মূল দৈনিক হার; আক্রান্ত মানুষের শতাংশ নয়।
    char buf[64];
    snprintf(buf, sizeof(buf), "Spread strength: %.3f", v->infectivity);
    DrawText(buf, (int)area.x, (int)y, 14, DARKBLUE);
    y += 26;
    snprintf(buf, sizeof(buf), "Base deaths/day: %.2f%%", v->severity * 100);
    DrawText(buf, (int)area.x, (int)y, 14, MAROON);
    y += 26;

    Rectangle infectedBar = { area.x, y, area.width, 20 };
    DrawProgressBar(infectedBar, v->globalInfected * 100.0f, ORANGE, LIGHTGRAY, "Infected");
    y += 26;

    Rectangle deadBar = { area.x, y, area.width, 20 };
    DrawProgressBar(deadBar, v->globalDead * 100.0f, BLACK, LIGHTGRAY, "Global Deaths");
    y += 30;

    snprintf(buf, sizeof(buf), "Resistance: %.0f%%", v->resistance * 100.0f);
    DrawText(buf, (int)area.x, (int)y, 14, DARKGRAY);
    y += 22;

    MutationTrait allTraits[] = {
        TRAIT_AIRBORNE, TRAIT_DRUG_RESISTANT, TRAIT_STEALTH, TRAIT_LETHAL,
        TRAIT_FAST_SPREAD, TRAIT_COLD_ADAPTED, TRAIT_HOT_ADAPTED, TRAIT_LONG_INCUBATION
    };
    DrawText("Active traits:", (int)area.x, (int)y, 14, DARKGRAY);
    y += 20;
    int shown = 0;
    for (int i = 0; i < 8; i++) {
        if (!virus_has_trait(v, allTraits[i])) continue;
        const char *name = virus_trait_name(allTraits[i]);
        float columnWidth = area.width / 2.0f;
        DrawText(name, (int)(area.x + (shown % 2) * columnWidth),
                 (int)(y + (shown / 2) * 19),
                 TextSizeToFit(name, 12, columnWidth - 6), DARKGRAY);
        shown++;
    }
    if (shown == 0) DrawText("None yet", (int)area.x, (int)y, 12, DARKGRAY);
}

// Cure-এর ধাপ, progress, কার্যকারিতা ও উৎপাদন দেখাই।
static void DrawResearchBody(Rectangle area, const CureState *c) {
    
    float y = area.y;

    char buf[64];
    snprintf(buf, sizeof(buf), "Phase: %s", phaseNames[c->phase]);
    DrawText(buf, (int)area.x, (int)y, 16, DARKBLUE);
    y += 24;

    Rectangle progBar = { area.x, y, area.width, 20 };
    const char *progressLabel = c->phase == PHASE_PRODUCTION ? "Stockpile goal" :
        c->phase == PHASE_DISTRIBUTION ? "Vaccinated" : "Phase Progress";
    DrawProgressBar(progBar, cure_phase_progress(c), BLUE, LIGHTGRAY, progressLabel);
    y += 26;

    Rectangle distBar = { area.x, y, area.width, 20 };
    if (c->phase != PHASE_DISTRIBUTION)
        DrawProgressBar(distBar, c->globalDistributed * 100.0f, DARKGREEN, LIGHTGRAY, "Vaccinated");
    else
        DrawText("Goal: 90% of living people", (int)area.x, (int)y, 14, DARKGREEN);
    y += 30;
    snprintf(buf, sizeof(buf), "Stability: %.0f%%", c->stability * 100.0f);
    DrawText(buf, (int)area.x, (int)y, 14, DARKGRAY);
    y += 20;
    snprintf(buf, sizeof(buf), "Effectiveness: %.0f%%", c->effectiveness * 100.0f);
    DrawText(buf, (int)area.x, (int)y, 14, DARKGRAY);
    y += 20;
    snprintf(buf, sizeof(buf), "Capacity: %.1f units/day", c->productionRate);
    DrawText(buf, (int)area.x, (int)y, 14, DARKGRAY);
    y += 22;
    DrawText("1 unit = doses for 1% of", (int)area.x, (int)y, 12, DARKGRAY);
    DrawText("the original world population.", (int)area.x, (int)y + 16, 12, DARKGRAY);
}

// যে tab খোলা আছে শুধু তার ভিতরের তথ্য দেখাই।
UIAction UI_DrawInfoPanel(GameState *gs) {
    Rectangle bounds = { 20, 80, 260, 345 };
    DrawUIPanel(bounds, RAYWHITE, DARKGRAY, 2.0f);

    float tabWidth = (bounds.width - 10) / 3.0f;
    Rectangle labTab      = { bounds.x + 5,                bounds.y + 8, tabWidth, 26 };
    Rectangle virusTab    = { bounds.x + 5 + tabWidth,     bounds.y + 8, tabWidth, 26 };
    Rectangle researchTab = { bounds.x + 5 + tabWidth * 2, bounds.y + 8, tabWidth, 26 };

    Color labColor      = (gActiveInfoTab == INFO_TAB_LAB)      ? DARKBLUE  : GRAY;
    Color virusColor    = (gActiveInfoTab == INFO_TAB_VIRUS)    ? MAROON    : GRAY;
    Color researchColor = (gActiveInfoTab == INFO_TAB_RESEARCH) ? DARKGREEN : GRAY;
    if (DrawUIButton(labTab, "Lab", labColor, SKYBLUE))          gActiveInfoTab = INFO_TAB_LAB;
    if (DrawUIButton(virusTab, "Virus", virusColor, RED))        gActiveInfoTab = INFO_TAB_VIRUS;
    if (DrawUIButton(researchTab, "Cure", researchColor, GREEN)) gActiveInfoTab = INFO_TAB_RESEARCH;

    Rectangle bodyArea = { bounds.x + 15, bounds.y + 45, bounds.width - 30, bounds.height - 55 };

    UIAction action = UI_NONE;
    switch (gActiveInfoTab) {
        case INFO_TAB_LAB:      action = DrawLabBody(bodyArea, gs); break;
        case INFO_TAB_VIRUS:    DrawVirusBody(bodyArea, &gs->virus);       break;
        case INFO_TAB_RESEARCH: DrawResearchBody(bodyArea, &gs->cure);     break;
    }

    return action;
}
