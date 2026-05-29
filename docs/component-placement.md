# TFR Framework Component Placement

This document defines where each current TFR Framework component should be placed.

TFR Framework and TFR Operations are the same addon.

* **TFR Operations** is the internal Workbench project name.
* **TFR Framework** is the public Workshop / repository name.

The goal of this document is to make setup clear for mission makers and avoid guessing where each component belongs.

---

## Main placement categories

Current TFR components can belong to one of these categories:

```text
GameMode
Player / character
PlayerController
NPC / civilian / mission target
Physical admin object
World marker
Runtime spawned prefab
ActionManager / user action
```

A component should not be placed randomly.

If a component is placed on the wrong entity, the system may fail silently, create warnings, or not work on dedicated server.

---

## Quick placement table

| Script                                      | Placement                        |                      Required? | Main purpose                                     |
| ------------------------------------------- | -------------------------------- | -----------------------------: | ------------------------------------------------ |
| `TFR_MissionManagerComponent.c`             | GameMode                         | Optional / operations required | Main dynamic mission and operation manager       |
| `TFR_JournalTaskBridgeComponent.c`          | GameMode                         |                       Optional | Updates Scenario Framework task/journal          |
| `TFR_PersistenceManagerComponent.c`         | GameMode                         |                       Optional | Saves and loads operation state                  |
| `TFR_PlayerInventoryPersistenceComponent.c` | GameMode                         |                       Optional | Saves and restores player inventory              |
| `TFR_HaloJumpComponent.c`                   | GameMode                         |                       Optional | Server-side HALO execution                       |
| `TFR_EnemyCommsManagerComponent.c`          | GameMode                         |                       Optional | Enemy communication, alerts and support requests |
| `TFR_SpawnDirectorComponent.c`              | GameMode or world manager entity |                       Optional | Area-based spawn director                        |
| `TFR_OperationsRadialGameModeComponent.c`   | GameMode                         |                       Optional | Registers dedicated TFR radial menu on clients   |
| `TFR_OperationsRadialComponent.c`           | Player character                 |                       Optional | Controls and fills the TFR radial menu           |
| `TFR_OperationsRadialNetworkComponent.c`    | PlayerController recommended     |                       Optional | Network bridge for radial actions                |
| `TFR_HaloMapSelectionComponent.c`           | Player character                 |                       Optional | Client map selection for HALO                    |
| `TFR_CivilDataComponent.c`                  | NPC / civilian / target          |                       Optional | Civil state, identification, arrest order        |
| `TFR_InteractionComponent.c`                | NPC / informant                  |                       Optional | Interrogation, intel, betrayal                   |
| `TFR_MoodComponent.c`                       | NPC / civilian                   |                       Optional | Mood, aggression, faction changes                |
| `TFR_SurrenderControlComponent.c`           | NPC / suspect / target           |                       Optional | Allows surrender flow                            |
| `TFR_MissionAdminConsoleComponent.c`        | Physical admin object            |                       Optional | Admin console/radio access control               |
| `TFR_SpawnAreaComponent.c`                  | World marker/entity              |                       Optional | Marks usable spawn areas                         |
| `TFR_HaloSpawnPointComponent.c`             | World marker/entity              |                       Optional | Marks HALO drop zone center                      |
| `TFR_ArtilleryImpactComponent.c`            | Runtime impact prefab            |                       Optional | Safe artillery/CAS impact wrapper                |

---

## GameMode components

GameMode components are global systems.

They usually control mission flow, persistence, HALO, radial registration, enemy communications or spawn management.

These components are the most important for setup.

A mission maker should not have to guess which of these belong on the GameMode.

---

### TFR_MissionManagerComponent

File:

```text
scripts/Game/TFR/Missions/TFR_MissionManagerComponent.c
```

Placement:

```text
GameMode
```

Purpose:

Main dynamic operation and mission manager.

It currently handles:

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
* restore hooks for persistence;
* journal publishing through the journal bridge.

Required:

```text
Required only if dynamic TFR operations/missions are used.
```

Depends on:

```text
TFR_SpawnAreaComponent
TFR_JournalTaskBridgeComponent optional
TFR_PersistenceManagerComponent restore flow optional
TFR_InteractionComponent for informants
TFR_CivilDataComponent for arrest/targets
```

