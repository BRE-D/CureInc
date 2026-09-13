// Header guard: include these declarations only once per compiled source file.
#ifndef VIRUS_H
#define VIRUS_H

#include "types.h"

// Clear the virus state and set starting spread, death, recovery, and mutation rates. Called by
// reset_game; returns nothing.
// v: virus data; const means read-only access.
void virus_init(Virus *v);
// Advance infections, deaths, recoveries, and hospital overload by one day. Read a snapshot of starting
// infections so region order does not change spread. Recoveries become susceptible again; returns nothing.
// gs: shared game state; const means this function only reads it.
void virus_update(GameState *gs);
// Recalculate population-weighted global totals after region changes. Infection and death use original
// population; vaccination uses living population. Called at initialization and after each daily update;
// returns nothing.
// gs: shared game state; const means this function only reads it.
void virus_refresh_totals(GameState *gs);

// Attempt a mutation 20..30 days after the last one; day 30 guarantees it. Keep old trait bits, apply the
// chosen effects, and cap rates. Repeated traits are allowed. Return 1 if mutated, otherwise 0.
// v: virus data; const means read-only access.
// day: current game-day number.
int virus_try_mutate(Virus *v, int day);
// Test a trait bit using bitwise AND. Return 1 if that bit is active, otherwise 0; does not change the
// virus.
// v: virus data; const means read-only access.
// t: mutation flag to test or name.
int virus_has_trait(const Virus *v, MutationTrait t);

// Return a display name for one mutation flag, or "None yet" for an unknown/no flag. The returned string
// is read-only.
// t: mutation flag to test or name.
const char *virus_trait_name(MutationTrait t);
// Return patient demand divided by available beds. A result above 1 means overload; a small minimum bed
// value prevents division by zero.
// r: region whose data is being examined.
float virus_hospital_load(const Region *r);

#endif
