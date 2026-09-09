#include "ui.h"
#include <stdio.h>

static bool gRegionPanelOpen   = false;
static int  gPausedSpeedBackup = 1;

void InitUI(void) {
    // Reserved for future UI resources (fonts, sounds)
}

void UI_ResetGameplayState(void) {
    gRegionPanelOpen   = false;
    gPausedSpeedBackup = 1;
}

void DrawUIPanel(Rectangle bounds, Color background, Color border, float borderWidth) {
    DrawRectangleRec(bounds, background);
    DrawRectangleLinesEx(bounds, borderWidth, border);
}

bool DrawUIButton(Rectangle bounds, const char *text, Color baseColor, Color hoverColor) {
    Vector2 mousePos = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePos, bounds);
    Color activeColor = isHovered ? hoverColor : baseColor;

    DrawRectangleRec(bounds, activeColor);
    DrawRectangleLinesEx(bounds, 2.0f, DARKGRAY);

    int fontSize = 18;
    int textWidth = MeasureText(text, fontSize);
    float textX = bounds.x + (bounds.width - textWidth) / 2.0f;
    float textY = bounds.y + (bounds.height - fontSize) / 2.0f;

    DrawText(text, (int)textX, (int)textY, fontSize, WHITE);

    return (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT));
}

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

// Day 1: screen-level widgets — return intent, never mutate//
UIAction UI_DrawMainMenu(GameScreen currentState) {
    (void)currentState; // not used yet — kept for future menu logic

    int screenWidth = GetScreenWidth();

    const char *title = "CURE INC.";
    int titleWidth = MeasureText(title, 50);
    DrawText(title, (screenWidth - titleWidth) / 2, 150, 50, DARKBLUE);

    Rectangle startBtn = { (float)(screenWidth - 200) / 2, 300, 200, 50 };
    if (DrawUIButton(startBtn, "START GAME", BLUE, SKYBLUE)) {
        return UI_START_GAME;
    }

    return UI_NONE;
}

