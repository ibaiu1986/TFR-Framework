# TFR Framework Setup

This document provides a quick setup guide for TFR Framework.

TFR Framework and TFR Operations are the same addon.

* **TFR Operations** is the internal Workbench project name.
* **TFR Framework** is the public Workshop / repository name.

The addon is designed to work on any map through configurable components, spawn areas, HALO points, GameMode systems and reusable mission logic.

For detailed component placement, see:

```text
docs/component-placement.md
```

---

## Setup philosophy

TFR Framework should be easy to configure, difficult to break and safe for dedicated servers.

A mission maker should not need to understand the internal code to use the addon.

The setup should clearly explain:

* what goes on the GameMode;
* what goes on the player character;
* what goes on PlayerController;
* what goes on NPCs or civilians;
* what goes on physical admin objects;
* what goes on world marker entities;
* what is required;
* what is optional.

If setup requires too much guessing, the system needs better defaults, clearer warnings or better documentation.

---

## Current official script structure

The current script structure is:

```text
scripts/Game/TFR/
├─ Actions/
├─ Components/
├─ Missions/
├─ Radial/
└─ Spawn/
```

This structure is accepted as official for now.

Scripts should not be moved just for cosmetic reasons.

---

## Setup documents

Use these documents together:

```text
docs/architecture.md
```

Explains the current addon architecture and main systems.

```text
docs/component-placement.md
```

Explains where each component should be placed.

```text
docs/folder-structure.md
```

Explains the official script folder structure.

```text
docs/performance.md
```

Explains performance rules and dedicated server safety.

```text
docs/roadmap.md
```

Explains the four main roadmap fronts.

```text
docs/phases.md
```

Explains the development phases.

---

## Main setup categories

TFR setup is divided into these categories:

```text
GameMode setup
Player setup
PlayerController setup
NPC / civilian setup
World marker setup
Admin object setup
Radial setup
Persistence setup
Spawn setup
HALO setup
Enemy communications setup
```

Not every mission needs every system.

Optional systems should remain disabled or unconfigured when not used.

---

## GameMode setup

GameMode components control global systems.

Common GameMode components include:

```text
TFR_MissionManagerComponent
TFR_JournalTaskBridgeComponent
TFR_PersistenceManagerComponent
TFR_PlayerInventoryPersistenceComponent
TFR_HaloJumpComponent
TFR_EnemyCommsManagerComponent
TFR_OperationsRadialGameModeComponent
TFR_SpawnDirectorComponent
```

Not all of them are required.

Use only the systems needed for the mission.

---

## Minimal operations setup

Use this when you only want basic dynamic operations.

### GameMode

Recommended:

```text
TFR_MissionManagerComponent
```

Optional:

```text
TFR_JournalTaskBridgeComponent
```

### World markers

Recommended:

```text
TFR_SpawnAreaComponent
```

Place spawn areas for towns, military areas, industrial zones or other mission areas.

### NPCs / informants

For informants:

```text
TFR_InteractionComponent
```

For civilians or suspects that need identification/arrest logic:

```text
TFR_CivilDataComponent
TFR_SurrenderControlComponent optional
```

### Actions

Use where needed:

```text
TFR_Action_Identificar
TFR_Action_Interrogar
TFR_Action_Arrestar
```

---

## HALO setup

Use this when you want HALO insertion.

### GameMode

Required:

```text
TFR_HaloJumpComponent
```

### Player character

Required for map selection:

```text
TFR_HaloMapSelectionComponent
```

### Optional world marker

Use this if you want fixed HALO drop zones:

```text
TFR_HaloSpawnPointComponent
```

### Optional action or radial

HALO can be triggered from:

```text
TFR_HaloJumpUserAction
```

or through the radial system.

### Notes

HALO should remain server-validated.

The client may select a map position, but the server should approve and execute the jump.

---

## Persistence setup

Use this when you want operation or inventory persistence.

### Operation persistence

GameMode:

```text
TFR_PersistenceManagerComponent
TFR_MissionManagerComponent
```

The persistence manager saves and loads operation state through the mission manager.

### Player inventory persistence

GameMode:

```text
TFR_PlayerInventoryPersistenceComponent
```

Player character must have the normal inventory components required by Arma Reforger.

### Admin persistence actions

Persistence can be controlled through:

```text
TFR_MissionSettingAction
```

or through radial persistence entries.

### Important persistence rules

* Keep autosave intervals safe.
* Do not save too frequently.
* Do not overwrite valid saves when no player controlled entity exists.
* Keep restore retries limited.
* Keep debug logs controlled.
* Use admin diagnostics when testing.

