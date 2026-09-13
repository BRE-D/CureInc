// Header guard: include these declarations only once per compiled source file.
#ifndef TYPES_H
#define TYPES_H

// Lose when deaths reach this fraction of original world population (0.30 = 30%).
#define LOSS_DEATH_SHARE 0.30f

// Number of simulated regions and region array length.
#define MAX_REGIONS      8
// Maximum simultaneous notification slots, not the random-event pool size.
#define MAX_EVENTS       8

// Game window width in pixels.
#define SCREEN_WIDTH     1366
// Game window height in pixels.
#define SCREEN_HEIGHT    768

// Real seconds per game day at normal speed (2 seconds).
#define DEFAULT_DAY_LENGTH  2.0f
// Base strength of imported infection pressure between regions.
#define GLOBAL_MIXING_RATE  0.02f

// Enum naming the active screen; only SCREEN_GAME advances simulation time.
typedef enum {
  // Main menu and help page.
  SCREEN_MENU = 0,
  // Live simulation and gameplay.
  SCREEN_GAME,
  // Frozen simulation with pause overlay.
  SCREEN_PAUSED,
  // Victory results.
  SCREEN_WIN,
  // Defeat results.
  SCREEN_LOSE
} GameScreen;

// Enum recording regional infection severity. These stored states are separate from UI card colors.
typedef enum {
    // Infected fraction at most 0.000001.
    REGION_CLEAN = 0,
    // Above clean threshold and below 30% infected.
    REGION_INFECTED,
    // At least 30% but below 60% infected.
    REGION_CRITICAL,
    // At least 60% infected.
    REGION_DEVASTATED
} RegionState;

// Ordered enum for cure stages. Incrementing the phase advances to the next stage.
typedef enum {
    // First research phase; needs 100 research points.
    PHASE_DISCOVERY = 0,
    // Second research phase; needs another 100 points.
    PHASE_TRIALS,
    // Build vaccine stock to 10 units.
    PHASE_PRODUCTION,
    // Produce and administer vaccines to susceptible people.
    PHASE_DISTRIBUTION
} ResearchPhase;

// Enum for regional climate, used by adapted mutations and the winter event.
typedef enum {
    // No hot/cold adaptation bonus.
    CLIMATE_TEMPERATE = 0,
    // Cold adaptation can boost spread; winter event applies.
    CLIMATE_COLD,
    // Hot adaptation can boost spread.
    CLIMATE_HOT
} RegionClimate;

// Bit-flag enum: each 1 << n uses a different bit, allowing several traits in one integer.
typedef enum {

  // No mutation flag.
  TRAIT_NONE = 0,
  // Adds spread strength when selected.
  TRAIT_AIRBORNE = (1 << 0),
  // Adds extra drug resistance when selected.
  TRAIT_DRUG_RESISTANT = (1 << 1),
  // Weakens the healthcare reduction of spread.
  TRAIT_STEALTH = (1 << 2),
  // Adds base daily mortality when selected.
  TRAIT_LETHAL = (1 << 3),
  // Multiplies cross-region mixing by 1.5.
  TRAIT_FAST_SPREAD = (1 << 4),
  // Multiplies spread by 1.15 in cold regions.
  TRAIT_COLD_ADAPTED = (1 << 5),
  // Multiplies spread by 1.15 in hot regions.
  TRAIT_HOT_ADAPTED = (1 << 6),
  // Adds spread strength when selected; no separate incubation timer.
  TRAIT_LONG_INCUBATION = (1 << 7)
} MutationTrait;

// Struct holding virus rates, mutation history, and calculated global outbreak fractions.
typedef struct {
    // Daily spreading coefficient; larger values increase new infections.
    float infectivity;
    // Base fraction of infected people dying per day, before healthcare effects.
    float severity;
    // Drug resistance (0..1); reduces research speed and vaccine effectiveness.
    float resistance;
    // Daily mutation probability on eligible days 20..29 after the last mutation.
    float mutationRate;
    // Fraction of starting infected people recovering per day.
    float recoveryRate;

    // Integer bit mask holding all acquired mutation flags.
    int activeTraits;
    // Game day of the most recent mutation; starts at zero.
    int lastMutationDay;
    // Most recently selected mutation flag.
    MutationTrait lastMutation;

    // Currently infected divided by original world population (0..1).
    float globalInfected;
    // Cumulative deaths divided by original world population (0..1).
    float globalDead;
} Virus;

