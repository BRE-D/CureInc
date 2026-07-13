#include "ui.h"
#include <stdio.h>

void InitUI(void) {
    // Reserved for future UI resources (audio hooks, custom fonts)
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

void DrawMainMenu(AppScreen *currentState) {
    int screenWidth = GetScreenWidth();

    const char *title = "CURE INC.";
    int titleWidth = MeasureText(title, 50);
    DrawText(title, (screenWidth - titleWidth) / 2, 150, 50, DARKBLUE);

    Rectangle startBtn = { (float)(screenWidth - 200) / 2, 300, 200, 50 };
    if (DrawUIButton(startBtn, "START GAME", BLUE, SKYBLUE)) {
        *currentState = STATE_GAMEPLAY;
    }
}

void DrawGameplayHUD(AppScreen *currentState, GameStats *stats) {
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

    Rectangle btn1x = { (float)screenWidth - 240, 15, 40, 30 };
    Rectangle btn2x = { (float)screenWidth - 190, 15, 40, 30 };
    Rectangle btnPause = { (float)screenWidth - 140, 15, 60, 30 };

    if (DrawUIButton(btn1x, "1x", stats->gameSpeed == 1 ? DARKBLUE : GRAY, BLUE)) {
        stats->gameSpeed = 1;
    }
    if (DrawUIButton(btn2x, "2x", stats->gameSpeed == 2 ? DARKBLUE : GRAY, BLUE)) {
        stats->gameSpeed = 2;
    }
    if (DrawUIButton(btnPause, "||", stats->gameSpeed == 0 ? MAROON : GRAY, RED)) {
        stats->gameSpeed = 0;
        *currentState = STATE_PAUSED;
    }
}

void DrawPauseOverlay(AppScreen *currentState) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.5f));

    Rectangle panel = { (float)(screenWidth - 300) / 2, (float)(screenHeight - 200) / 2, 300, 200 };
    DrawUIPanel(panel, RAYWHITE, DARKGRAY, 2.0f);

    DrawText("PAUSED", (int)panel.x + 110, (int)panel.y + 20, 20, BLACK);

    Rectangle resumeBtn = { panel.x + 50, panel.y + 70, 200, 40 };
    if (DrawUIButton(resumeBtn, "RESUME", GREEN, LIME)) {
        *currentState = STATE_GAMEPLAY;
    }

    Rectangle menuBtn = { panel.x + 50, panel.y + 120, 200, 40 };
    if (DrawUIButton(menuBtn, "MAIN MENU", RED, MAROON)) {
        *currentState = STATE_MAIN_MENU;
    }
}

void DrawRegionPanel(Rectangle bounds, RegionData *region, GameStats *stats) {
    if (!region || !region->isSelected) return;

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