Setup notes:

* Put it on the GameMode.
* Configure mission prefabs.
* Configure spawn areas in the map.
* Enable Auto Start Intel Chain only when wanted.
* Use safe spawn distances and enemy counts.
* Avoid enabling too many heavy systems at once.

Risk level:

```text
High
```

Reason:

This is one of the largest systems in the addon. It has many fields and many dependencies.

Cleanup priority:

* safer defaults;
* better setup warnings;
* clearer required/optional fields;
* simpler presets;
* fewer critical fields exposed at once.

---

### TFR_JournalTaskBridgeComponent

File:

```text
scripts/Game/TFR/Missions/TFR_JournalTaskBridgeComponent.c
```

Placement:

```text
GameMode
```

Purpose:

Bridge between TFR missions and Scenario Framework task/journal systems.

It updates the main task for:

* intel spawned;
* intel received;
* mission created;
* mission updated;
* mission completed;
* mission failed;
* admin reset.

Required:

```text
Optional
```

Required only if TFR missions should update the Scenario Framework task/journal.

Depends on:

```text
SCR_ScenarioFrameworkLayerTask
LayerTask1 or configured layer task entity name
TFR_MissionManagerComponent
```

Setup notes:

* Put it on the GameMode.
* Make sure the configured LayerTask entity exists.
* Default expected entity name is usually `LayerTask1`.
* If the task entity is missing, the component should warn clearly.

Risk level:

```text
Medium
```

Cleanup priority:

* better warning if LayerTask is missing;
* clear setup docs;
* avoid journal phase conflicts.

---

### TFR_PersistenceManagerComponent

File:

```text
scripts/Game/TFR/Components/TFR_PersistenceManagerComponent.c
```

Placement:

```text
GameMode
```

Purpose:

Main operation persistence manager.

It saves and loads:

* operation active state;
* intel chain state;
* mission active state;
* mission type;
* mission area;
* mission position;
* active mission ID;
* convoy route positions.

Required:

```text
Optional
```

Required only if operation persistence is enabled.

Depends on:

```text
TFR_MissionManagerComponent
```

Setup notes:

* Put it on the GameMode.
* Keep Run Server Only enabled.
* Use safe autosave interval.
* Avoid autosave intervals that are too short.
* Keep save files separated.
* Use admin actions/radial for diagnostics and resets.

Risk level:

```text
High
```

Reason:

Persistence can cause data loss if it overwrites good data with bad or empty state.

Cleanup priority:

* safe autosave defaults;
* better restore diagnostics;
* no save spam;
* clear reset behaviour;
* clear documentation.

---

### TFR_PlayerInventoryPersistenceComponent

File:

```text
scripts/Game/TFR/Components/TFR_PlayerInventoryPersistenceComponent.c
```

Placement:

```text
GameMode
```

Purpose:

Player inventory persistence manager.

It can:

* scan connected player inventories;
* save player inventory JSON;
* restore inventory storage slot items;
* avoid duplicate prefab counts;
* block save while initial restore is pending;
* avoid overwriting valid saves when no player controlled entity exists.

Required:

```text
Optional
```

Required only if player inventory persistence is enabled.

Depends on:

```text
PlayerManager
SCR_InventoryStorageManagerComponent on player character
InventoryItemComponent on inventory items
```

Setup notes:

* Put it on the GameMode.
* Keep Run Server Only enabled.
* Use safe autoscan interval.
* Keep restore retry limits controlled.
* Do not restore unsafe equipment slots unless specifically tested.
* Avoid saving when no valid players exist.

Risk level:

```text
High
```

Cleanup priority:

* debug should be disabled by default;
* safe scan interval;
* clear admin diagnostics;
* clear max restore item limits;
* clear warning when no valid players exist.

---

### TFR_HaloJumpComponent

File:

```text
scripts/Game/TFR/Components/TFR_HaloJumpComponent.c
```

Placement:

```text
GameMode
```

Purpose:

Server-side HALO execution manager.

It can:

* jump one player;
* jump all players;
* jump player to selected map position;
* validate parachute requirements;
* calculate jump position;
* apply teleport;
* request auto deploy fallback.

Required:

```text
Optional
```

Required only if HALO insertion is used.