---

## Radial setup

Use this when you want the dedicated TFR radial menu.

### GameMode

Required:

```text
TFR_OperationsRadialGameModeComponent
```

### Player character

Required:

```text
TFR_OperationsRadialComponent
```

### PlayerController

Recommended:

```text
TFR_OperationsRadialNetworkComponent
```

The network component can be tolerated on the controlled character, but PlayerController is preferred.

### Optional related systems

The radial can expose:

```text
HALO
Mission admin actions
Persistence actions
```

Only expose entries that are actually supported and intended for the player/admin role.

### Known review point

Player inventory radial entries exist in the enum, but they should be checked.

They should either be fully implemented through the network component or hidden until ready.

---

## Admin object setup

Use this when you want a physical radio, terminal, laptop, box or console for admin actions.

### Physical object

Add:

```text
TFR_MissionAdminConsoleComponent
```

Also add the related actions through the object action setup:

```text
TFR_MissionAdminActions
TFR_MissionSettingAction optional
```

### Setup notes

Configure:

```text
Enabled
Authorized Faction Key
Hide Actions If Unauthorized
```

Admin objects should call safe manager APIs.

They should not directly own mission or persistence state.

---

## NPC / civilian setup

Use these components depending on the NPC role.

### Civilian / suspect / arrest target

```text
TFR_CivilDataComponent
```

Optional:

```text
TFR_SurrenderControlComponent
```

Related actions:

```text
TFR_Action_Identificar
TFR_Action_Arrestar
```

### Informant

```text
TFR_InteractionComponent
```

Related action:

```text
TFR_Action_Interrogar
```

### Mood / aggression civilian

```text
TFR_MoodComponent
```

Use carefully.

Do not place mood logic on every civilian without performance testing.

---

## Spawn area setup

Use this when missions or spawn director need map-independent positions.

### World marker

Add:

```text
TFR_SpawnAreaComponent
```

Configure:

```text
Area Type
Radius
Enabled
Optional Area Name
```

Supported area types include:

```text
ANY
TOWN
MILITARY
INDUSTRIAL
ROAD
CHECKPOINT
BASE
COMPOUND
```

### Notes

The entity name is not the most important part.

The important setup values are:

```text
area type
radius
enabled state
optional exact name
```

This allows the addon to work on any map.

---

## Spawn director setup

Use this when you want predefined area-based spawns or persistent patrols.

### GameMode or manager entity

Add:

```text
TFR_SpawnDirectorComponent
```

### Required world setup

Place one or more:

```text
TFR_SpawnAreaComponent
```

### Configure rules

The spawn director can use:

```text
Spawn Rules
Vehicle Spawn Rules
Prefab Pools
Waypoint Prefabs
Persistent Patrol Settings
```

### Performance warning

Persistent patrols should be limited.

Vehicle patrol health checks and route refresh intervals should be safe for dedicated servers.

---

## Enemy communications setup

Use this when you want enemies to detect, alert or call support.

### GameMode

Add:

```text
TFR_EnemyCommsManagerComponent
```

### Optional NPC support

Add to civilians/enemies where needed:

```text
TFR_MoodComponent
```

### Optional runtime support

For artillery/CAS wrapper impacts:

```text
TFR_ArtilleryImpactComponent
```

usually on a runtime spawned impact prefab.

### Performance warning

Enemy communications can be performance-sensitive.

Configure carefully:

```text
scan interval
scan radius
alert radius
maximum responders
reinforcement cooldown
CAS/artillery cooldown
debug logs
```

Do not enable heavy communication systems if the mission does not need them.

---

## Recommended GameMode presets

The long-term goal is to provide prepared GameMode presets.

Planned presets:

```text
TFR_GameMode_Core.et
TFR_GameMode_Operations.et
TFR_GameMode_FullPersistent.et
```

### TFR_GameMode_Core.et

Minimal framework setup.

Should include only the basic required systems.

### TFR_GameMode_Operations.et

Standard operations setup.

Should include dynamic operations, mission flow, intel chain and common gameplay systems.

### TFR_GameMode_FullPersistent.et

Full persistent operations setup.

Should include operations, persistence and extended systems with safe dedicated server limits.

---

## Minimal setup checklist

Use this checklist for a basic operation mission.

### GameMode

```text
[ ] TFR_MissionManagerComponent added
[ ] Mission prefabs configured
[ ] Mission settings reviewed
[ ] Auto Start Intel Chain configured intentionally
```

### World

```text
[ ] TFR_SpawnAreaComponent markers placed
[ ] Area types configured
[ ] Area radius configured
[ ] Areas enabled
```

