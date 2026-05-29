# TFR Framework Architecture

This document describes the current architecture of TFR Framework.

TFR Framework and TFR Operations are the same addon.
**TFR Operations** is the internal Workbench project name.
**TFR Framework** is the public Workshop / repository name.

The addon is designed to work on any map through configurable components, spawn areas, HALO points, GameMode systems and reusable mission logic.

---

## Main idea

TFR Framework is a modular addon for Arma Reforger.

It provides reusable systems for:

* dynamic operations;
* mission flow;
* safe placement;
* spawn areas;
* HALO insertion;
* persistence;
* interaction;
* enemy communications;
* radial/admin tools;
* dedicated server gameplay.

The addon should not be locked to one specific map.

Each mission maker or server owner should be able to configure the addon for their own scenario by placing the required components and prefabs.

---

## Current main blocks

The current code is organized around these main blocks:

* actions;
* components;
* missions;
* radial;
* spawn.

---

## Actions

Actions are user interaction scripts attached to entities through an ActionManagerComponent.

Current action systems include:

* identifying civilians or targets;
* interrogating informants;
* arresting suspects;
* requesting HALO insertion;
* mission admin actions;
* persistence admin actions.

Important current action scripts:

* `TFR_Action_Identificar`
* `TFR_Action_Interrogar`
* `TFR_Action_Arrestar`
* `TFR_HaloJumpUserAction`
* `TFR_MissionAdminActions`
* `TFR_MissionSettingAction`

Actions should stay lightweight.

They should not contain heavy mission logic directly.
When possible, actions should call components or managers that own the real system logic.

---

## Components

Components are the main building blocks of the addon.

Some components belong on the GameMode, some belong on player characters, some belong on NPCs, and some belong on physical objects such as radios, consoles or markers.

Current important components include:

* `TFR_ArtilleryImpactComponent`
* `TFR_CivilDataComponent`
* `TFR_EnemyCommsManagerComponent`
* `TFR_HaloJumpComponent`
* `TFR_HaloMapSelectionComponent`
* `TFR_InteractionComponent`
* `TFR_MissionAdminConsoleComponent`
* `TFR_MoodComponent`
* `TFR_PersistenceManagerComponent`
* `TFR_PlayerInventoryPersistenceComponent`
* `TFR_SurrenderControlComponent`

---

## GameMode components

These components are intended to live on the GameMode or on GameMode-related entities.

Current GameMode-related systems include:

* mission manager;
* journal bridge;
* persistence manager;
* player inventory persistence;
* HALO manager;
* radial menu holder;
* enemy communications manager;
* spawn director, when used globally.

Examples:

* `TFR_MissionManagerComponent`
* `TFR_JournalTaskBridgeComponent`
* `TFR_PersistenceManagerComponent`
* `TFR_PlayerInventoryPersistenceComponent`
* `TFR_HaloJumpComponent`
* `TFR_OperationsRadialGameModeComponent`
* `TFR_EnemyCommsManagerComponent`
* `TFR_SpawnDirectorComponent`

These components must be documented clearly because they are the most important part of the setup.

---

## Player / character components

These components are intended to live on player characters or controlled characters.

Current player-related systems include:

* HALO map selection;
* operations radial control;
* optional radial network fallback if not placed on PlayerController.

Examples:

* `TFR_HaloMapSelectionComponent`
* `TFR_OperationsRadialComponent`

The radial network component is recommended on PlayerController, but can be tolerated on the controlled character if needed:

* `TFR_OperationsRadialNetworkComponent`

---

## NPC / civilian components

These components are intended to live on civilians, informants, suspects or mission targets.

Current NPC-related systems include:

* civil data;
* interrogation;
* mood/aggression;
* surrender control.

Examples:

* `TFR_CivilDataComponent`
* `TFR_InteractionComponent`
* `TFR_MoodComponent`
* `TFR_SurrenderControlComponent`

