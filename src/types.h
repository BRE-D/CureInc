#ifndef TYPES_H
#define TYPES_H

#define LOSS_DEATH_SHARE 0.30f // মোট জনসংখ্যার ৩০% মারা গেলে হারবে; কমালে খেলা কঠিন।

#define MAX_REGIONS      8
#define MAX_EVENTS       8

#define SCREEN_WIDTH     1366
#define SCREEN_HEIGHT    768

#define DEFAULT_DAY_LENGTH  2.0f // ১ game day = ২ বাস্তব সেকেন্ড।
#define GLOBAL_MIXING_RATE  0.02f // অন্য অঞ্চল থেকে রোগ আসার মাত্রা।

typedef enum {
  SCREEN_MENU = 0,
  SCREEN_GAME,
  SCREEN_PAUSED,
  SCREEN_WIN,
  SCREEN_LOSE
} GameScreen;

typedef enum {
    REGION_CLEAN = 0,
    REGION_INFECTED,
    REGION_CRITICAL,
    REGION_DEVASTATED
} RegionState;

typedef enum {
    PHASE_DISCOVERY = 0,
    PHASE_TRIALS,
    PHASE_PRODUCTION,
    PHASE_DISTRIBUTION
} ResearchPhase;

typedef enum {
    CLIMATE_TEMPERATE = 0,
    CLIMATE_COLD,
    CLIMATE_HOT
} RegionClimate;

typedef enum {
  // 1 << n মানে n নম্বর bit; তাই একসঙ্গে একাধিক trait রাখা যায়।
  TRAIT_NONE = 0,
  TRAIT_AIRBORNE = (1 << 0),
  TRAIT_DRUG_RESISTANT = (1 << 1),
  TRAIT_STEALTH = (1 << 2),
  TRAIT_LETHAL = (1 << 3),
  TRAIT_FAST_SPREAD = (1 << 4),
  TRAIT_COLD_ADAPTED = (1 << 5),
  TRAIT_HOT_ADAPTED = (1 << 6),
  TRAIT_LONG_INCUBATION = (1 << 7)
} MutationTrait;

typedef struct {
    float infectivity; // প্রতিদিন সংক্রমণ ছড়ানোর শক্তি।
    float severity; // চিকিৎসার প্রভাব ধরার আগের দৈনিক মৃত্যুর হার।
    float resistance; // research ও vaccine-এর কার্যকারিতা কমায়।
    float mutationRate; // ২০–২৯ দিনের মধ্যে প্রতিদিন mutation-এর সম্ভাবনা।
    float recoveryRate; // প্রতিদিন আক্রান্তদের কত অংশ সুস্থ হয়।

    int activeTraits; // একটি int-এর আলাদা bit-এ একাধিক বৈশিষ্ট্য রাখি।
    int lastMutationDay; // সর্বশেষ mutation কোন দিনে হয়েছে।
    MutationTrait lastMutation; // সর্বশেষ mutation-এর ধরন।

    float globalInfected; // বর্তমানে আক্রান্ত / শুরুর মোট জনসংখ্যা।
    float globalDead; // এ পর্যন্ত মৃত / শুরুর মোট জনসংখ্যা।
} Virus;

typedef struct {
  int completionDay; // Distribution শুরু হওয়ার দিন; শুরু না হলে ০।
  ResearchPhase phase; // Discovery, Trials, Production অথবা Distribution।
  float researchProgress; // বর্তমান research ধাপ কত শতাংশ শেষ: ০–১০০।
  float stability; // স্থিতিশীলতা: ০–১; mutation হলে কমে।
  float effectiveness; // দেওয়া vaccine-এর কত অংশ মানুষকে সুরক্ষা দেয়: ০–১।
  float productionRate; // প্রতিদিন উৎপাদনের ক্ষমতা, vaccine unit-এ।
  float globalDistributed; // সুরক্ষিত মানুষ / জীবিত মানুষ; শুরুর জনসংখ্যা দিয়ে ভাগ নয়।

  float funding; // বিশ্বব্যাপী খরচের টাকা; আলাদা অঞ্চলের wallet নয়।
  float fundingPerTick; // প্রতি game day-তে আয়।
  float rpPerTick; // upgrade multiplier-এর আগের মূল research/day।

  int   scientistCount; // কেনা scientist-এর সংখ্যা।
  int   labLevel; // Lab upgrade level: ০–৩।
  int   productionLevel; // উৎপাদন upgrade level: ০–৩।
  float vaccineStockpile; // মজুত vaccine; ১ unit = শুরুর জনসংখ্যার ১%-এর জন্য dose।
} CureState;

typedef struct {
  const char *name; // অঞ্চলের নাম।
  float population; // অঞ্চলের শুরুর জনসংখ্যা, million এককে।
  float infected; // বর্তমানে আক্রান্ত মানুষের অংশ: ০–১।
  float dead; // এ পর্যন্ত মৃত মানুষের অংশ; এই মান কমে না।
  int   overloadedDays; // একটানা কয় দিন হাসপাতাল overloaded।
  float vaccinated; // vaccine-এ সুরক্ষিত মানুষের অংশ: ০–১।
  float healthcareCapacity; // হাসপাতালের মূল ক্ষমতার মান: ০–১।
  float borderControl; // সীমান্ত নিয়ন্ত্রণের শক্তি: ০–১।
  RegionState state; // অঞ্চলের সংক্রমণের অবস্থা।
  RegionClimate climate; // স্বাভাবিক, ঠান্ডা অথবা গরম আবহাওয়া।
  float cureResearch; // স্থানীয় research point: ০–১০০।
  int bordersClosed; // ১ হলে player সীমান্ত বন্ধ করেছে।
} Region;

typedef struct {
  const char *title; // খবরের শিরোনাম।
  const char *description; // খবরের বিবরণ।
  int active; // ১ হলে খবর দেখা যাবে।
  float timer; // খবর দেখানোর বাকি বাস্তব সেকেন্ড।
} Event;

typedef struct {
    GameScreen screen; // বর্তমানে কোন screen চলছে।
    Virus     virus; // Virus-এর সব তথ্য একসঙ্গে।
    CureState cure; // Research ও vaccine-এর সব তথ্য।

    Region    regions[MAX_REGIONS]; // আটটি অঞ্চলের array।
    Event     eventLog[MAX_EVENTS]; // পর্দায় দেখানোর খবরের array।
    int       eventCount; // সক্রিয় খবরের সংখ্যা।
    int       lastEventIndex; // একই event পরপর আসা কমাতে আগের event মনে রাখি।

    const char *endReason; // জয় বা হারের কারণ।
    int   day; // কত game day পার হয়েছে।
    float dayTimer; // পরের দিন হওয়ার জন্য জমা হওয়া সময়।
    float dayLength; // এক game day-এর জন্য যত বাস্তব সেকেন্ড লাগে।
    int   gameSpeed; // ১ = স্বাভাবিক, ২ = দ্বিগুণ; pause নির্ধারিত হয় screen দিয়ে।
    int selectedRegionIndex; // Player কোন অঞ্চলে click করেছে তার index: ০–৭।
} GameState;

#endif
