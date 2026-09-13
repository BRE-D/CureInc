# CureInc - Pandemic Management Strategy Game

CureInc is a real-time pandemic management strategy game built in **C with raylib**. The player leads a global response team, develops a cure, manages research and vaccine production, supports individual regions, and tries to control the outbreak before too many people die.

---

## Game Objective

### Win
You win when all of the following are true:
- The cure has reached the **Distribution** phase.
- At least **90% of the living population is protected/vaccinated**.
- Fewer than **5% of the living population is infected**.

### Lose
You lose if either condition occurs:
- Global deaths reach **30% of the original population**.
- **All 8 regions** remain healthcare-overloaded for **30 consecutive game days**.

---

## How to Play

1. Run `CureInc.exe`.
2. Click **PLAY**.
3. Hire scientists and upgrade the laboratory to complete cure research faster.
4. Monitor the eight regions and support regions that are struggling.
5. After Trials, vaccines enter Production. When the vaccine stockpile is ready, Distribution starts automatically.
6. Upgrade vaccine production to protect people faster.
7. Reach the win conditions before the death or healthcare-collapse conditions are reached.

The **How to Play** screen inside the game also explains the main gameplay loop.

---

## Core Gameplay

### Resources
- Starting funding: **$50**
- Base income: **+$10 per game day**
- Funding is used for scientists, laboratory upgrades, vaccine-production upgrades, local research, and border control.

### Cure Development
The cure progresses through four phases:

1. **Discovery**
2. **Trials**
3. **Production**
4. **Distribution**

Research is performed during Discovery and Trials. Production builds the vaccine stockpile, and Distribution protects the living population.

### Research and Production
- **Hire Scientist: $100**
  - Each scientist increases research speed by **20%**.
  - Each scientist also adds **0.2 vaccine units/day** to production.
- **Upgrade Lab: Level 1-3**
  - Costs: **$150 / $300 / $450**
  - Each lab level increases research speed by **25%**.
- **Upgrade Production: Level 1-3**
  - Costs: **$200 / $400 / $600**
  - Each level adds **0.5 vaccine units/day**.

Laboratory upgrades are useful during Discovery and Trials. Production upgrades remain useful once vaccine production begins.

---

## Regional Management

The game contains eight regions:

- The North
- Dorne
- Westeros
- The Vale
- Essos
- The Iron Islands
- Beyond the Wall
- The Dothraki Sea

Click a region to inspect its outbreak and healthcare status.

### Regional Actions
- **Fund Local Research ($100)**
  - Adds up to **15 local research points**.
  - Local research contributes to cure research while global research is active and also improves that region's effective healthcare capacity.
- **Close Borders ($100)**
  - Stops imported infection from other regions and reduces local spread.
- **Reopen Borders**
  - Reopens a previously closed region at no additional cost.

---

## Virus System

The virus tracks:
- Infectivity
- Severity
- Recovery rate
- Drug resistance
- Mutation rate
- Active mutation traits

Mutations can begin after roughly **20 game days**, with additional mutations occurring later depending on the mutation check. A base mutation increases infectivity by about **3%** and resistance by **5 percentage points**, while the selected mutation trait can add an additional effect.

Possible mutation traits include:
- Airborne
- Drug Resistant
- Stealth
- Lethal
- Fast Spread
- Cold Adapted
- Hot Adapted
- Long Incubation

Each successful mutation also reduces cure stability by **3 percentage points**, down to a minimum stability of **60%**.

---

## Random Events

A random world event is triggered every **7 game days**. Events can affect existing game variables such as:
- Funding and daily income
- Research progress and base research rate
- Border control
- Vaccine stockpile

Examples include Funding Surge, Public Panic, Lab Breakthrough, Budget Cuts, Volunteer Surge, Supply Disruption, Supply Chain Collapse, Political Infighting, and Medical Miracle.

Some events are informational and are used to communicate changes in the world state.

---

## Controls and UI

### Main Tabs
- **Lab** - Hire scientists and purchase upgrades.
- **Virus** - View virus statistics, global infection, deaths, resistance, and mutation information.
- **Cure** - View cure phase, research/production/distribution progress, stability, and effectiveness.

### Gameplay Controls
- **Click a region** - Select and inspect that region.
- **TAB** - Cycle through regions.
- **1x** - Normal speed.
- **2x** - Double speed.
- **Pause** - Pause the simulation while keeping information visible.

The top HUD shows important global information and the next recommended action.

---

## Strategy Tips

### Early Game
- Hire scientists when you can afford them.
- Upgrade the lab to accelerate Discovery and Trials.
- Use local research to support vulnerable regions and contribute additional research.
- Close borders selectively when imported infection becomes a serious threat.

### Production and Distribution
- Upgrade Production to create vaccines faster.
- Production automatically builds the initial vaccine stockpile.
- Distribution starts automatically once the required stockpile is reached.
- Continue monitoring deaths, infections, and hospital load while vaccination expands.

### Remember
To win, protect **90% of living people** and reduce infection among survivors to **below 5%** before global deaths reach **30%** or every region suffers prolonged healthcare collapse.

---

## Building from Source

### Requirements
- GCC / MinGW-compatible C compiler
- raylib installed in the location expected by the build configuration

The included Windows build script expects raylib under:

```text
C:\raylib\
```

### Build
Run:

```text
build.bat
```

or use the included `Makefile` if your local development environment is configured for it.

### Run
After a successful build:

```text
CureInc.exe
```

---

## Technical Details

- Language: **C (C11)**
- Graphics library: **raylib**
- Target platform: **Windows**
- Resolution: **1366 x 768**
- Target frame rate: **60 FPS**
- Default game-day length: **2 real seconds**
- Regions: **8**
- Cure phases: **4**

### Project Structure

```text
src/
├── main.c       - Main game loop, time progression, and win/lose checks
├── virus.c/h    - Virus spread, deaths, recovery, healthcare load, and mutations
├── cure.c/h     - Research, funding, vaccine production, and distribution
├── region.c/h   - Region initialization and region states
├── events.c/h   - Random event system
├── ui.c/h       - Menus, HUD, map/region interface, tabs, and end screens
└── types.h      - Shared game structures, enums, and constants
```

---

## Developers

- **Shafin Rahman Khan**
- **Shamiha Nawar Tanaj**
- **Tasmia Karim Tanisha**

**Engine/Library:** raylib  
**Language:** C  
**Year:** 2026

---

## Educational Use

This project was created for educational purposes as a group game-development project.
