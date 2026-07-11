#include "skills.h"
#include "events.h"

void skills_init(GameState *gs)
{
    if (!gs) return;

    gs->skillCount = 6;

    // Skill 0: Public Awareness (boosts trust & border control slightly)
    gs->skills[0] = (SkillNode){
        .name = "Public Awareness",
        .description = "Encourage masks and distancing (+15% trust, +5% border)",
        .unlocked = 0,
        .cost = 15.0f,
        .prereqIndex = -1,
        .researchMod = 0.0f,
        .fundingMod = 0.0f,
        .distributionMod = 0.1f,
        .borderMod = 0.05f
    };

    // Skill 1: Global Funding (increases passive daily funding)
    gs->skills[1] = (SkillNode){
        .name = "Global Funding",
        .description = "Secure backing from major nations (+15 funding/day)",
        .unlocked = 0,
        .cost = 20.0f,
        .prereqIndex = -1,
        .researchMod = 0.0f,
        .fundingMod = 15.0f,
        .distributionMod = 0.0f,
        .borderMod = 0.0f
    };

    // Skill 2: Border Quarantine (boosts border controls)
    gs->skills[2] = (SkillNode){
        .name = "Border Quarantine",
        .description = "Deploy airport screening and travel bans (+20% border)",
        .unlocked = 0,
        .cost = 30.0f,
        .prereqIndex = 0, // requires Public Awareness
        .researchMod = 0.0f,
        .fundingMod = 0.0f,
        .distributionMod = 0.0f,
        .borderMod = 0.20f
    };

    // Skill 3: Genetic Sequencing (boosts research speed)
    gs->skills[3] = (SkillNode){
        .name = "Genetic Sequencing",
        .description = "Sequence the virus genome (+25% research speed)",
        .unlocked = 0,
        .cost = 25.0f,
        .prereqIndex = -1,
        .researchMod = 0.25f,
        .fundingMod = 0.0f,
        .distributionMod = 0.0f,
        .borderMod = 0.0f
    };

    // Skill 4: Accelerated Trials (boosts research speed)
    gs->skills[4] = (SkillNode){
        .name = "Accelerated Trials",
        .description = "Fast-track clinical safety testing (+50% research speed)",
        .unlocked = 0,
        .cost = 40.0f,
        .prereqIndex = 3, // requires Genetic Sequencing
        .researchMod = 0.50f,
        .fundingMod = 0.0f,
        .distributionMod = 0.0f,
        .borderMod = 0.0f
    };

    // Skill 5: Mass Production (boosts distribution rate)
    gs->skills[5] = (SkillNode){
        .name = "Mass Production",
        .description = "Re-tool factories for manufacturing (+40% distribution)",
        .unlocked = 0,
        .cost = 35.0f,
        .prereqIndex = 1, // requires Global Funding
        .researchMod = 0.0f,
        .fundingMod = 0.0f,
        .distributionMod = 0.40f,
        .borderMod = 0.0f
    };
}

int skills_unlock(GameState *gs, int skill_index)
{
    if (!gs || skill_index < 0 || skill_index >= gs->skillCount) return 0;

    SkillNode *skill = &gs->skills[skill_index];
    if (skill->unlocked) return 0;

    // Check prerequisites
    if (skill->prereqIndex != -1) {
        if (!gs->skills[skill->prereqIndex].unlocked) {
            return 0; // Prerequisite not unlocked
        }
    }

    // Check cost
    if (gs->cure.researchPoints < skill->cost) {
        return 0; // Not enough RP
    }

    // Deduct cost and unlock
    gs->cure.researchPoints -= skill->cost;
    skill->unlocked = 1;

    // Apply immediate flat modifiers
    if (skill->fundingMod > 0.0f) {
        gs->cure.fundingPerTick += skill->fundingMod;
    }

    if (skill->borderMod > 0.0f) {
        for (int i = 0; i < MAX_REGIONS; i++) {
            gs->regions[i].borderControl += skill->borderMod;
            if (gs->regions[i].borderControl > 1.0f) {
                gs->regions[i].borderControl = 1.0f;
            }
        }
    }

    // Add a notification event
    event_log_add(gs, "Research Upgrade", skill->name);

    return 1;
}