Depends on:

```text
ParachuteComponent
ParachuteItemComponent
TFR_HaloSpawnPointComponent optional
TFR_HaloMapSelectionComponent for map-selected jumps
```

Setup notes:

* Put it on the GameMode.
* Configure jump altitude.
* Configure parachute requirements.
* Configure drop zone source.
* Place optional HALO spawn points if fixed DZs are wanted.

Risk level:

```text
Medium
```

Cleanup priority:

* keep server validation;
* keep retry logic limited;
* document parachute requirements;
* document dedicated server behaviour.

---

### TFR_EnemyCommsManagerComponent

File:

```text
scripts/Game/TFR/Components/TFR_EnemyCommsManagerComponent.c
```

Placement:

```text
GameMode
```

Purpose:

Enemy communication and reaction manager.

It can:

* detect contacts;
* alert nearby enemies;
* trigger mood/aggression;
* request reinforcement;
* request artillery/CAS through safe wrappers.

Required:

```text
Optional
```

Required only if enemy communications are used.

Depends on:

```text
TFR_MoodComponent optional
TFR_ArtilleryImpactComponent optional
Enemy faction setup
```

Setup notes:

* Put it on the GameMode.
* Keep scan intervals safe.
* Keep radius limits controlled.
* Keep CAS/artillery cooldowns controlled.
* Do not enable if not needed.

Risk level:

```text
High
```

Reason:

This system can affect performance because it scans and reacts to entities.

Cleanup priority:

* verify scan intervals;
* verify maximum responders;
* verify cooldowns;
* debug disabled by default;
* no unnecessary scans when disabled.

---

### TFR_SpawnDirectorComponent

File:

```text
scripts/Game/TFR/Spawn/TFR_SpawnDirectorComponent.c
```

Placement:

```text
GameMode or dedicated world manager entity
```

Purpose:

Area-based spawn director.

It can:

* spawn groups;
* spawn vehicles;
* spawn crew;
* assign waypoints;
* create patrols;
* manage persistent vehicle patrols;
* respawn patrols.

Required:

```text
Optional
```

Required only if spawn director rules are used.

Depends on:

```text
TFR_SpawnAreaComponent
Configured prefabs
Waypoint prefabs
AI group/waypoint components
```

Setup notes:

* Can live on GameMode or a dedicated manager entity.
* Requires placed `TFR_SpawnAreaComponent` areas.
* Auto Spawn On Start should be used carefully.
* Persistent patrols should be limited.
* Patrol health checks should use safe intervals.

Risk level:

```text
High
```

Cleanup priority:

* safe defaults for persistent patrols;
* no unnecessary area scans;
* good warnings for missing prefabs;
* clear limits for vehicle/crew counts.

---

### TFR_OperationsRadialGameModeComponent

File:

```text
scripts/Game/TFR/Radial/TFR_OperationsRadialGameModeComponent.c
```

Placement:

```text
GameMode
```

Purpose:

Registers the dedicated TFR radial menu on clients.

Required:

```text
Optional
```

Required only if the TFR radial menu is used.

Depends on:

```text
TFR_OperationsRadialMenu
TFR_OperationsRadialComponent on player character
```

Setup notes:

* Put it on the GameMode.
* Configure the radial menu instance.
* It should not register UI on dedicated server.
* It uses frame update for the menu on clients.

Risk level:

```text
Medium
```

Cleanup priority:

* avoid boot log spam;
* confirm dedicated server guard;
* document radial setup.

---

## Player / character components

These components belong on player characters or controlled entities.

---

### TFR_HaloMapSelectionComponent

File:

```text
scripts/Game/TFR/Components/TFR_HaloMapSelectionComponent.c
```

Placement:

```text
Player character
```

Purpose:

Client-side HALO map selection flow.

It allows a player to:

* open map selection;
* click a drop zone;
* send request to server;
* receive approved jump position;
* apply client owner assist.

Required:

```text
Optional
```

Required only for HALO map selection.

Depends on:

```text
TFR_HaloJumpComponent on GameMode
PlayerController
SCR_MapEntity
```

Setup notes:

* Put it on the player character prefab.
* It should only operate for the local controlled owner.
* It should not run UI logic on dedicated server.
* It must request server validation before executing HALO.