These components are used by actions such as identify, interrogate and arrest.

---

## Physical object components

These components are intended to live on physical objects placed in the world, such as radios, terminals, laptops, boxes or admin consoles.

Current physical-object systems include:

* mission admin console;
* action holder for mission admin actions;
* action holder for persistence actions.

Example:

* `TFR_MissionAdminConsoleComponent`

The entity should also have the required user actions in its ActionManagerComponent.

---

## Spawn and map-independent placement

The addon is intended to work on any map.

Instead of hardcoding locations for one terrain, the addon uses spawn markers and area components.

Current spawn-related systems include:

* `TFR_SpawnAreaComponent`
* `TFR_SpawnDirectorComponent`
* `TFR_HaloSpawnPointComponent`

`TFR_SpawnAreaComponent` marks usable areas in the world.

Supported area types currently include:

* `ANY`
* `TOWN`
* `MILITARY`
* `INDUSTRIAL`
* `ROAD`
* `CHECKPOINT`
* `BASE`
* `COMPOUND`

The name of the entity is not the most important part.
The important setup values are:

* area type;
* radius;
* enabled state;
* optional area name.

This allows the same addon systems to be adapted to different maps.

---

## HALO system

Current HALO-related systems include:

* `TFR_HaloJumpComponent`
* `TFR_HaloMapSelectionComponent`
* `TFR_HaloSpawnPointComponent`
* `TFR_HaloJumpUserAction`
* HALO radial entry.

The HALO flow is:

1. Player requests HALO.
2. Client opens map selection when using map mode.
3. Player selects a drop zone.
4. Server validates the request.
5. Server calculates approved jump position.
6. Server teleports the player.
7. Client owner assist helps ensure the player reaches the approved position in dedicated server conditions.
8. Auto deploy can request parachute deployment when enabled.

HALO must remain server-safe and should not rely on heavy loops.

---

## Mission system

Current mission-related systems include:

* `TFR_MissionManagerComponent`
* `TFR_JournalTaskBridgeComponent`
* `TFR_InteractionComponent`
* `TFR_CivilDataComponent`
* `TFR_MoodComponent`
* mission admin actions;
* identify/interrogate/arrest actions.

The mission manager currently handles:

* Operation Lite;
* intel chains;
* informant intel jobs;
* patrol intel jobs;
* main mission creation;
* mission types;
* rewards;
* convoy mission logic;
* reinforcement waves;
* betrayal / hunt waves;
* admin recovery;
* persistence restore hooks;
* journal publishing.

Mission-related systems belong mainly to the mission and operations front of the roadmap.

The mission manager is one of the largest systems and must be reviewed carefully during cleanup so it does not become too difficult to configure.

---

## Persistence system

Current persistence-related systems include:

* `TFR_PersistenceManagerComponent`
* `TFR_PlayerInventoryPersistenceComponent`
* `TFR_MissionSettingAction`
* radial persistence actions;
* MissionManager restore hooks.

Persistence currently covers:

* operation state;
* intel chain state;
* active mission state;
* player inventory persistence;
* admin resets;
* load on start;
* autosave;
* restore retry logic.

Persistence must remain server-friendly.

Important rules:

* do not save too often;
* do not overwrite good data with empty player states;
* keep restore retries controlled;
* keep autosave intervals safe;
* keep debug logs controlled;
* make admin diagnostics clear.

---

## Enemy communications and mood

Current related systems include:

* `TFR_EnemyCommsManagerComponent`
* `TFR_MoodComponent`
* `TFR_ArtilleryImpactComponent`

Enemy communications can:

* detect BLUFOR contacts;
* alert nearby enemies;
* force mood/aggression;
* request reinforcements;
* request CAS/artillery when enabled.

Mood can:

* move civilians between calm, nervous, scared and aggressive states;
* change faction when needed;
* activate hostile lock;
* enable scavenger behaviour to pick up weapons.

