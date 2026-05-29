# TFR Framework Components

This document lists the current components and main scripts used by TFR Framework.

TFR Framework and TFR Operations are the same addon.

* **TFR Operations** is the internal Workbench project name.
* **TFR Framework** is the public Workshop / repository name.

This document is a technical component index.

For placement details, see:

```text
docs/component-placement.md
```

For setup steps, see:

```text
docs/setup.md
```

For architecture overview, see:

```text
docs/architecture.md
```

---

## Component philosophy

TFR Framework should avoid forcing mission makers to guess where each component belongs.

Each component should have:

* a clear purpose;
* safe default values;
* useful validation warnings;
* limited performance cost;
* clear Workbench naming;
* documented placement;
* predictable server/client behaviour.

Optional systems should remain disabled or harmless when not used.

---

## Current script groups

The addon currently uses this official structure:

```text
scripts/Game/TFR/
├─ Actions/
├─ Components/
├─ Missions/
├─ Radial/
└─ Spawn/
```

The current structure is accepted and should not be changed just for cosmetic reasons.

---

## Component categories

Current systems are grouped into these categories:

```text
Actions
GameMode components
Player / character components
PlayerController components
NPC / civilian components
Physical admin object components
World marker components
Runtime spawned prefab components
Mission systems
Radial systems
Spawn systems
Persistence systems
Performance-sensitive systems
```

---

# Actions

Actions are user interaction scripts.

They are usually placed through an `ActionManagerComponent`.

Actions should stay lightweight and call components or managers for real logic.

---

## TFR_Action_Identificar

File:

```text
scripts/Game/TFR/Actions/TFR_Action_Identificar.c
```

Type:

```text
User action
```

Placement:

```text
ActionManagerComponent on NPC / civilian / target
```

Main purpose:

Identifies a civilian or target.

Main dependencies:

```text
TFR_CivilDataComponent
SCR_CharacterControllerComponent
SCR_CharacterFactionAffiliationComponent
```

Used by:

```text
Civilian identification flow
Arrest flow prerequisite
```

Status:

```text
Active
```

Risk level:

```text
Low
```

Review notes:

* Action is lightweight.
* Uses `TFR_CivilDataComponent`.
* Blocks USSR faction.
* Blocks dead/unconscious targets.
* Good candidate for keeping as-is unless naming or hints need cleanup.

---

## TFR_Action_Interrogar

File:

```text
scripts/Game/TFR/Actions/TFR_Action_Interrogar.c
```

Type:

```text
User action
```

Placement:

```text
ActionManagerComponent on NPC / informant
```

Main purpose:

Allows interrogation of an NPC or informant.

Main dependencies:

```text
TFR_InteractionComponent
```

Used by:

```text
Intel chain
Informant flow
MissionManager through InteractionComponent
```

Status:

```text
Active
```

Risk level:

```text
Low / Medium
```

Review notes:

* Action correctly delegates logic to `TFR_InteractionComponent`.
* Should stay lightweight.
* Good structure for future actions.

---

## TFR_Action_Arrestar

File:

```text
scripts/Game/TFR/Actions/TFR_Action_Arrestar.c
```

Type:

```text
User action
```

Placement:

```text
ActionManagerComponent on suspect / target
```

Main purpose:

Allows arrest/surrender flow for identified targets with arrest order.

Main dependencies:

```text
TFR_CivilDataComponent
TFR_SurrenderControlComponent
SCR_CharacterControllerComponent
TFR_MissionManagerComponent
```

Used by:

```text
Arrest mission flow
Suspect surrender flow
MissionManager target arrested notification
```

Status:

```text
Active
```

Risk level:

```text
Medium
```

Review notes:

* Depends on multiple components.
* Uses ACE surrender call through character controller.
* Should keep clear warnings when components are missing.
* Should remain server-safe.

---

## TFR_HaloJumpUserAction

File:

```text
scripts/Game/TFR/Actions/TFR_HaloJumpUserAction.c
```

Type:

```text
User action
```

Placement:

```text
ActionManagerComponent on configured HALO interaction source
```

Main purpose:

Starts HALO insertion request or map selection flow.

Main dependencies:

```text
TFR_HaloMapSelectionComponent
TFR_HaloJumpComponent
```

Used by:

```text
HALO insertion system
```

Status:

```text
Active
```

Risk level:

```text
Medium
```

Review notes:

* Should not execute sensitive HALO logic directly.
* Should start local selection/request and let server validate.
* Retry logic should remain limited.

---

## TFR_MissionAdminActions

File:

```text
scripts/Game/TFR/Actions/TFR_MissionAdminActions.c
```

Type:

```text
Admin user actions
```

Placement:

```text
ActionManagerComponent on admin radio / console / terminal / object
```

Main purpose:

Allows mission admin actions through a physical object.

Main dependencies:

```text
TFR_MissionAdminConsoleComponent
TFR_MissionManagerComponent
```

Used by:

```text
Admin mission diagnostics
Force next intel
Complete mission
Fail mission
Restart intel chain
```

Status:

```text
Active
```

Risk level:

```text
Medium
```

Review notes:

* Admin actions should always validate access.
* Sensitive logic should stay in server-side manager/component APIs.
* Good place to keep physical admin interaction separate from radial admin interaction.

---

## TFR_MissionSettingAction

File:

```text
scripts/Game/TFR/Actions/TFR_MissionSettingAction.c
```

Type:

```text
Admin user actions
```

Placement:

```text
ActionManagerComponent on admin radio / console / terminal / object
```

Main purpose:

Allows persistence admin actions through a physical object.

Main dependencies:

```text
TFR_PersistenceManagerComponent
```

Used by:

```text
Save operation
Load operation
Reset operation persistence
Reset player inventory persistence
Reset vehicle cargo persistence
Reset all persistence
Print persistence state
```

Status:

```text
Active
```

Risk level:

```text
Medium
```

Review notes:

* Should keep persistence actions safe and clear.
* Should not bypass manager validation.
* Useful for testing persistence on dedicated server.

---

# GameMode components

GameMode components control global systems.

These are usually the most important for setup.

---

## TFR_MissionManagerComponent

File:

```text
scripts/Game/TFR/Missions/TFR_MissionManagerComponent.c
```

Type:

```text
GameMode component
Mission system
Operation manager
```

Placement:

```text
GameMode
```

Main purpose:

Main dynamic mission and operation manager.

Main responsibilities:

* Operation Lite;
* intel chains;
* informant jobs;
* patrol jobs;
* mission selection;
* mission spawning;
* mission state;
* rewards;
* convoy missions;
* reinforcements;
* betrayal/hunt waves;
* admin recovery;
* persistence restore hooks;
* journal publishing.

Main dependencies:

```text
TFR_SpawnAreaComponent
TFR_InteractionComponent
TFR_CivilDataComponent
TFR_JournalTaskBridgeComponent optional
TFR_PersistenceManagerComponent optional
```

Used by:

```text
TFR_Action_Arrestar
TFR_InteractionComponent
TFR_MissionAdminConsoleComponent
TFR_OperationsRadialNetworkComponent
TFR_PersistenceManagerComponent
TFR_JournalTaskBridgeComponent
```

Status:

```text
Active
Core operations system
```

Risk level:

```text
High
```

Reason:

This is one of the largest scripts and has many configuration fields.

Review priorities:

* safer defaults;
* better validation warnings;
* clearer missing-prefab messages;
* clear required vs optional settings;
* simplify Workbench setup;
* avoid too many critical fields being required at once;
* keep mission check loop controlled;
* keep restore hooks stable.

---

## TFR_JournalTaskBridgeComponent

File:

```text
scripts/Game/TFR/Missions/TFR_JournalTaskBridgeComponent.c
```

Type:

```text
GameMode component
Journal / task bridge
```

Placement:

```text
GameMode
```

Main purpose:

Updates Scenario Framework task/journal from TFR mission events.

Main responsibilities:

* publish intel spawned;
* publish intel received;
* publish mission created;
* publish mission updated;
* mark mission completed;
* mark mission failed;
* reset journal state for new intel chain.

Main dependencies:

```text
SCR_ScenarioFrameworkLayerTask
SCR_ScenarioFrameworkTask
LayerTask1 or configured task entity name
TFR_MissionManagerComponent
```

Used by:

```text
TFR_MissionManagerComponent
```

Status:

```text
Active
Optional
```

Risk level:

```text
Medium
```

Review priorities:

* clear warning if LayerTask is missing;
* clear warning if task component is missing;
* avoid intel text overwriting mission phase;
* keep server-only behaviour.

---

## TFR_PersistenceManagerComponent

File:

```text
scripts/Game/TFR/Components/TFR_PersistenceManagerComponent.c
```

