# TFR Framework Folder Structure

This document defines the official script folder structure for TFR Framework.

TFR Framework and TFR Operations are the same addon.

* **TFR Operations** is the internal Workbench project name.
* **TFR Framework** is the public Workshop / repository name.

The goal of this document is to keep the addon structure clear, stable and easy to understand without moving scripts unnecessarily.

---

## Official script structure

The current script structure is accepted as the official structure for now.

```text
scripts/Game/TFR/
├─ Actions/
├─ Components/
├─ Missions/
├─ Radial/
└─ Spawn/
```

This structure is simple, clear and already separates the main systems of the addon.

No additional folders should be created unless there is a real need later.

The priority is not to create more folders.
The priority is to keep the addon easy to configure, easy to debug and safe for dedicated servers.

---

## Folder rule

Do not move scripts just for cosmetic reasons.

Moving scripts in Arma Reforger Workbench can break references, prefabs or dependencies if done too quickly.

The current structure is valid and should remain stable unless a future system becomes too large or difficult to maintain.

Code cleanup should focus first on:

* safer defaults;
* clearer component setup;
* better validation warnings;
* reduced debug spam;
* dedicated server performance;
* documentation of where each component belongs;
* avoiding unnecessary scans;
* avoiding duplicated `CallLater` loops;
* keeping optional systems disabled when not used.

Folder cleanup and code refactor must be separate tasks.

---

## Actions folder

Official folder:

```text
scripts/Game/TFR/Actions/
```

Purpose:

This folder contains user actions attached to entities through an `ActionManagerComponent`.

Actions should be small and should not contain heavy mission logic directly.

Current action scripts:

```text
TFR_Action_Arrestar.c
TFR_Action_Identificar.c
TFR_Action_Interrogar.c
TFR_HaloJumpUserAction.c
TFR_MissionAdminActions.c
TFR_MissionSettingAction.c
```

### Current responsibilities

`TFR_Action_Identificar.c`

Identifies civilians or mission targets.

It depends mainly on:

```text
TFR_CivilDataComponent
```

It is part of the identification/arrest/interrogation flow.

---

`TFR_Action_Interrogar.c`

Allows the player to interrogate an entity.

It depends mainly on:

```text
TFR_InteractionComponent
```

The action itself should stay lightweight.
The interrogation logic belongs in the interaction component and mission systems.

---

`TFR_Action_Arrestar.c`

Allows a suspect or target to be arrested or forced into surrender flow.

It depends on:

```text
TFR_CivilDataComponent
TFR_SurrenderControlComponent
TFR_MissionManagerComponent
SCR_CharacterControllerComponent
```

It connects the arrest action with mission completion logic.

---

`TFR_HaloJumpUserAction.c`

Allows a player to request HALO insertion through a user action.

It belongs in `Actions/` even though it is related to HALO, because it is still a user action.

It should call HALO components instead of containing the full HALO system itself.

---

`TFR_MissionAdminActions.c`

Contains admin actions for mission control.

These actions are intended for radios, terminals, admin consoles or similar objects.

They should call admin components or mission manager APIs instead of modifying mission state directly.

---

`TFR_MissionSettingAction.c`

Contains admin actions for persistence control.

Examples:

* save operation;
* load operation;
* reset operation persistence;
* reset player inventory persistence;
* reset vehicle cargo persistence;
* reset all persistence;
* print persistence state.

These actions depend on:

```text
TFR_PersistenceManagerComponent
```

### Rules for actions

Actions should:

* stay lightweight;
* call components/managers for real logic;
* avoid duplicated logic;
* avoid heavy scans;
* avoid persistent loops;
* avoid debug spam;
* provide useful feedback when possible;
* be easy to understand in Workbench.

Actions should not:

* own mission state;
* own persistence state;
* run expensive systems;
* bypass server validation for sensitive actions.

---

## Components folder

Official folder:

```text
scripts/Game/TFR/Components/
```

Purpose:

This folder contains the main gameplay components of the addon.

Some components go on GameMode.
Some go on player characters.
Some go on NPCs or mission entities.
Some go on physical objects like radios or consoles.

Current component scripts:

```text
TFR_ArtilleryImpactComponent.c
TFR_CivilDataComponent.c
TFR_EnemyCommsManagerComponent.c
TFR_HaloJumpComponent.c
TFR_HaloMapSelectionComponent.c
TFR_InteractionComponent.c
TFR_MissionAdminConsoleComponent.c
TFR_MoodComponent.c
TFR_PersistenceManagerComponent.c
TFR_PlayerInventoryPersistenceComponent.c
TFR_SurrenderControlComponent.c
```