Risk level:

```text
Medium
```

Cleanup priority:

* keep retries limited;
* clean map callbacks;
* clear hints;
* no UI on dedicated server.

---

### TFR_OperationsRadialComponent

File:

```text
scripts/Game/TFR/Radial/TFR_OperationsRadialComponent.c
```

Placement:

```text
Player character
```

Purpose:

Controls and fills the TFR radial menu for the player.

It creates entries for:

* HALO;
* mission admin;
* persistence admin.

Required:

```text
Optional
```

Required only if TFR radial menu is used.

Depends on:

```text
TFR_OperationsRadialGameModeComponent on GameMode
TFR_OperationsRadialController
TFR_OperationsRadialEntry
TFR_OperationsRadialNetworkComponent for server actions
```

Setup notes:

* Put it on the player character prefab.
* Configure the radial controller.
* Enable only entries that should be visible.
* Do not show admin entries to normal players unless intended.

Risk level:

```text
Medium
```

Cleanup priority:

* hide unimplemented entries;
* avoid client/server confusion;
* clean callbacks in OnDelete;
* avoid boot log spam.

---

## PlayerController component

---

### TFR_OperationsRadialNetworkComponent

File:

```text
scripts/Game/TFR/Radial/TFR_OperationsRadialNetworkComponent.c
```

Recommended placement:

```text
PlayerController
```

Tolerated placement:

```text
Player character
```

Purpose:

Network bridge for radial actions.

It allows:

* client radial entry request;
* server validation;
* faction permission check;
* execution of mission admin actions;
* execution of persistence actions;
* owner-only hint response.

Required:

```text
Optional
```

Required if radial mission/admin/persistence actions are used.

Depends on:

```text
TFR_MissionManagerComponent
TFR_PersistenceManagerComponent
FactionAffiliationComponent on user entity
```

Setup notes:

* Recommended on PlayerController.
* Can be tolerated on controlled character if needed.
* Sensitive actions must go through server validation.
* Authorized faction should be configured.
* Admin/persistence permissions should be explicit.

Risk level:

```text
Medium
```

Cleanup priority:

* implement or hide player inventory radial entries;
* avoid normal log spam;
* clear unauthorized messages;
* clear missing manager messages.

---

## NPC / civilian / mission target components

These components belong on NPCs, civilians, informants, suspects or mission targets.

---

### TFR_CivilDataComponent

File:

```text
scripts/Game/TFR/Components/TFR_CivilDataComponent.c
```

Placement:

```text
NPC / civilian / suspect / mission target
```

Purpose:

Stores civil or target data.

It currently stores:

* enabled state;
* arrest order;
* can surrender;
* identified state.

Required:

```text
Optional
```

Required for identify/arrest flow.

Depends on:

```text
TFR_Action_Identificar
TFR_Action_Arrestar
```

Setup notes:

* Add to civilians or suspects that need identification/arrest logic.
* Identified state is internal.
* Arrest order currently accepts text values like `si`, `sí`, `true`, `1`.

Risk level:

```text
Low
```

Cleanup priority:

* consider safer boolean setup for arrest order;
* keep compatibility with old field names if needed;
* clear Workbench labels.

---

### TFR_InteractionComponent

File:

```text
scripts/Game/TFR/Components/TFR_InteractionComponent.c
```

Placement:

```text
NPC / informant / interactable civilian
```

Purpose:

Handles interrogation logic.

It can:

* show a short response;
* provide intel to MissionManager;
* mark itself consumed;
* trigger betrayal;
* request hunt waves through MissionManager.

Required:

```text
Optional
```

Required for interrogation flow.

Depends on:

```text
TFR_Action_Interrogar
TFR_MissionManagerComponent
```

Setup notes:

* Add to informants or NPCs that can be interrogated.
* Configure intel stage if needed.
* Configure single use.
* Configure betrayal only if used.
* Hunt wave prefab list should be configured only if betrayal/hunt is used.

Risk level:

```text
Medium
```

Cleanup priority:

* keep debug disabled;
* validate betrayal values;
* validate hunt wave prefab list;
* clear warning if MissionManager missing.

---

### TFR_MoodComponent

File:

```text
scripts/Game/TFR/Components/TFR_MoodComponent.c
```