Type:

```text
GameMode component
Persistence manager
```

Placement:

```text
GameMode
```

Main purpose:

Saves and loads operation state.

Main responsibilities:

* save operation state;
* load operation state;
* reset operation persistence;
* reset player inventory persistence file;
* reset vehicle cargo persistence file;
* reset all persistence;
* print admin diagnostics;
* call MissionManager restore API.

Main dependencies:

```text
TFR_MissionManagerComponent
FileIO
SCR_JsonSaveContext
SCR_JsonLoadContext
```

Used by:

```text
TFR_MissionSettingAction
TFR_OperationsRadialNetworkComponent
TFR_MissionManagerComponent restore flow
```

Status:

```text
Active
Optional
```

Risk level:

```text
High
```

Reason:

Persistence can cause data loss if bad state overwrites good state.

Review priorities:

* autosave interval safety;
* clear load delay;
* clear reset behaviour;
* no save spam;
* no debug spam;
* server-only by default;
* useful admin diagnostics.

---

## TFR_PlayerInventoryPersistenceComponent

File:

```text
scripts/Game/TFR/Components/TFR_PlayerInventoryPersistenceComponent.c
```

Type:

```text
GameMode component
Player inventory persistence
```

Placement:

```text
GameMode
```

Main purpose:

Saves and restores player inventory items.

Main responsibilities:

* scan player inventories;
* save player inventory file;
* restore inventory storage slot items;
* avoid duplicate prefab counts;
* clean inventory storage before restore when enabled;
* block autosave while restore is pending;
* avoid saving when no valid controlled players exist.

Main dependencies:

```text
PlayerManager
SCR_InventoryStorageManagerComponent
InventoryItemComponent
SCR_EditableEntityComponent
FileIO
SCR_JsonSaveContext
SCR_JsonLoadContext
```

Used by:

```text
Persistence setup
Future radial player inventory actions
Admin diagnostics
```

Status:

```text
Active
Optional
```

Risk level:

```text
High
```

Reason:

Inventory scans and restore operations can affect performance and data safety.

Review priorities:

* debug should be disabled by default;
* safe autoscan interval;
* safe restore retry limits;
* protect good saves from empty player state;
* max restore item limit;
* avoid restoring unsafe equipment slots unless tested.

---

## TFR_HaloJumpComponent

File:

```text
scripts/Game/TFR/Components/TFR_HaloJumpComponent.c
```

Type:

```text
GameMode component
HALO manager
```

Placement:

```text
GameMode
```

Main purpose:

Server-side HALO jump execution.

Main responsibilities:

* jump player;
* jump all players;
* jump player at selected position;
* validate parachute requirements;
* calculate jump position;
* teleport player;
* auto deploy fallback;
* support map-selected drop zone.

Main dependencies:

```text
PlayerManager
SCR_PlayerController
SCR_ChimeraCharacter
ParachuteComponent
ParachuteItemComponent
TFR_HaloSpawnPointComponent optional
```

Used by:

```text
TFR_HaloMapSelectionComponent
TFR_HaloJumpUserAction
TFR_OperationsRadialEntry
```

Status:

```text
Active
Optional
```

Risk level:

```text
Medium
```

Review priorities:

* keep server-side validation;
* keep teleport retry logic limited;
* document parachute component requirements;
* document dedicated server fallback behaviour;
* no debug spam.

---

## TFR_EnemyCommsManagerComponent

File:

```text
scripts/Game/TFR/Components/TFR_EnemyCommsManagerComponent.c
```

Type:

```text
GameMode component
Enemy communications manager
```

Placement:

```text
GameMode
```

Main purpose:

Enemy communication, alert and support system.

Main responsibilities:

* scan for BLUFOR contacts;
* detect enemies;
* alert nearby AI;
* force mood/aggression;
* request reinforcements;
* request CAS/artillery through wrapper impact systems.

Main dependencies:

```text
TFR_MoodComponent optional
TFR_ArtilleryImpactComponent optional
Faction setup
World query systems
```

Used by:

```text
Enemy reaction system
Mood/aggression flow
Support fire wrapper flow
```

Status:

```text
Active
Optional
```

Risk level:

```text
High
```

Reason:

This system can use scans and affect dedicated server performance.

Review priorities:

* scan interval safety;
* scan radius limits;
* maximum responders;
* reinforcement cooldown;
* CAS/artillery cooldown;
* no scans when disabled;
* debug disabled by default.