This folder is intentionally broad.

At the moment, it is better to keep these systems together than to split them into too many folders.

---

## GameMode components

The following components are intended to live on the GameMode or on GameMode-related entities:

```text
TFR_EnemyCommsManagerComponent.c
TFR_HaloJumpComponent.c
TFR_PersistenceManagerComponent.c
TFR_PlayerInventoryPersistenceComponent.c
```

Also related GameMode systems exist in other folders:

```text
TFR_MissionManagerComponent.c
TFR_JournalTaskBridgeComponent.c
TFR_OperationsRadialGameModeComponent.c
TFR_SpawnDirectorComponent.c
```

### GameMode component rules

GameMode components should:

* be server-safe;
* validate their settings;
* use safe defaults;
* avoid debug spam;
* clean `CallLater` calls in `OnDelete`;
* avoid expensive loops when disabled;
* explain missing setup through useful warnings;
* be documented clearly.

GameMode components are the most important for setup.

A mission maker should not have to guess which components belong on the GameMode.

---

## Player / character components

The following components are intended for player characters or controlled entities:

```text
TFR_HaloMapSelectionComponent.c
```

Related radial player component:

```text
TFR_OperationsRadialComponent.c
```

The radial network component is recommended on PlayerController, but can be tolerated on the controlled character when needed:

```text
TFR_OperationsRadialNetworkComponent.c
```

### Player component rules

Player components should:

* avoid running on dedicated server when they are UI-only;
* avoid sensitive server logic on the client;
* use RPC/server validation when needed;
* clean input listeners and callbacks;
* avoid repeated client-side spam;
* provide clear local hints.

---

## NPC / civilian components

The following components are intended for NPCs, civilians, informants, suspects or mission targets:

```text
TFR_CivilDataComponent.c
TFR_InteractionComponent.c
TFR_MoodComponent.c
TFR_SurrenderControlComponent.c
```

### Current responsibilities

`TFR_CivilDataComponent.c`

Stores civil/target data such as:

* enabled state;
* arrest order;
* identified state;
* surrender permission.

Used by:

```text
TFR_Action_Identificar
TFR_Action_Arrestar
```

---

`TFR_InteractionComponent.c`

Handles interrogation logic.

It can:

* provide intel;
* mark itself as consumed;
* trigger betrayal;
* call mission manager;
* start hunt waves through mission manager.

Used by:

```text
TFR_Action_Interrogar
TFR_MissionManagerComponent
```

---

`TFR_MoodComponent.c`

Handles civilian mood and aggression.

It can move an NPC between:

```text
CALM
NERVOUS
SCARED
AGGRESSIVE
```

It can also:

* change real faction;
* activate hostile lock;
* start scavenger behaviour;
* force aggression from enemy communications.

This component is performance-sensitive and must be reviewed carefully.

---

`TFR_SurrenderControlComponent.c`

Small component that tracks whether the entity can surrender.

Used by:

```text
TFR_Action_Arrestar
```

### NPC component rules

NPC components should:

* be optional when possible;
* use safe defaults;
* avoid heavy loops per NPC;
* avoid debug spam;
* avoid world scans unless clearly limited;
* document what prefab/entity they belong to.

---

## Physical object / admin components

The following component is intended for physical admin objects such as radios, laptops, terminals, boxes or consoles:

```text
TFR_MissionAdminConsoleComponent.c
```

It controls access to mission admin actions.

It should be used together with action scripts in an `ActionManagerComponent`.

### Admin object rules

Admin objects should:

* validate faction or permission;
* avoid client-side sensitive execution;
* call mission manager or persistence manager APIs;
* provide useful logs only when needed;
* stay optional.

---

## HALO components inside current structure

HALO scripts currently live in different folders depending on their type:

```text
scripts/Game/TFR/Components/TFR_HaloJumpComponent.c
scripts/Game/TFR/Components/TFR_HaloMapSelectionComponent.c
scripts/Game/TFR/Spawn/TFR_HaloSpawnPointComponent.c
scripts/Game/TFR/Actions/TFR_HaloJumpUserAction.c
```

This is accepted.

They should not be moved just because they are all HALO-related.

The current placement makes sense:

* HALO action stays in `Actions/`;
* HALO manager and map selection stay in `Components/`;
* HALO spawn point stays in `Spawn/`.

### HALO rules

HALO systems should:

* keep actual jump execution server-side;
* allow client map selection only as a request flow;
* validate positions on server;
* avoid heavy loops;
* keep retry systems limited;
* clean `CallLater` calls;
* work on dedicated server.