// Struct holding shared research, funding, staff, vaccine stock, and survivor protection.
typedef struct {
  // Day Distribution opened; zero means it has not opened yet.
  int completionDay;
  // Current cure stage; enum order follows the research-to-distribution journey.
  ResearchPhase phase;
  // Points completed in the current research phase (0..100).
  float researchProgress;
  // Cure stability (0..1); mutations lower it and it scales research/effectiveness.
  float stability;
  // Fraction of administered doses that successfully protect people (0..1).
  float effectiveness;
  // Current vaccine capacity in units per game day; used after Trials.
  float productionRate;
  // Protected people divided by living world population (0..1).
  float globalDistributed;

  // Shared spendable money for all global and regional purchases.
  float funding;
  // Income per game day, despite the historical Tick name.
  float fundingPerTick;
  // Base research points per game day before bonuses and multipliers.
  float rpPerTick;

  // Number of purchased scientists; improves research and production.
  int   scientistCount;
  // Purchased lab level (0..3), improving research speed.
  int   labLevel;
  // Purchased factory level (0..3), improving vaccine output.
  int   productionLevel;
  // Available vaccine units; one unit supplies doses for 1% of original world population.
  float vaccineStockpile;
} CureState;

// Struct for one region. Population is in millions; infected/dead/vaccinated use its original population.
typedef struct {
  // Read-only region name displayed in cards and details.
  const char *name;
  // Original region population in millions, used as the weight in world totals.
  float population;
  // Currently infected fraction of original regional population (0..1).
  float infected;
  // Cumulative dead fraction of original regional population; never decreases.
  float dead;
  // Consecutive days with hospital demand above capacity; resets when load recovers.
  int   overloadedDays;
  // Successfully protected fraction of original regional population (0..1).
  float vaccinated;
  // Base hospital/prevention score (0..1), improved by local research.
  float healthcareCapacity;
  // Baseline border strength (0..1); higher values reduce spread.
  float borderControl;
  // Stored infection category for this region, separate from UI card colors.
  RegionState state;
  // Region climate used by climate-adapted traits and winter outbreaks.
  RegionClimate climate;
  // Local research points (0..100); boost global research and this region healthcare.
  float cureResearch;
  // Whether player-ordered closure is active (1/true closed, 0/false open).
  int bordersClosed;
} Region;

// Struct for one notification: text pointers, visible flag, and remaining display time.
typedef struct {
  // Read-only notification heading; the event stores the pointer rather than copying text.
  const char *title;
  // Read-only notification detail; must remain valid while the event is displayed.
  const char *description;
  // 1 means the notification is active; 0 means this slot is free.
  int active;
  // Remaining real seconds before the notification expires; frozen while paused.
  float timer;
} Event;

// Struct containing the shared game data passed between simulation, cure, events, and UI.
typedef struct {
    // Current menu/gameplay/pause/result screen.
    GameScreen screen;
    // All virus state and calculated global infection/death totals.
    Virus     virus;
    // All research, funding, staffing, and vaccination state.
    CureState cure;

    // Array of the eight simulation regions; indices identify the same regions throughout the game.
    Region    regions[MAX_REGIONS];
    // Array of eight notification slots, not the pool of all possible random events.
    Event     eventLog[MAX_EVENTS];
    // Number of currently active notification slots.
    int       eventCount;
    // Last random-event pool index; -1 initially. Used to reduce immediate repeats.
    int       lastEventIndex;
    // Read-only win/lose explanation; NULL until a result is decided.
    const char *endReason;
    // Number of completed game days.
    int   day;
    // Accumulated frame seconds multiplied by game speed; whole days are removed from it.
    float dayTimer;
    // Seconds required for one game day at normal speed.
    float dayLength;
    // Simulation speed multiplier: 1 or 2. The screen state controls pausing.
    int   gameSpeed;
    // Index (0..7) of the region selected for the details panel.
    int selectedRegionIndex;
} GameState;

#endif
