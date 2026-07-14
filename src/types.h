#ifndef TYPES_H
#define TYPES_H

  /* 
   All required constants
  */

#define MAX_REGIONS      8
#define MAX_EVENTS       8
#define MAX_SKILLS       16

#define SCREEN_WIDTH     1366
#define SCREEN_HEIGHT    768

#define DEFAULT_DAY_LENGTH  3.0f   /* real-time seconds per simulated day */
#define GLOBAL_MIXING_RATE  0.02f  /* how strongly infected regions leak into others, per day */

  /* 
   All required enums
  */

/* Which screen is currently active — single unified enum shared by the
   simulation and the UI layer. Previously ui.h had its own separate
   AppScreen enum kept in sync by hand; now there is exactly one enum
   and one variable (GameState.screen), so nothing can drift. */
typedef enum {
  SCREEN_MENU = 0,
  SCREEN_GAME,
  SCREEN_PAUSED,
  SCREEN_WIN,
  SCREEN_LOSE
} GameScreen;

/* Infection severity tier of a region — used for colour coding */
typedef enum {
    REGION_CLEAN = 0,
    REGION_INFECTED,
    REGION_CRITICAL,
    REGION_DEVASTATED
} RegionState;

/* Stages of the cure development pipeline */
typedef enum {
    PHASE_DISCOVERY = 0,   /* identify and sequence the pathogen */
    PHASE_TRIALS,          /* clinical safety and efficacy testing */
    PHASE_PRODUCTION,      /* mass manufacturing of doses */
    PHASE_DISTRIBUTION     /* global rollout and vaccination */
} ResearchPhase;

/* Climate zone of a region — determines which mutation traits
   give the virus a spread bonus there */
typedef enum {
    CLIMATE_TEMPERATE = 0,
    CLIMATE_COLD,
    CLIMATE_HOT
} RegionClimate;

typedef enum {
  TRAIT_NONE = 0,
  TRAIT_AIRBORNE = (1 << 0),       /* +infectivity */
  TRAIT_DRUG_RESISTANT = (1 << 1), /* +resistance to cure */
  TRAIT_STEALTH = (1 << 2),      /* reduces detected case count, hurts trust */
  TRAIT_LETHAL = (1 << 3),       /* +severity / death rate */
  TRAIT_FAST_SPREAD = (1 << 4),  /* faster inter-region transmission */
  TRAIT_COLD_ADAPTED = (1 << 5), /* boosts spread in northern regions */
  TRAIT_HOT_ADAPTED = (1 << 6),  /* boosts spread in tropical regions */
  TRAIT_LONG_INCUBATION = (1 << 7) /* delays detection, allows silent spread */
} MutationTrait;

  /* 
   All required Structs
  */

/*
 * Virus - biological state of the pathogen.
 */
typedef struct {
    float infectivity;      /* base per-tick spread rate                  */
    float severity;         /* rate at which healthcare capacity degrades  */
    float resistance;       /* reduces final cure effectiveness (0-1)      */
    float mutationRate;     /* probability of acquiring a new trait daily  */
    int   activeTraits;     /* bitmask of active MutationTrait flags       */
    float globalInfected;   /* fraction of total world population infected */
    float globalDead;       /* cumulative fraction of population dead       */
 } Virus;

/*
 * CureState - the full research and production pipeline.
 */
typedef struct {
  ResearchPhase phase;
  float researchProgress;  /* 0-100, progress through the current phase  */
  float stability;         /* 0-1, degrades when virus mutates           */
  float effectiveness;     /* 0-1, potency of finished cure              */
  float productionRate;    /* doses generated per game-day (PRODUCTION+) */
  float globalDistributed; /* 0-1, fraction of population vaccinated     */
  float funding;           /* current funding pool                       */
  float fundingPerTick;    /* passive funding income per game-day        */
  float researchPoints;    /* currency spent to unlock skills            */
  float rpPerTick;         /* research points earned per game-day        */
} CureState;

/*
 * Region - one of MAX_REGIONS world regions.
 */
typedef struct {
  const char *name;
  float population;
  float infected;
  float vaccinated;
  float healthcareCapacity;
  float publicTrust;
  float borderControl;
  RegionState state;
  RegionClimate climate;
  float cureResearch;   /* 0-100, local research investment       */
  int bordersClosed;    /* 1 = player has locked this region down */
} Region;

/*
 * Event - a single entry in the rolling world event log.
 */
typedef struct {
  const char *title;
  const char *description;
  int active;
  float timer; /* display lifetime in real seconds          */
} Event;

/*
 * SkillNode - one node in the player's upgrade tree.
 * Modifiers are applied globally when the node is unlocked.
 */
typedef struct {
    const char *name;
    const char *description;
    int         unlocked;
    float       cost;         /* research point cost to unlock             */
    int         prereqIndex;  /* index of required prior skill (-1 = root) */

    float researchMod;
    float fundingMod;
    float distributionMod;
    float borderMod;
} SkillNode;

/*
 * GameState - top-level container.
 * Every module receives a pointer to this struct.
 */
typedef struct {
    GameScreen screen;   // What screen is currently active — including pause

    Virus     virus;
    CureState cure;

    Region    regions[MAX_REGIONS];
    Event     eventLog[MAX_EVENTS];
    int       eventCount;

    SkillNode skills[MAX_SKILLS];
    int       skillCount;

    int   day;
    float dayTimer;
    float dayLength;
    int   gameSpeed;
    int   selectedRegionIndex;
  } GameState;

#endif /* TYPES_H */