---

## TFR_SpawnDirectorComponent

File:

```text
scripts/Game/TFR/Spawn/TFR_SpawnDirectorComponent.c
```

Type:

```text
GameMode or manager entity component
Spawn director
```

Placement:

```text
GameMode or dedicated world manager entity
```

Main purpose:

Area-based spawn director.

Main responsibilities:

* spawn groups;
* spawn vehicles;
* spawn crew;
* assign waypoints;
* create patrols;
* manage persistent vehicle patrols;
* respawn patrols;
* refresh patrol routes.

Main dependencies:

```text
TFR_SpawnAreaComponent
Configured group prefabs
Configured vehicle prefabs
Configured crew prefabs
Configured waypoint prefabs
AI group / waypoint components
```

Used by:

```text
Area-based spawn systems
Persistent patrol systems
```

Status:

```text
Active
Optional
```

Risk level:

```text
High
```

Reason:

Persistent patrols, vehicle spawns and route refreshes can affect performance.

Review priorities:

* persistent patrol defaults;
* patrol check interval;
* route refresh interval;
* max spawn counts;
* no unnecessary scans;
* good warnings for missing prefabs.

---

## TFR_OperationsRadialGameModeComponent

File:

```text
scripts/Game/TFR/Radial/TFR_OperationsRadialGameModeComponent.c
```

Type:

```text
GameMode component
Radial menu holder
```

Placement:

```text
GameMode
```

Main purpose:

Registers the dedicated TFR radial menu on clients.

Main dependencies:

```text
TFR_OperationsRadialMenu
TFR_OperationsRadialComponent
```

Used by:

```text
TFR radial menu setup
```

Status:

```text
Active
Optional
```

Risk level:

```text
Medium
```

Review priorities:

* avoid boot log spam;
* confirm dedicated server guard;
* confirm UI only runs on clients;
* document required menu configuration.

---

# Player / character components

These components live on player characters or controlled entities.

---

## TFR_HaloMapSelectionComponent

File:

```text
scripts/Game/TFR/Components/TFR_HaloMapSelectionComponent.c
```

Type:

```text
Player character component
HALO map selection
```

Placement:

```text
Player character
```

Main purpose:

Client map selection flow for HALO.

Main responsibilities:

* open map;
* bind map click selection;
* convert selected point;
* request HALO from server;
* wait for server response;
* close map;
* apply client owner assist after approved server jump.

Main dependencies:

```text
TFR_HaloJumpComponent
SCR_MapEntity
PlayerController
RPC system
```

Used by:

```text
TFR_HaloJumpUserAction
TFR_OperationsRadialEntry
```

Status:

```text
Active
Optional
```

Risk level:

```text
Medium
```

Review priorities:

* clean map callbacks;
* keep retry counts limited;
* no UI on dedicated server;
* clear local hints;
* timeout server response safely.

---

## TFR_OperationsRadialComponent

File:

```text
scripts/Game/TFR/Radial/TFR_OperationsRadialComponent.c
```

Type:

```text
Player character component
Radial menu controller/filler
```

Placement:

```text
Player character
```

Main purpose:

Controls and fills the TFR Operations radial menu.

Main responsibilities:

* wait for dedicated radial menu;
* take controller control;
* rebuild entries before open;
* create HALO entry;
* create mission admin entries;
* create persistence entries;
* clean radial callbacks.

Main dependencies:

```text
TFR_OperationsRadialGameModeComponent
TFR_OperationsRadialController
TFR_OperationsRadialEntry
TFR_OperationsRadialNetworkComponent
```

Used by:

```text
TFR radial setup
```

Status:

```text
Active
Optional
```

Risk level:

```text
Medium
```

Review priorities:

* hide unimplemented entries;
* avoid boot log spam;
* verify client-only behaviour;
* clean callbacks in OnDelete;
* clear setup warning if controller is missing.

---

# PlayerController components

---

## TFR_OperationsRadialNetworkComponent

File:

```text
scripts/Game/TFR/Radial/TFR_OperationsRadialNetworkComponent.c
```

Type:

```text
Network bridge component
Radial server action bridge
```

Recommended placement:

```text
PlayerController
```

Tolerated placement:

```text
Player character
```

Main purpose:

Network bridge for radial admin/server actions.

Main responsibilities:

* receive client radial action request;
* validate on server;
* check authorized faction;
* call MissionManager admin APIs;
* call PersistenceManager APIs;
* send owner-only hint response.

Main dependencies:

```text
TFR_MissionManagerComponent
TFR_PersistenceManagerComponent
FactionAffiliationComponent
TFR_OperationsRadialEntry
```

Used by:

```text
TFR_OperationsRadialEntry
```

Status:

```text
Active
Optional
```

Risk level:

```text
Medium
```

Known issue / review point:

```text
TFR_OperationsRadialEntry declares PLAYER_INVENTORY_* entries,
but the network component currently appears focused on mission and operation persistence actions.
Those entries should be implemented properly or hidden until ready.
```

Review priorities:

* implement or hide player inventory radial entries;
* reduce normal log spam;
* clear unauthorized messages;
* clear missing manager messages;
* keep server validation.

---

# NPC / civilian / target components

---

## TFR_CivilDataComponent

File:

```text
scripts/Game/TFR/Components/TFR_CivilDataComponent.c
```

Type:

```text
NPC / target data component
```

Placement:

```text
NPC / civilian / suspect / mission target
```

Main purpose:

Stores civil or target state.

Main responsibilities:

* enabled state;
* arrest order state;
* surrender permission;
* identified state.

Main dependencies:

```text
TFR_Action_Identificar
TFR_Action_Arrestar
```

Used by:

```text
Identification flow
Arrest flow
Mission target flow
```

Status:

```text
Active
Optional
```

Risk level:

```text
Low
```

Review priorities:

* consider boolean arrest order in future;
* preserve compatibility with current text values if needed;
* clear Workbench labels.

---

## TFR_InteractionComponent

File:

```text
scripts/Game/TFR/Components/TFR_InteractionComponent.c
```

Type:

```text
NPC interaction component
Informant component
```

Placement:

```text
NPC / informant / interactable civilian
```

Main purpose:

Handles interrogation and intel.

Main responsibilities:

* allow/deny interaction;
* show interrogation response;
* send intel to MissionManager;
* handle single use;
* trigger betrayal;
* request hunt waves through MissionManager.

Main dependencies:

```text
TFR_Action_Interrogar
TFR_MissionManagerComponent
RPC system
```

Used by:

```text
Intel chain
Informant flow
Betrayal flow
```

Status:

```text
Active
Optional
```

Risk level:

```text
Medium
```

Review priorities:

* validate betrayal chance;
* validate hunt wave settings;
* clear missing MissionManager warning;
* keep debug off by default;
* avoid client/server confusion.

---

## TFR_MoodComponent

File:

```text
scripts/Game/TFR/Components/TFR_MoodComponent.c
```

Type:

```text
NPC mood/aggression component
```

Placement:

```text
NPC / civilian
```

Main purpose:

Controls civilian mood and aggression.

States:

```text
CALM
NERVOUS
SCARED
AGGRESSIVE
```

Main responsibilities:

* passive mood changes;
* auto calm;
* hostile lock;
* faction changes;
* scavenger weapon pickup;
* forced aggression.

Main dependencies:

```text
FactionAffiliationComponent
InventoryStorageManagerComponent optional
WeaponComponent optional
TFR_EnemyCommsManagerComponent optional
```

Used by:

```text
Enemy communications
Civilian behaviour
Dynamic hostility systems
```

Status:

```text
Active
Optional
```

Risk level:

```text
High
```

Reason:

This component can run periodic ticks and nearby scans.

Review priorities:

* review mood tick interval;
* review passive stimulus interval;
* review scavenger tick interval;
* review scavenger radius;
* avoid too many NPCs using it;
* keep Run Server Only enabled by default;
* debug disabled by default.

---

## TFR_SurrenderControlComponent

File:

```text
scripts/Game/TFR/Components/TFR_SurrenderControlComponent.c
```

Type:

```text
NPC surrender helper component
```

Placement:

```text
NPC / suspect / mission target
```

Main purpose:

Stores whether surrender is enabled.

Main dependencies:

```text
TFR_Action_Arrestar
ACE / character surrender flow
```

Used by:

```text
Arrest flow
Surrender flow
```

Status:

```text
Active
Optional
```

Risk level:

```text
Low
```

Review priorities:

* keep simple;
* document relation with arrest flow;
* avoid unnecessary expansion.

---

# Physical admin object components

---

## TFR_MissionAdminConsoleComponent

File:

```text
scripts/Game/TFR/Components/TFR_MissionAdminConsoleComponent.c
```

Type:

```text
Physical admin object component
```

Placement:

```text
Radio / laptop / terminal / admin console / box / physical object
```

Main purpose:

Controls access to mission admin actions.

Main responsibilities:

* enabled state;
* authorized faction;
* hide actions if unauthorized;
* check user access;
* call MissionManager admin APIs.

Main dependencies:

```text
FactionAffiliationComponent
TFR_MissionManagerComponent
TFR_MissionAdminActions
```

Used by:

```text
Physical admin console/radio actions
```

Status:

```text
Active
Optional
```

Risk level:

```text
Low / Medium
```

Review priorities:

* keep denied logs debug-only;
* clear warning if MissionManager missing;
* consistent access checks;
* avoid normal log spam.

---

# World marker components

---

## TFR_SpawnAreaComponent

File:

```text
scripts/Game/TFR/Spawn/TFR_SpawnAreaComponent.c
```

Type:

```text
World marker component
Spawn area marker
```

Placement:

```text
World marker/entity
```

Main purpose:

Marks map-independent spawn areas.

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

Main responsibilities:

* register active area;
* expose area type;
* expose radius;
* expose optional area name;
* provide static area collection.

Main dependencies:

```text
TFR_MissionManagerComponent
TFR_SpawnDirectorComponent
```

Used by:

```text
Mission placement
SpawnDirector
Convoy route selection
Area-based systems
```

Status:

```text
Active
Optional but important for map-independent setup
```

Risk level:

```text
Medium
```

Review priorities:

* validate radius;
* keep no ticks;
* debug disabled by default;
* clear setup docs for map makers.

---

## TFR_HaloSpawnPointComponent

File:

```text
scripts/Game/TFR/Spawn/TFR_HaloSpawnPointComponent.c
```

Type:

```text
World marker component
HALO drop zone marker
```

Placement:

```text
World marker/entity
```

Main purpose:

Marks HALO drop zone center.

Main responsibilities:

* register active HALO spawn point;
* expose priority;
* expose position;
* select best active spawn point.

Main dependencies:

```text
TFR_HaloJumpComponent
```

Used by:

```text
HALO jump position source
```

Status:

```text
Active
Optional
```

Risk level:

```text
Low
```

Review priorities:

* keep no loops;
* keep priority behaviour clear;
* debug disabled by default.

---

# Runtime spawned prefab components

---

## TFR_ArtilleryImpactComponent

File:

```text
scripts/Game/TFR/Components/TFR_ArtilleryImpactComponent.c
```

Type:

```text
Runtime spawned prefab component
Impact wrapper
```

Placement:

```text
Runtime spawned impact prefab
```

Main purpose:

Safe wrapper for artillery/CAS impact.

Main responsibilities:

* optional delayed impact execution;
* optional visual impact prefab spawn;
* self-delete after lifetime;
* avoid direct loading of unsafe ammo/effect modules by EnemyComms.

Main dependencies:

```text
TFR_EnemyCommsManagerComponent optional
Visual impact prefab optional
```

Used by:

```text
Enemy support / artillery / CAS wrapper flow
```

Status:

```text
Active
Optional
```

Risk level:

```text
Medium
```

Review priorities:

* debug should be disabled by default;
* confirm server/client spawn ownership;
* keep lifetime limited;
* clear error if visual prefab cannot load.

---

# Radial system scripts

Radial scripts live in:

```text
scripts/Game/TFR/Radial/
```

---

## TFR_OperationsRadialMenu

Type:

```text
Dedicated radial menu
```

Main purpose:

Dedicated TFR radial menu instance.

Status:

```text
Active
Optional
```

Risk level:

```text
Low / Medium
```

Review priorities:

* keep separate from vanilla global radial;
* ensure display setup is reliable.

---

## TFR_OperationsRadialController

Type:

```text
Radial controller
```

Main purpose:

Handles radial input and open/close behaviour.

Main responsibilities:

* register input listener;
* filter analog input;
* avoid double open;
* toggle open/close;
* clean input listeners.

Status:

```text
Active
Optional
```

Risk level:

```text
Medium
```

Review priorities:

* reduce normal log spam;
* ensure unregister always works;
* avoid duplicate listeners.

---

## TFR_OperationsRadialEntry

Type:

```text
Radial menu entry
```

Main purpose:

Defines radial actions.

Current action groups:

```text
HALO
Mission admin
Persistence admin
Player inventory entries declared
```

Status:

```text
Active
Needs review
```

Risk level:

```text
Medium
```

Review priorities:

* implement or hide player inventory entries;
* reduce normal log spam;
* ensure HALO retry logic is limited;
* keep server-sensitive actions routed through network component.

---

# Spawn system scripts

Spawn scripts live in:

```text
scripts/Game/TFR/Spawn/
```

---

## TFR_SpawnDirectorComponent

Already listed as a GameMode/manager component.

Important extra note:

This script is powerful and should be treated as performance-sensitive.

It should not be enabled with large persistent patrol rules without testing.

---

## TFR_SpawnAreaComponent

Already listed as a world marker component.

Important extra note:

This is the key component for making the addon map-independent.

---

## TFR_HaloSpawnPointComponent

Already listed as a HALO world marker component.

---

# Persistence system scripts

Persistence is currently split between:

```text
scripts/Game/TFR/Components/
scripts/Game/TFR/Actions/
scripts/Game/TFR/Radial/
```

Main persistence scripts:

```text
TFR_PersistenceManagerComponent.c
TFR_PlayerInventoryPersistenceComponent.c
TFR_MissionSettingAction.c
TFR_OperationsRadialNetworkComponent.c
```

Persistence should remain conservative and safe.

---

# Performance-sensitive components

These components must be reviewed carefully:

```text
TFR_MissionManagerComponent
TFR_PersistenceManagerComponent
TFR_PlayerInventoryPersistenceComponent
TFR_EnemyCommsManagerComponent
TFR_MoodComponent
TFR_SpawnDirectorComponent
```

Reasons:

* mission loops;
* autosave;
* inventory scans;
* world queries;
* mood ticks;
* scavenger scans;
* persistent patrol checks;
* route refreshes;
* support request systems.

General rules:

* debug disabled by default;
* no unnecessary scans;
* no duplicate `CallLater` loops;
* server-only when possible;
* clear intervals;
* clear limits;
* optional systems disabled when unused.

---

# Known review points

## 1. Player inventory radial entries

`TFR_OperationsRadialEntry.c` declares player inventory entries.

These should be reviewed.

Decision needed:

```text
Implement through TFR_OperationsRadialNetworkComponent
or hide/remove until ready.
```

---

## 2. Debug defaults

Some systems may still have debug or boot logs too visible.

Review needed:

```text
TFR_ArtilleryImpactComponent
TFR_PlayerInventoryPersistenceComponent
TFR_OperationsRadialNetworkComponent
TFR_OperationsRadialComponent
TFR_OperationsRadialGameModeComponent
TFR_OperationsRadialController
```

Goal:

```text
No debug spam during normal gameplay.
```

---

## 3. MissionManager complexity

`TFR_MissionManagerComponent` is powerful but large.

Review needed:

* setup warnings;
* required prefab validation;
* safer presets;
* fewer critical fields exposed at once;
* clearer admin diagnostics.

Goal:

```text
Mission makers should not need to understand the full script to configure operations.
```

---

## 4. Persistence safety

Persistence must avoid data loss.

Review needed:

* operation autosave;
* player inventory autoscan;
* restore timing;
* empty player state protection;
* reset behaviour;
* save file separation.

Goal:

```text
Stable persistence that does not overwrite good data with bad state.
```

---

## 5. EnemyComms / Mood / SpawnDirector performance

These systems are powerful but performance-sensitive.

Review needed:

* scan intervals;
* radius limits;
* max entities;
* tick intervals;
* persistent patrol limits;
* debug logs;
* server-only guards.

Goal:

```text
No major FPS drops on dedicated servers.
```

---

# Component documentation requirements

Each important component should eventually document:

* where it must be placed;
* whether it is required or optional;
* what system it belongs to;
* which fields must be configured;
* what safe defaults it uses;
* what warnings it can generate;
* whether it runs server-side, client-side or both;
* whether it has performance risks.

---

# Final goal

The component setup should be clear enough that a mission maker can configure the addon without reading the internal code.

The addon should remain:

* modular;
* map-independent;
* difficult to break;
* safe for dedicated servers;
* clear in Workbench;
* documented enough for future users and contributors.

The most important rule is:

```text
If setup is confusing, the solution is not to blame the mission maker.
The solution is to simplify the component, improve defaults, add validation warnings and document it clearly.
```
