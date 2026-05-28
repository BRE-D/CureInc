#ifndef TYPES_H
#define TYPES_H

// ---- VIRUS ----
typedef struct {
    float infectivity;     // how fast it spreads (0.0 - 1.0)
    float severity;        // how bad it is (0.0 - 1.0)
    float resistance;      // how hard to cure (0.0 - 1.0)
    float mutation_rate;   // how often it mutates (0.0 - 1.0)
} Virus;

// ---- REGION ----
typedef struct {
    char name[32];
    float infection_rate;  // % of population infected
    float compliance;      // how well people follow rules
    float severity;        // local severity level
    int population;
    int infected;
} Region;

// ---- CURE ----
typedef struct {
    float progress;        // 0.0 - 1.0
    float stability;       // 0.0 - 1.0
    float global_coverage; // 0.0 - 1.0
} Cure;

// ---- RESOURCES ----
typedef struct {
    int funding;
    int personnel;
    int supplies;
} Resources;

#endif