UIAction UI_DrawGameplayHUD(const GameStats *stats) {
    int screenWidth = GetScreenWidth();

    Rectangle headerBar = { 0, 0, (float)screenWidth, 60 };
    DrawUIPanel(headerBar, LIGHTGRAY, GRAY, 2.0f);

    Rectangle cureBarBounds = { 20, 15, 220, 30 };
    DrawProgressBar(cureBarBounds, stats->cureProgress, BLUE, DARKGRAY, "Cure");

    Rectangle infectBarBounds = { 260, 15, 220, 30 };
    DrawProgressBar(infectBarBounds, stats->globalInfection, RED, DARKGRAY, "Infected");

    char budgetText[32];
    snprintf(budgetText, sizeof(budgetText), "Budget: $%d", stats->budget);
    DrawText(budgetText, 510, 20, 20, DARKGREEN);

    char dayText[32];
    snprintf(dayText, sizeof(dayText), "Day %d", stats->dayCount);
    DrawText(dayText, 700, 20, 20, BLACK);

    Rectangle btn1x    = { (float)screenWidth - 240, 15, 40, 30 }; //bujhtehobe
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

void UI_DrawRegionPanel(Rectangle bounds, RegionData *region, GameStats *stats) {
    if (!region->isSelected) return;

    DrawUIPanel(bounds, RAYWHITE, DARKGRAY, 2.0f);
    DrawText(region->name, (int)bounds.x + 15, (int)bounds.y + 15, 22, DARKBLUE);

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

    Rectangle fundBtn = { bounds.x + 15, bounds.y + 230, bounds.width - 30, 35 };
    if (DrawUIButton(fundBtn, "Fund Research ($500)", DARKGREEN, GREEN)) {
        if (stats->budget >= 500) {
            stats->budget -= 500;
            region->cureResearch += 5.0f;
            if (region->cureResearch > 100.0f) region->cureResearch = 100.0f;
        }
    }

    const char *toggleLabel = region->bordersClosed ? "Reopen Borders" : "Close Borders ($200)";
    Rectangle borderBtn = { bounds.x + 15, bounds.y + 275, bounds.width - 30, 35 };
    if (DrawUIButton(borderBtn, toggleLabel, MAROON, RED)) {
        if (!region->bordersClosed && stats->budget >= 200) {
            stats->budget -= 200;
            region->bordersClosed = true;
        } else if (region->bordersClosed) {
            region->bordersClosed = false;
        }
    }
}

//full-screen coordinators//
void UI_DrawEventLog(const GameState *gs) {
    int y = SCREEN_HEIGHT - 80;
    for (int i = 0; i < MAX_EVENTS; i++) {
        if (!gs->eventLog[i].active) continue;

        Rectangle box = { 20, (float)y, 1000, 46 };
        DrawRectangleRec(box, Fade(DARKBLUE, 0.85f));
        DrawRectangleLinesEx(box, 1.5f, BLUE);

        char text[256];
        snprintf(text, sizeof(text), "[!] %s: %s", gs->eventLog[i].title, gs->eventLog[i].description);
        DrawText(text, 28, y + 13, 20, WHITE);
        y -= 54;
    }
}

void UI_DrawGameplay(GameState *gs, Rectangle regionNode) {
    DrawText("[ Interactive World Map Placeholder ]", 350, 200, 20, LIGHTGRAY);

    Region *sel = &gs->regions[gs->selectedRegionIndex];
    bool hovered = CheckCollisionPointRec(GetMousePosition(), regionNode);

    DrawRectangleRec(regionNode, hovered ? SKYBLUE : BLUE);
    DrawRectangleLinesEx(regionNode, 2, DARKBLUE);
    DrawText(sel->name, (int)regionNode.x + 10, (int)regionNode.y + 10, 18, WHITE);

    if (gs->screen == SCREEN_GAME) {
        if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) gRegionPanelOpen = true;
        if (IsKeyPressed(KEY_TAB)) gs->selectedRegionIndex = (gs->selectedRegionIndex + 1) % MAX_REGIONS;
    }

    UI_DrawEventLog(gs);

    GameStats stats = {0};
    stats.cureProgress    = gs->cure.researchProgress;
    stats.globalInfection = gs->virus.globalInfected * 100.0f;
    stats.budget          = (int)gs->cure.funding;
    stats.dayCount        = gs->day;
    stats.gameSpeed       = (gs->screen == SCREEN_PAUSED) ? 0 : gs->gameSpeed;

    UIAction hudAction = UI_DrawGameplayHUD(&stats);
    if (hudAction == UI_SPEED_1)      { gs->gameSpeed = 1; gPausedSpeedBackup = 1; }
    else if (hudAction == UI_SPEED_2) { gs->gameSpeed = 2; gPausedSpeedBackup = 2; }
    else if (hudAction == UI_PAUSE)   { gs->screen = SCREEN_PAUSED; }

    RegionData rd = {0};
    rd.name          = sel->name;
    rd.population    = (int)(sel->population * 1000000.0f);
    rd.infectedCount = (int)(sel->infected * rd.population);
    rd.cureResearch  = sel->cureResearch;
    rd.bordersClosed = sel->bordersClosed;
    rd.isSelected    = gRegionPanelOpen;

    Rectangle panel = { (float)SCREEN_WIDTH - 320, 70, 300, 350 };
    UI_DrawRegionPanel(panel, &rd, &stats);

    gRegionPanelOpen   = rd.isSelected;
    sel->cureResearch  = rd.cureResearch;
    sel->bordersClosed = rd.bordersClosed;
    gs->cure.funding   = (float)stats.budget;

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

UIAction UI_DrawEndScreen(GameScreen screen) {
    bool won = (screen == SCREEN_WIN);

    const char *message  = won ? "CURE DISTRIBUTED" : "HUMANITY HAS FALLEN";
    Color       msgColor = won ? DARKGREEN : RED;
    int         fontSize = won ? 34 : 40;

    int textWidth = MeasureText(message, fontSize);
    DrawText(message, (SCREEN_WIDTH - textWidth) / 2, 320, fontSize, msgColor);

    Rectangle menuBtn = { (float)(SCREEN_WIDTH - 200) / 2, 420, 200, 50 };
    if (DrawUIButton(menuBtn, "MAIN MENU", BLUE, SKYBLUE)) {
        return UI_MAIN_MENU;
    }

    return UI_NONE;
}