---

## Persistence components inside current structure

Persistence scripts currently live in:

```text
scripts/Game/TFR/Components/TFR_PersistenceManagerComponent.c
scripts/Game/TFR/Components/TFR_PlayerInventoryPersistenceComponent.c
scripts/Game/TFR/Actions/TFR_MissionSettingAction.c
```

This is accepted.

They should not be moved for now.

### Persistence responsibilities

`TFR_PersistenceManagerComponent.c`

Handles operation persistence:

* operation state;
* intel chain state;
* active mission state;
* save/load;
* admin resets;
* restore hooks through MissionManager.

---

`TFR_PlayerInventoryPersistenceComponent.c`

Handles player inventory persistence:

* scans player inventories;
* saves player inventory JSON;
* restores inventory storage slot items;
* avoids duplicate prefab counts;
* blocks autosave while restore is pending;
* avoids overwriting valid saves with invalid empty player states.

### Persistence rules

Persistence must:

* run server-side by default;
* use safe autosave intervals;
* avoid overwriting valid saves with empty data;
* avoid heavy scans;
* keep restore retry limits;
* provide admin diagnostics;
* avoid debug spam in normal gameplay.

---

## Missions folder

Official folder:

```text
scripts/Game/TFR/Missions/
```

Purpose:

This folder contains the dynamic operation and mission flow.

Current mission scripts:

```text
TFR_JournalTaskBridgeComponent.c
TFR_MissionManagerComponent.c
```

### Current responsibilities

`TFR_MissionManagerComponent.c`

Main system for operations and missions.

It currently handles:

* Operation Lite;
* intel chains;
* informant jobs;
* patrol jobs;
* main mission creation;
* mission types;
* convoy mission logic;
* rewards;
* reinforcement waves;
* betrayal/hunt waves;
* admin recovery;
* restore hooks for persistence;
* communication with journal bridge.

This is currently one of the largest and most important scripts in the addon.

---

`TFR_JournalTaskBridgeComponent.c`

Bridge between TFR missions and Scenario Framework task/journal systems.

It updates the main task/journal state for:

* intel spawned;
* intel received;
* mission created;
* mission updated;
* mission completed;
* mission failed;
* admin reset.

### Mission rules

Mission scripts should:

* run server-side where needed;
* keep mission state clear;
* expose safe public APIs for actions/admin/persistence;
* avoid direct UI dependencies where possible;
* use the journal bridge for task updates;
* avoid becoming impossible to configure.

The MissionManager should be reviewed carefully during cleanup, but it should not be moved just for folder cosmetics.

---

## Radial folder

Official folder:

```text
scripts/Game/TFR/Radial/
```

Purpose:

This folder contains the dedicated TFR radial menu system.

Current radial scripts:

```text
TFR_OperationsRadialComponent.c
TFR_OperationsRadialController.c
TFR_OperationsRadialEntry.c
TFR_OperationsRadialGameModeComponent.c
TFR_OperationsRadialMenu.c
TFR_OperationsRadialNetworkComponent.c
```

### Current responsibilities

`TFR_OperationsRadialMenu.c`

Dedicated TFR radial menu.

It does not use the global vanilla radial menu.

---

`TFR_OperationsRadialController.c`

Controls radial opening/closing and input handling.

It handles:

* input registration;
* analog value filtering;
* toggle open/close;
* avoiding double opening;
* listener cleanup.

---

`TFR_OperationsRadialGameModeComponent.c`

GameMode component that registers the dedicated radial menu on clients.

It avoids UI registration on dedicated server.

---

`TFR_OperationsRadialComponent.c`

Player/character component that controls and fills the radial menu.

It creates entries for:

* HALO;
* mission admin;
* persistence admin.

---

`TFR_OperationsRadialEntry.c`

Defines radial entries and sends actions.

HALO opens the local map selection flow.
Mission and persistence actions go through the network component.

---

`TFR_OperationsRadialNetworkComponent.c`

Network bridge for radial actions.

It validates the request on server and calls:

```text
TFR_MissionManagerComponent
TFR_PersistenceManagerComponent
```

### Known radial review point

`TFR_OperationsRadialEntry.c` declares player inventory radial entries.

These should be checked.

They should either be:

* fully implemented through the network component; or
* hidden/removed until ready.

### Radial rules

Radial scripts should:

* keep UI client-side;
* avoid UI work on dedicated server;
* route sensitive actions through server validation;
* avoid duplicate input listeners;
* clean callbacks;
* expose only working entries;
* avoid debug spam.