These systems are powerful but performance-sensitive.

They must keep clear limits for:

* scan radius;
* scan interval;
* cooldowns;
* active responders;
* CAS cooldown;
* debug logs;
* server-only execution.

---

## Radial system

Current radial-related systems include:

* `TFR_OperationsRadialMenu`
* `TFR_OperationsRadialController`
* `TFR_OperationsRadialGameModeComponent`
* `TFR_OperationsRadialComponent`
* `TFR_OperationsRadialEntry`
* `TFR_OperationsRadialNetworkComponent`

The radial system provides access to:

* HALO map selection;
* mission admin actions;
* persistence actions.

The radial menu is dedicated to TFR and does not use the global vanilla radial menu.

Sensitive actions must go through the radial network component so the server can validate the request.

Known review point:

Some player inventory radial entries are declared in the entry enum. These should either be fully implemented in the network component or hidden until ready.

---

## Current dependency overview

High-level dependencies:

* `TFR_Action_Identificar` depends on `TFR_CivilDataComponent`.
* `TFR_Action_Arrestar` depends on `TFR_CivilDataComponent`, `TFR_SurrenderControlComponent` and `TFR_MissionManagerComponent`.
* `TFR_Action_Interrogar` depends on `TFR_InteractionComponent`.
* `TFR_InteractionComponent` depends on `TFR_MissionManagerComponent`.
* `TFR_MissionManagerComponent` uses `TFR_SpawnAreaComponent`.
* `TFR_MissionManagerComponent` can publish to `TFR_JournalTaskBridgeComponent`.
* `TFR_PersistenceManagerComponent` restores through `TFR_MissionManagerComponent`.
* `TFR_PlayerInventoryPersistenceComponent` depends on player inventory components.
* `TFR_HaloMapSelectionComponent` depends on `TFR_HaloJumpComponent`.
* `TFR_HaloJumpComponent` can use `TFR_HaloSpawnPointComponent`.
* `TFR_EnemyCommsManagerComponent` can interact with `TFR_MoodComponent`.
* Radial admin entries go through `TFR_OperationsRadialNetworkComponent`.

---

## Roadmap front mapping

### Front 1 — Safe Position / Safe Placement

Related current systems:

* `TFR_SpawnAreaComponent`
* `TFR_SpawnDirectorComponent`
* `TFR_HaloSpawnPointComponent`
* `TFR_HaloJumpComponent`
* `TFR_HaloMapSelectionComponent`
* `TFR_ArtilleryImpactComponent`

### Front 2 — Mission system for mission makers

Related current systems:

* `TFR_MissionManagerComponent`
* `TFR_JournalTaskBridgeComponent`
* `TFR_InteractionComponent`
* `TFR_CivilDataComponent`
* `TFR_MoodComponent`
* `TFR_MissionAdminConsoleComponent`
* identify/interrogate/arrest actions

### Front 3 — Stable persistence

Related current systems:

* `TFR_PersistenceManagerComponent`
* `TFR_PlayerInventoryPersistenceComponent`
* `TFR_MissionSettingAction`
* radial persistence actions
* MissionManager restore hooks

### Front 4 — Simplification, cleanup and optimization

Related current systems:

* `TFR_MissionManagerComponent`
* `TFR_PersistenceManagerComponent`
* `TFR_PlayerInventoryPersistenceComponent`
* `TFR_EnemyCommsManagerComponent`
* `TFR_MoodComponent`
* `TFR_SpawnDirectorComponent`
* radial system
* GameMode presets
* example prefabs
* setup checklist

---

## Architecture rule

TFR Framework must remain map-independent.

Map-specific setup should be done through placed components, spawn areas, HALO points, prefabs and configuration.

The code should avoid hardcoding one terrain, one scenario or one mission layout.

The final goal is that a mission maker can adapt the addon to any map by placing and configuring the right components, without needing to understand the internal code.