Placement:

```text
NPC / civilian
```

Purpose:

Handles civilian mood and aggression.

Mood states:

```text
CALM
NERVOUS
SCARED
AGGRESSIVE
```

It can:

* change faction;
* keep hostile lock;
* activate scavenger;
* let civilians pick up weapons;
* be forced aggressive by other systems.

Required:

```text
Optional
```

Required only for mood/aggression/scavenger behaviour.

Depends on:

```text
FactionAffiliationComponent
InventoryStorageManagerComponent optional
WeaponComponent for scavenger checks
TFR_EnemyCommsManagerComponent optional
```

Setup notes:

* Use carefully.
* This component has periodic ticks.
* Do not add to every civilian unless needed.
* Run Server Only should usually stay enabled.
* Scavenger should remain limited.

Risk level:

```text
High
```

Cleanup priority:

* review tick intervals;
* review scavenger search radius;
* avoid too many NPCs using it;
* debug disabled by default;
* clear performance limits.

---

### TFR_SurrenderControlComponent

File:

```text
scripts/Game/TFR/Components/TFR_SurrenderControlComponent.c
```

Placement:

```text
NPC / suspect / mission target
```

Purpose:

Small component that allows surrender state to be enabled by TFR.

Required:

```text
Optional
```

Required only for arrest/surrender flow.

Depends on:

```text
TFR_Action_Arrestar
ACE surrender / character controller flow
```

Setup notes:

* Add to suspects or arrest targets that should be able to surrender.
* This component is intentionally simple.

Risk level:

```text
Low
```

Cleanup priority:

* document relation with ACE surrender.
* keep simple.

---

## Physical admin object components

These components belong on radios, terminals, laptops, boxes, command posts or admin consoles.

---

### TFR_MissionAdminConsoleComponent

File:

```text
scripts/Game/TFR/Components/TFR_MissionAdminConsoleComponent.c
```

Placement:

```text
Physical admin object
```

Purpose:

Controls access to mission admin actions.

It checks:

* enabled state;
* authorized faction;
* whether unauthorized users should see actions.

It calls:

```text
TFR_MissionManagerComponent
```

Required:

```text
Optional
```

Required only if physical admin console/radio actions are used.

Depends on:

```text
FactionAffiliationComponent
TFR_MissionManagerComponent
TFR_MissionAdminActions
```

Setup notes:

* Add to a radio, laptop, terminal, box or similar object.
* Add the related user actions to the object ActionManager.
* Configure authorized faction key.
* Leave Hide Actions If Unauthorized enabled if normal users should not see actions.

Risk level:

```text
Low / Medium
```

Cleanup priority:

* consistent permission checks;
* useful denied messages when debug enabled;
* no normal log spam.

---

## World marker components

These components belong on placed world entities used as markers.

---

### TFR_SpawnAreaComponent

File:

```text
scripts/Game/TFR/Spawn/TFR_SpawnAreaComponent.c
```

Placement:

```text
World marker/entity
```

Purpose:

Marks a map-independent spawn area.

The addon can then use those areas for:

* mission placement;
* enemy spawn;
* vehicle spawn;
* patrol spawn;
* area selection.

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

Required:

```text
Optional, but required for area-based mission/spawn systems.
```

Depends on:

```text
TFR_MissionManagerComponent
TFR_SpawnDirectorComponent
```

Setup notes:

* Place simple marker entities around the map.
* Add `TFR_SpawnAreaComponent`.
* Configure area type.
* Configure radius.
* Enable or disable as needed.
* Optional area name can be used for exact matching.

Risk level:

```text
Medium
```

Cleanup priority:

* clear docs for mission makers;
* debug disabled;
* no continuous scans;
* validate radius.

---

### TFR_HaloSpawnPointComponent

File:

```text
scripts/Game/TFR/Spawn/TFR_HaloSpawnPointComponent.c
```

Placement:

```text
World marker/entity
```

Purpose:

Marks a HALO drop zone center.

The HALO manager can use it before falling back to GameMode position or override position.

Required:

```text
Optional
```

Required only if fixed HALO drop zone markers are used.

Depends on:

```text
TFR_HaloJumpComponent
```

Setup notes:

* Place a simple entity where the HALO DZ should be.
* Add `TFR_HaloSpawnPointComponent`.
* Configure priority if multiple HALO spawn points exist.
* Higher priority wins.

Risk level:

```text
Low
```

Cleanup priority:

* keep no loops;
* clear priority behaviour;
* debug disabled.

---

## Runtime spawned prefab components

---

### TFR_ArtilleryImpactComponent

File:

```text
scripts/Game/TFR/Components/TFR_ArtilleryImpactComponent.c
```

Placement:

```text
Runtime spawned prefab
```

Expected prefab:

```text
TFR_ArtilleryImpact.et
```

Purpose:

Safe wrapper for artillery/CAS impact.

It can:

* spawn optional visual impact prefab;
* execute delayed impact logic;
* delete itself after lifetime;
* prevent EnemyComms from loading unsafe ammo/effect modules directly.

Required:

```text
Optional
```

Required only if enemy communications or support systems use artillery/CAS wrapper impacts.

Depends on:

```text
TFR_EnemyCommsManagerComponent optional
Visual impact prefab optional
```

Setup notes:

* Add to the impact wrapper prefab.
* Configure lifetime.
* Configure optional visual impact prefab.
* Debug should normally be disabled.

Risk level:

```text
Medium
```

Cleanup priority:

* debug should be disabled by default;
* server/client spawn ownership should be clear;
* lifetime should remain limited.

---

## Action scripts placement

Action scripts are not components, but they still have placement requirements.

They are usually attached through an `ActionManagerComponent`.

---

### Identify action

File:

```text
scripts/Game/TFR/Actions/TFR_Action_Identificar.c
```

Placement:

```text
ActionManagerComponent on NPC / civilian / target
```

Required target component:

```text
TFR_CivilDataComponent
```

Purpose:

Identifies the entity.

---

### Interrogate action

File:

```text
scripts/Game/TFR/Actions/TFR_Action_Interrogar.c
```

Placement:

```text
ActionManagerComponent on NPC / informant
```

Required target component:

```text
TFR_InteractionComponent
```

Purpose:

Runs interrogation flow.

---

### Arrest action

File:

```text
scripts/Game/TFR/Actions/TFR_Action_Arrestar.c
```

Placement:

```text
ActionManagerComponent on suspect / arrest target
```

Required target components:

```text
TFR_CivilDataComponent
TFR_SurrenderControlComponent
```

Related manager:

```text
TFR_MissionManagerComponent
```

Purpose:

Allows arrest/surrender flow and notifies mission manager.

---

### HALO action

File:

```text
scripts/Game/TFR/Actions/TFR_HaloJumpUserAction.c
```

Placement:

```text
ActionManagerComponent on object/player interaction source as configured
```

Required systems:

```text
TFR_HaloMapSelectionComponent on player character
TFR_HaloJumpComponent on GameMode
```

Purpose:

Starts HALO selection or request flow.

---

### Mission admin actions

File:

```text
scripts/Game/TFR/Actions/TFR_MissionAdminActions.c
```

Placement:

```text
ActionManagerComponent on admin radio/console/object
```

Required object component:

```text
TFR_MissionAdminConsoleComponent
```

Purpose:

Allows admin mission actions through a physical object.

---

### Persistence admin actions

File:

```text
scripts/Game/TFR/Actions/TFR_MissionSettingAction.c
```

Placement:

```text
ActionManagerComponent on admin radio/console/object
```

Required manager:

```text
TFR_PersistenceManagerComponent
```

Purpose:

Allows admin persistence save/load/reset actions.

---

## Recommended minimal setups

### Minimal operations setup

For basic dynamic operations:

```text
GameMode:
- TFR_MissionManagerComponent
- TFR_JournalTaskBridgeComponent optional

World:
- TFR_SpawnAreaComponent markers

NPC prefabs:
- TFR_CivilDataComponent where needed
- TFR_InteractionComponent on informants

Actions:
- TFR_Action_Identificar where needed
- TFR_Action_Interrogar where needed
- TFR_Action_Arrestar where needed
```

---

### HALO setup

For HALO by map:

```text
GameMode:
- TFR_HaloJumpComponent

Player character:
- TFR_HaloMapSelectionComponent

Optional world marker:
- TFR_HaloSpawnPointComponent

Optional action/radial:
- TFR_HaloJumpUserAction
- TFR_OperationsRadialComponent
```

