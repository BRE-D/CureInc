// Header guard: include these declarations only once per compiled source file.
#ifndef CURE_H
#define CURE_H
#include "types.h"

// Price of one scientist, shared by purchase logic and UI.
#define SCIENTIST_COST 100.0f
// Next lab price equals this value times (current level + 1).
#define LAB_BASE_COST 150.0f
// Next factory price equals this value times (current level + 1).
#define PRODUCTION_BASE_COST 200.0f

// Clear cure data and set Discovery, full stability, $50 starting funds, $10/day income, and 0.5 base
// research/day. Called by reset_game; returns nothing.
// c: cure data; const means read-only access.
void cure_init(CureState *c);
// Add daily income, refresh capacity/effectiveness, then advance research or vaccine
// production/distribution. Called by day_tick with one day. Consume all administered doses, including
// unsuccessful ones; returns nothing.
// gs: shared game state; const means this function only reads it.
// dtDays: elapsed game days (main passes 1).
void cure_update(GameState *gs, float dtDays);

// Return actual research points/day after local research, scientists, lab level, stability, and
// resistance. Return zero after Trials; shared by simulation and UI purchase previews.
// gs: shared game state; const means this function only reads it.
float cure_research_rate(const GameState *gs);
// Return vaccine units/day: base 1, plus 0.5 per production level and 0.2 per scientist. Simulation and UI
// share this formula; actual production starts after Trials.
// c: cure data; const means read-only access.
float cure_production_rate(const CureState *c);
// Return current phase progress clamped to 0..100: research points, stockpile relative to 10 units, or
// protected percentage of survivors.
// c: cure data; const means read-only access.
float cure_phase_progress(const CureState *c);

// Buy one scientist if funding covers SCIENTIST_COST and refresh production capacity. Return 1 on
// purchase, otherwise 0. Scientists improve research and production.
// c: cure data; const means read-only access.
int  cure_hire_scientist(CureState *c);
// Buy the next lab level only during research and below level 3. Charge base cost times next level; return
// 1 on purchase, otherwise 0. Later phases have no research to accelerate.
// c: cure data; const means read-only access.
int  cure_upgrade_lab(CureState *c);
// Buy the next production level below level 3 and refresh units/day. Charge base cost times next level;
// return 1 on purchase, otherwise 0.
// c: cure data; const means read-only access.
int  cure_upgrade_production(CureState *c);

#endif