---

## Spawn folder

Official folder:

```text
scripts/Game/TFR/Spawn/
```

Purpose:

This folder contains map-independent placement and spawn systems.

Current spawn scripts:

```text
TFR_HaloSpawnPointComponent.c
TFR_SpawnAreaComponent.c
TFR_SpawnDirectorComponent.c
```

### Current responsibilities

`TFR_HaloSpawnPointComponent.c`

Physical HALO insertion point.

It:

* registers active HALO points;
* supports priority;
* has no loops;
* does not spawn anything directly.

---

`TFR_SpawnAreaComponent.c`

Map-independent area marker.

It marks usable areas with:

* area type;
* radius;
* enabled state;
* optional area name.

Supported area types:

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

The entity name is not the most important part.
The important setup values are the area type, radius and enabled state.

---

`TFR_SpawnDirectorComponent.c`

Director for modular spawns by area.

It can handle:

* group spawn rules;
* vehicle spawn rules;
* prefab pools;
* crew spawning;
* waypoints;
* patrol modes;
* persistent vehicle patrols;
* patrol respawn;
* route refresh.

This script is powerful and performance-sensitive.

### Spawn rules

Spawn scripts should:

* remain map-independent;
* use placed area components instead of hardcoded positions;
* avoid continuous scans;
* run server-side where needed;
* keep patrol checks limited;
* avoid unsafe spawn positions where possible;
* avoid debug spam;
* clean spawned/persistent entities when needed.

---

## Current folder decision

The current folder structure is official and accepted:

```text
scripts/Game/TFR/
├─ Actions/
├─ Components/
├─ Missions/
├─ Radial/
└─ Spawn/
```

We are not creating these folders for now:

```text
Admin/
Core/
HALO/
Persistence/
Utilities/
```

They may be added in the future only if there is a real need.

Examples of when a new folder may become justified:

* a system becomes too large;
* many shared helpers appear;
* persistence grows into several independent files;
* admin systems become complex;
* HALO grows beyond the current simple set;
* utilities become reusable across many systems.

Until then, keeping the structure simple is better.

---

## Cleanup priority without moving files

Since the current folder structure is accepted, the next cleanup work should focus on behaviour and setup quality.

Priority areas:

### 1. Component placement documentation

Document where each component belongs:

* GameMode;
* player character;
* PlayerController;
* NPC/civilian;
* physical object;
* world marker.

---

### 2. Safe defaults

Review attributes and defaults.

Important examples:

* debug should be disabled by default;
* optional systems should be disabled if not required;
* scan intervals should be safe;
* autosave intervals should be safe;
* max counts should have limits.

---

### 3. Validation warnings

Add clear warnings when setup is incomplete.

Warnings should explain exactly what is missing.

Bad warning:

```text
Error.
```

Good warning:

```text
TFR_MissionManagerComponent: Intel Civil Prefab is empty. Informant intel jobs cannot spawn.
```

---

### 4. Dedicated server safety

Review:

* `System.IsConsoleApp()`;
* `Replication.IsServer()`;
* `Run Server Only`;
* client UI guards;
* server-only mission logic;
* cleanup in `OnDelete`.

---

### 5. Performance safety

Review:

* `CallLater` loops;
* repeated timers;
* world scans;
* `QueryEntitiesByAABB`;
* autosave intervals;
* mood ticks;
* enemy comms scans;
* persistent patrol checks;
* debug logs.

---

### 6. Documentation and setup checklist

The final setup must be clear enough that a mission maker can use the addon without knowing the internal code.

The documentation should eventually explain:

* minimum setup;
* full setup;
* required GameMode components;
* optional components;
* player prefab requirements;
* NPC prefab requirements;
* admin console setup;
* radial setup;
* spawn area setup;
* persistence setup;
* performance limits.

---

## Do not do this now

Do not rename classes.

Do not move scripts.

Do not split MissionManager yet.

Do not split SpawnDirector yet.

Do not rewrite persistence yet.

Do not change prefab setup while also changing code.

Do not optimize everything at once.

Each cleanup step should be isolated and tested.

---

## Final goal

The current structure should support the four roadmap fronts:

1. Safe Position / Safe Placement
2. Mission system for mission makers
3. Stable persistence
4. Simplification, cleanup and optimization

The addon should remain:

* map-independent;
* modular;
* clear in Workbench;
* difficult to break;
* lightweight on dedicated servers;
* understandable for mission makers;
* stable enough for long operations.

The current folder structure is good enough to continue development.

The next work should focus on making each existing system safer, clearer and better documented.
