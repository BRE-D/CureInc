#include "ui.h"
#include <stdio.h>
#include "virus.h"
#include "cure.h"
#include <string.h>

static bool gRegionPanelOpen   = false;
static int  gPausedSpeedBackup = 1;
static bool gShowHowToPlay = false;
typedef enum { INFO_TAB_LAB = 0, INFO_TAB_VIRUS, INFO_TAB_RESEARCH } InfoTab;
static InfoTab gActiveInfoTab = INFO_TAB_LAB;

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
UIAction UI_DrawMainMenu(GameScreen currentState)
{
    (void)currentState;

    int screenWidth = GetScreenWidth();

    /* HOW TO PLAY SCREEN */
    if (gShowHowToPlay)
    {
        const char *title = "HOW TO PLAY";

        int titleWidth = MeasureText(title, 42);

        DrawText(
            title,
            (screenWidth - titleWidth) / 2,
            100,
            42,
            DARKBLUE
        );


        Rectangle panel = {
            (float)(screenWidth - 700) / 2,
            180,
            700,
            380
        };

        DrawUIPanel(
            panel,
            RAYWHITE,
            DARKGRAY,
            2.0f
        );


        DrawText(
            "GOAL",
            (int)panel.x + 30,
            (int)panel.y + 30,
            24,
            DARKGREEN
        );

        DrawText(
            "Develop and distribute a vaccine before humanity collapses.",
            (int)panel.x + 30,
            (int)panel.y + 65,
            18,
            BLACK
        );


        DrawText(
            "HOW TO PLAY",
            (int)panel.x + 30,
            (int)panel.y + 115,
            24,
            DARKBLUE
        );


        DrawText(
            "- Hire scientists to increase research speed.",
            (int)panel.x + 40,
            (int)panel.y + 155,
            18,
            BLACK
        );

        DrawText(
            "- Upgrade your laboratory and vaccine production.",
            (int)panel.x + 40,
            (int)panel.y + 185,
            18,
            BLACK
        );

        DrawText(
            "- Monitor infections, deaths and healthcare capacity.",
            (int)panel.x + 40,
            (int)panel.y + 215,
            18,
            BLACK
        );

        DrawText(
            "- React to mutations and random world events.",
            (int)panel.x + 40,
            (int)panel.y + 245,
            18,
            BLACK
        );

        DrawText(
            "- Reach 90% vaccination and reduce infection below 5%.",
            (int)panel.x + 40,
            (int)panel.y + 275,
            18,
            BLACK
        );


        Rectangle backBtn = {
            (float)(screenWidth - 200) / 2,
            600,
            200,
            50
        };

        if (DrawUIButton(
                backBtn,
                "BACK",
                BLUE,
                SKYBLUE))
        {
            gShowHowToPlay = false;
        }

        return UI_NONE;
    }


    /* MAIN MENU */

    const char *title = "CURE INC.";

    int titleWidth = MeasureText(
        title,
        50
    );

    DrawText(
        title,
        (screenWidth - titleWidth) / 2,
        150,
        50,
        DARKBLUE
    );


    Rectangle playBtn = {
        (float)(screenWidth - 220) / 2,
        300,
        220,
        50
    };

    Rectangle helpBtn = {
        (float)(screenWidth - 220) / 2,
        370,
        220,
        50
    };

    Rectangle exitBtn = {
        (float)(screenWidth - 220) / 2,
        440,
        220,
        50
    };


    if (DrawUIButton(
            playBtn,
            "PLAY",
            DARKGREEN,
            GREEN))
    {
        return UI_START_GAME;
    }


    if (DrawUIButton(
            helpBtn,
            "HOW TO PLAY",
            BLUE,
            SKYBLUE))
    {
        gShowHowToPlay = true;

        return UI_NONE;
    }


    if (DrawUIButton(
            exitBtn,
            "EXIT",
            MAROON,
            RED))
    {
        return UI_EXIT;
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

void UI_DrawRegionPanel(Rectangle bounds, RegionData *region, GameStats *stats, CureState *cure) {
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
    if (DrawUIButton(fundBtn, "Fund Local Research ($100)", DARKGREEN, GREEN)) {
        if (cure->funding >= 100) {
            cure->funding -= 100;
            region->cureResearch += 15.0f;
            if (region->cureResearch > 100.0f) region->cureResearch = 100.0f;
        }
    }

    const char *toggleLabel = region->bordersClosed ? "Reopen Borders" : "Close Borders ($100)";
    Rectangle borderBtn = { bounds.x + 15, bounds.y + 275, bounds.width - 30, 35 };
    if (DrawUIButton(borderBtn, toggleLabel, MAROON, RED)) {
        if (!region->bordersClosed && cure->funding >= 100) {
            cure->funding -= 100;
            region->bordersClosed = true;
        } else if (region->bordersClosed) {
            region->bordersClosed = false;
        }
    }
}

//full-screen coordinators//
// Stopgap categorization: Event has no `type` field yet (see events.c),
// so we infer a category from the title text. The correct long-term fix
// is an explicit enum set when the event is created, not guessed here.
static Color EventColor(const char *title) {
    if (strstr(title, "utat") || strstr(title, "utbreak")) return MAROON;
    if (strstr(title, "esearch") || strstr(title, "reakthrough")) return DARKGREEN;
    return DARKBLUE;
}

void UI_DrawEventLog(const GameState *gs) {
    int y = SCREEN_HEIGHT - 80;
    for (int i = 0; i < MAX_EVENTS; i++) {
        if (!gs->eventLog[i].active) continue;

        Color cardColor = EventColor(gs->eventLog[i].title);

        Rectangle box = { 20, (float)y, 1000, 46 };
        DrawRectangleRec(box, Fade(cardColor, 0.85f));
        DrawRectangleLinesEx(box, 1.5f, cardColor);

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

    UIAction labAction = UI_DrawInfoPanel(gs);
    
    /* Handle lab panel actions */
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
    stats.cureProgress    = gs->cure.researchProgress;
    stats.globalInfection = gs->virus.globalInfected * 100.0f;
    stats.budget          = (int)gs->cure.funding;
    stats.dayCount        = gs->day;
    stats.gameSpeed       = (gs->screen == SCREEN_PAUSED) ? 0 : gs->gameSpeed;
    stats.fundingRate     = gs->cure.fundingPerTick;
    stats.researchRate    = gs->cure.rpPerTick;
    stats.stability       = gs->cure.stability;

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
    if (gs->screen == SCREEN_GAME)
        UI_DrawRegionPanel(panel, &rd, &stats, &gs->cure);

    gRegionPanelOpen   = rd.isSelected;
    sel->cureResearch  = rd.cureResearch;
    sel->bordersClosed = rd.bordersClosed;

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

UIAction UI_DrawEndScreen(const GameState *gs)
{
    bool won = (gs->screen == SCREEN_WIN);

    const char *title =
        won ? "VICTORY" : "DEFEAT";

    const char *subtitle =
        won ?
        "Humanity has contained the outbreak." :
        "Humanity could not contain the outbreak.";

    Color titleColor =
        won ? DARKGREEN : RED;


    int titleSize = 48;

    int titleWidth =
        MeasureText(title, titleSize);

    DrawText(
        title,
        (SCREEN_WIDTH - titleWidth) / 2,
        100,
        titleSize,
        titleColor
    );


    int subtitleWidth =
        MeasureText(subtitle, 22);

    DrawText(
        subtitle,
        (SCREEN_WIDTH - subtitleWidth) / 2,
        165,
        22,
        DARKGRAY
    );


    Rectangle panel = {
        (float)(SCREEN_WIDTH - 600) / 2,
        220,
        600,
        340
    };

    DrawUIPanel(
        panel,
        RAYWHITE,
        DARKGRAY,
        2.0f
    );


    char buffer[128];

    int x = (int)panel.x + 50;
    int y = (int)panel.y + 35;


    snprintf(
        buffer,
        sizeof(buffer),
        "Days Survived: %d",
        gs->day
    );

    DrawText(
        buffer,
        x,
        y,
        22,
        BLACK
    );

    y += 45;


    snprintf(
        buffer,
        sizeof(buffer),
        "Total Deaths: %.1f%%",
        gs->virus.globalDead * 100.0f
    );

    DrawText(
        buffer,
        x,
        y,
        22,
        RED
    );

    y += 45;


    snprintf(
        buffer,
        sizeof(buffer),
        "Global Vaccinated: %.1f%%",
        gs->cure.globalDistributed * 100.0f
    );

    DrawText(
        buffer,
        x,
        y,
        22,
        DARKGREEN
    );

    y += 45;


    snprintf(
        buffer,
        sizeof(buffer),
        "Final Infection: %.1f%%",
        gs->virus.globalInfected * 100.0f
    );

    DrawText(
        buffer,
        x,
        y,
        22,
        MAROON
    );

    y += 45;


    if (gs->cure.completionDay > 0)
    {
        snprintf(
            buffer,
            sizeof(buffer),
            "Cure Completed: Day %d",
            gs->cure.completionDay
        );
    }
    else
    {
        snprintf(
            buffer,
            sizeof(buffer),
            "Cure Completed: No"
        );
    }

    DrawText(
        buffer,
        x,
        y,
        22,
        DARKBLUE
    );


    /* Reason for victory/defeat */

    if (gs->endReason != NULL)
    {
        int reasonWidth =
            MeasureText(gs->endReason, 16);

        DrawText(
            gs->endReason,
            (SCREEN_WIDTH - reasonWidth) / 2,
            (int)panel.y + 285,
            16,
            DARKGRAY
        );
    }


    Rectangle menuBtn = {
        (float)(SCREEN_WIDTH - 220) / 2,
        600,
        220,
        50
    };

    if (DrawUIButton(
            menuBtn,
            "MAIN MENU",
            BLUE,
            SKYBLUE))
    {
        return UI_MAIN_MENU;
    }


    return UI_NONE;
}

static UIAction DrawLabBody(Rectangle area, const CureState *c) {
    UIAction action = UI_NONE;
    float y = area.y;

    /* Display current stats */
    char buf[64];
    snprintf(buf, sizeof(buf), "Scientists: %d", c->scientistCount);
    DrawText(buf, (int)area.x, (int)y, 14, DARKGRAY);
    y += 18;

    snprintf(buf, sizeof(buf), "Lab Level: %d/3", c->labLevel);
    DrawText(buf, (int)area.x, (int)y, 14, DARKGRAY);
    y += 18;

    snprintf(buf, sizeof(buf), "Production Level: %d/3", c->productionLevel);
    DrawText(buf, (int)area.x, (int)y, 14, DARKGRAY);
    y += 18;

    snprintf(buf, sizeof(buf), "Vaccine Stock: %.1f", c->vaccineStockpile);
    DrawText(buf, (int)area.x, (int)y, 14, DARKGRAY);
    y += 26;

    /* Buttons with costs */
    Rectangle hireBtn    = { area.x, y,      area.width, 32 };
    Rectangle upgradeBtn = { area.x, y + 40, area.width, 32 };
    Rectangle prodBtn    = { area.x, y + 80, area.width, 32 };

    if (DrawUIButton(hireBtn, "Hire Scientist ($100)", DARKBLUE, SKYBLUE))       
        action = UI_HIRE_SCIENTIST;
    
    const char *labBtnText = (c->labLevel >= 3) ? "Lab Maxed" : 
        (c->labLevel == 2) ? "Upgrade Lab ($450)" :
        (c->labLevel == 1) ? "Upgrade Lab ($300)" : "Upgrade Lab ($150)";
    if (DrawUIButton(upgradeBtn, labBtnText, DARKBLUE, SKYBLUE))       
        action = UI_UPGRADE_LAB;
    
    const char *prodBtnText = (c->productionLevel >= 3) ? "Production Maxed" :
        (c->productionLevel == 2) ? "Upgrade Prod ($600)" :
        (c->productionLevel == 1) ? "Upgrade Prod ($400)" : "Upgrade Prod ($200)";
    if (DrawUIButton(prodBtn, prodBtnText, DARKBLUE, SKYBLUE)) 
        action = UI_INCREASE_PRODUCTION;

    return action;
}

static void DrawVirusBody(Rectangle area, const Virus *v) {
    float y = area.y;

    Rectangle infBar = { area.x, y, area.width, 20 };
    DrawProgressBar(infBar, v->infectivity * 100.0f, RED, LIGHTGRAY, "Infectivity");
    y += 26;

    Rectangle sevBar = { area.x, y, area.width, 20 };
    DrawProgressBar(sevBar, v->severity * 100.0f, MAROON, LIGHTGRAY, "Severity");
    y += 26;

    Rectangle infectedBar = { area.x, y, area.width, 20 };
    DrawProgressBar(infectedBar, v->globalInfected * 100.0f, ORANGE, LIGHTGRAY, "Infected");
    y += 26;

    Rectangle deadBar = { area.x, y, area.width, 20 };
    DrawProgressBar(deadBar, v->globalDead * 100.0f, BLACK, LIGHTGRAY, "Deaths");
    y += 30;

    char buf[64];
    snprintf(buf, sizeof(buf), "Resistance: %.0f%%", v->resistance * 100.0f);
    DrawText(buf, (int)area.x, (int)y, 14, DARKGRAY);
    y += 22;

    MutationTrait allTraits[] = {
        TRAIT_AIRBORNE, TRAIT_DRUG_RESISTANT, TRAIT_STEALTH, TRAIT_LETHAL,
        TRAIT_FAST_SPREAD, TRAIT_COLD_ADAPTED, TRAIT_HOT_ADAPTED, TRAIT_LONG_INCUBATION
    };

    char traitsBuf[160] = "Traits: ";
    bool any = false;
    for (int i = 0; i < 8; i++) {
        if (virus_has_trait(v, allTraits[i])) {
            if (any) strncat(traitsBuf, ", ", sizeof(traitsBuf) - strlen(traitsBuf) - 1);
            strncat(traitsBuf, virus_trait_name(allTraits[i]), sizeof(traitsBuf) - strlen(traitsBuf) - 1);
            any = true;
        }
    }
    if (!any) strncat(traitsBuf, "None yet", sizeof(traitsBuf) - strlen(traitsBuf) - 1);
    DrawText(traitsBuf, (int)area.x, (int)y, 12, DARKGRAY);
}

static void DrawResearchBody(Rectangle area, const CureState *c) {
    static const char *phaseNames[] = { "Discovery", "Trials", "Production", "Distribution" };
    float y = area.y;

    char buf[64];
    snprintf(buf, sizeof(buf), "Phase: %s", phaseNames[c->phase]);
    DrawText(buf, (int)area.x, (int)y, 16, DARKBLUE);
    y += 24;

    Rectangle progBar = { area.x, y, area.width, 20 };
    DrawProgressBar(progBar, c->researchProgress, BLUE, LIGHTGRAY, "Phase Progress");
    y += 26;

    Rectangle distBar = { area.x, y, area.width, 20 };
    DrawProgressBar(distBar, c->globalDistributed * 100.0f, DARKGREEN, LIGHTGRAY, "Distributed");
    y += 30;

    snprintf(buf, sizeof(buf), "Stability: %.0f%%", c->stability * 100.0f);
    DrawText(buf, (int)area.x, (int)y, 14, DARKGRAY);
    y += 20;

    snprintf(buf, sizeof(buf), "Effectiveness: %.0f%%", c->effectiveness * 100.0f);
    DrawText(buf, (int)area.x, (int)y, 14, DARKGRAY);
    y += 20;

    snprintf(buf, sizeof(buf), "Production: %.1f/day", c->productionRate);
    DrawText(buf, (int)area.x, (int)y, 14, DARKGRAY);
}

UIAction UI_DrawInfoPanel(GameState *gs) {
    Rectangle bounds = { 20, 80, 260, 300 };
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
        case INFO_TAB_LAB:      action = DrawLabBody(bodyArea, &gs->cure); break;
        case INFO_TAB_VIRUS:    DrawVirusBody(bodyArea, &gs->virus);       break;
        case INFO_TAB_RESEARCH: DrawResearchBody(bodyArea, &gs->cure);     break;
    }

    return action;
}