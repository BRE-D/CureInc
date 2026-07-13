#ifndef UI_H
#define UI_H

#include "raylib.h"
#include <stdbool.h>
#include "types.h"


//Game States structure
typedef struct UI_GameStats
{
    float cure_progress;
    float global_infection;
    int budget;
    int day_count;
    int game_speed;
} UI_GameStats;

typedef struct UI_RegionData
{
    const char *name;
    int population;
    int infected_count;
    float cure_research;
    bool borders_closed;
    bool is_selected;
} UI_RegionData;

//Functions declaration

void InitUI();

bool DrawUIbutton(Rectangle bounds, const char *text, Color base_color, Color hover_color);
void DrawUIpanel(Rectangle bounds, Color background, Color border, float border_width);
void Drawprogression(Rectangle bounds, float percentage, Color bar_color, Color bg_color, const char *label);

//Screen Drawing fucntions

void DrawMainmenu(GameState *state);
void DrawGameplayHUD(GameState *state, UI_GameStats *stats);
void DrawPauseoverlay(GameState *state);

void DrawRegionpanel(Rectangle bounds, Region *region, GameState *state);

#endif