---

### Persistence setup

For operation persistence:

```text
GameMode:
- TFR_PersistenceManagerComponent
- TFR_MissionManagerComponent

Optional:
- TFR_PlayerInventoryPersistenceComponent
```

For admin persistence control:

```text
Physical object:
- ActionManagerComponent
- TFR_MissionSettingAction entries
```

Or through radial:

```text
Player / PlayerController:
- TFR_OperationsRadialNetworkComponent

GameMode:
- TFR_PersistenceManagerComponent
```

---

### Radial setup

For TFR radial:

```text
GameMode:
- TFR_OperationsRadialGameModeComponent

Player character:
- TFR_OperationsRadialComponent

PlayerController recommended:
- TFR_OperationsRadialNetworkComponent
```

Optional systems exposed through radial:

```text
HALO:
- TFR_HaloJumpComponent
- TFR_HaloMapSelectionComponent

Mission admin:
- TFR_MissionManagerComponent

Persistence:
- TFR_PersistenceManagerComponent
```

---

### Enemy communications setup

For enemy communications:

```text
GameMode:
- TFR_EnemyCommsManagerComponent

NPCs/civilians/enemies where needed:
- TFR_MoodComponent optional
```

Optional support impact:

```text
Runtime prefab:
- TFR_ArtilleryImpactComponent
```

Important:

Enemy communications should be configured carefully because scan intervals, radius and support calls can affect performance.

---

### Spawn director setup

For area-based spawns:

```text
GameMode or manager entity:
- TFR_SpawnDirectorComponent

World markers:
- TFR_SpawnAreaComponent
```

Configure:

```text
Spawn rules
Vehicle spawn rules
Prefab pools
Area types
Waypoint prefabs
Persistent patrol settings
```

Important:

Persistent patrols should be limited and tested on dedicated server.

---

## Setup risks by category

### Highest risk

These systems should be reviewed carefully:

```text
TFR_MissionManagerComponent
TFR_PersistenceManagerComponent
TFR_PlayerInventoryPersistenceComponent
TFR_EnemyCommsManagerComponent
TFR_MoodComponent
TFR_SpawnDirectorComponent
```

Reason:

They can affect mission flow, persistence safety or dedicated server performance.

---

### Medium risk

```text
TFR_HaloJumpComponent
TFR_HaloMapSelectionComponent
TFR_OperationsRadialComponent
TFR_OperationsRadialNetworkComponent
TFR_JournalTaskBridgeComponent
TFR_ArtilleryImpactComponent
TFR_SpawnAreaComponent
```

Reason:

They depend on correct placement and setup, but are more limited in scope.

---

### Lower risk

```text
TFR_CivilDataComponent
TFR_SurrenderControlComponent
TFR_HaloSpawnPointComponent
TFR_MissionAdminConsoleComponent
```

Reason:

They are smaller and easier to reason about.

---

## General placement rules

1. GameMode owns global systems.
2. Player character owns local player UI or player-specific components.
3. PlayerController is preferred for network bridges.
4. NPCs own their own interaction, mood and civil state.
5. Physical admin objects own admin console components and actions.
6. World markers own spawn area or HALO spawn point components.
7. Runtime prefabs own temporary behaviour like impact wrappers.
8. Actions should stay on the entities the player interacts with.

---

## Do not do this

Do not put every component on the GameMode.

Do not put UI-only components on dedicated server logic.

Do not put global managers on individual NPCs.

Do not put persistence components on player characters.

Do not put MissionManager on an admin console object.

Do not put SpawnAreaComponent on every random object.

Do not enable EnemyComms or Mood everywhere without performance testing.

Do not expose admin radial entries to normal players unless intended.

---

## Final goal

The final goal is that a mission maker can understand setup without reading the code.

A mission maker should be able to know:

* what goes on GameMode;
* what goes on the player;
* what goes on PlayerController;
* what goes on NPCs;
* what goes on admin objects;
* what goes on world markers;
* what is optional;
* what is required for each system.

This directly supports the fourth roadmap front:

```text
Simplification, cleanup and optimization of the addon.
```

The setup should be clear, difficult to break and safe for dedicated servers.