### NPCs

```text
[ ] Informant prefab has TFR_InteractionComponent
[ ] Informant has TFR_Action_Interrogar
[ ] Arrest targets have TFR_CivilDataComponent if needed
[ ] Arrest targets have TFR_Action_Arrestar if needed
[ ] Identification targets have TFR_Action_Identificar if needed
```

### Optional journal

```text
[ ] TFR_JournalTaskBridgeComponent added
[ ] LayerTask entity exists
[ ] LayerTask name matches the configured field
```

### Dedicated server

```text
[ ] Debug logs disabled unless testing
[ ] Optional systems disabled if unused
[ ] No unnecessary enemy comms enabled
[ ] No unnecessary persistence autoscan enabled
[ ] No heavy spawn director persistent patrols unless tested
```

---

## Full setup checklist

Use this checklist for a complete persistent operations scenario.

### GameMode

```text
[ ] TFR_MissionManagerComponent
[ ] TFR_JournalTaskBridgeComponent
[ ] TFR_PersistenceManagerComponent
[ ] TFR_PlayerInventoryPersistenceComponent optional
[ ] TFR_HaloJumpComponent optional
[ ] TFR_EnemyCommsManagerComponent optional
[ ] TFR_OperationsRadialGameModeComponent optional
[ ] TFR_SpawnDirectorComponent optional
```

### Player character

```text
[ ] TFR_HaloMapSelectionComponent if HALO map selection is used
[ ] TFR_OperationsRadialComponent if radial is used
```

### PlayerController

```text
[ ] TFR_OperationsRadialNetworkComponent recommended if radial admin actions are used
```

### World

```text
[ ] TFR_SpawnAreaComponent markers placed
[ ] TFR_HaloSpawnPointComponent optional
```

### Admin objects

```text
[ ] TFR_MissionAdminConsoleComponent optional
[ ] Mission admin actions optional
[ ] Persistence admin actions optional
```

### NPCs

```text
[ ] TFR_CivilDataComponent where needed
[ ] TFR_InteractionComponent where needed
[ ] TFR_MoodComponent only where needed
[ ] TFR_SurrenderControlComponent where needed
```

### Persistence

```text
[ ] Save directory reviewed
[ ] Operation save file reviewed
[ ] Player inventory save file reviewed
[ ] Autosave interval safe
[ ] Restore on start configured
[ ] Restore retries limited
[ ] Empty-player save protection enabled
```

### Performance

```text
[ ] Debug logs disabled by default
[ ] EnemyComms scan interval safe
[ ] Mood tick interval safe
[ ] SpawnDirector persistent patrol checks safe
[ ] Inventory autoscan interval safe
[ ] No repeated unnecessary CallLater loops
```

---

## Common setup mistakes

### Mistake: putting everything on GameMode

Not every component belongs on GameMode.

NPC state, interaction and mood usually belong on NPCs.
Player UI/radial components usually belong on player characters.
World marker components belong on marker entities.

---

### Mistake: enabling optional systems without using them

Optional systems should remain disabled or unconfigured unless needed.

Examples:

```text
Enemy communications
Player inventory persistence
Persistent vehicle patrols
Mood/scavenger on many NPCs
Radial admin entries for normal players
```

---

### Mistake: missing spawn areas

Dynamic missions and spawn director systems need map-independent placement.

If no spawn areas are placed, mission placement may fail or fall back to less ideal logic.

---

### Mistake: missing PlayerController network bridge for radial admin actions

Radial UI can open locally, but admin/persistence actions need server validation.

Use:

```text
TFR_OperationsRadialNetworkComponent
```

preferably on PlayerController.

---

### Mistake: unsafe persistence settings

Autosave that runs too often can hurt server performance.

Saving when no valid players exist can risk bad saves if protections are disabled.

Keep persistence conservative.

---

## Setup rule

Start minimal.

Add systems only when needed.

Recommended order:

```text
1. MissionManager
2. Spawn areas
3. NPC interaction
4. Journal bridge
5. HALO
6. Admin tools
7. Persistence
8. Radial
9. Enemy communications
10. Spawn director / persistent patrols
```

Compile and test after each system.

Do not enable everything at once.

---

## Final goal

The final goal is that a mission maker can set up TFR Framework without guessing.

The setup should be:

* clear;
* modular;
* map-independent;
* difficult to break;
* safe for dedicated servers;
* documented enough to use without reading the code.

If a component is hard to place correctly, the component needs better naming, better defaults, better validation warnings or better documentation.
