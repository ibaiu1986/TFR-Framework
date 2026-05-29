# TFR Framework Performance Guidelines

This document defines the performance rules and current performance review targets for TFR Framework.

TFR Framework and TFR Operations are the same addon.

* **TFR Operations** is the internal Workbench project name.
* **TFR Framework** is the public Workshop / repository name.

Performance is a core requirement of the addon, especially for dedicated servers and long-running operations.

The addon must avoid heavy systems that cause FPS drops, server lag, script spam or unstable gameplay.

---

## Main goal

TFR Framework must remain safe for dedicated servers.

Systems should be:

* modular;
* limited;
* optional;
* server-aware;
* disabled when unused;
* clear in Workbench;
* predictable under load.

The addon must avoid:

* unnecessary world scans;
* duplicated `CallLater` loops;
* unlimited timers;
* debug spam;
* heavy logic every frame;
* aggressive autosave intervals;
* uncontrolled AI systems;
* uncontrolled communication systems;
* persistent patrols without limits.

---

## General performance rules

All systems should follow these rules:

* Avoid unnecessary scans.
* Avoid repeated world queries unless clearly limited.
* Avoid duplicated `CallLater` loops.
* Avoid unlimited timers.
* Avoid heavy logic in `EOnFrame`.
* Avoid debug logs during normal gameplay.
* Avoid saving too often.
* Avoid spawning too many entities at once.
* Avoid permanent systems running when disabled.
* Keep optional systems disabled when unused.
* Use server-only execution for server logic.
* Use client-only guards for UI logic.
* Clean `CallLater` calls in `OnDelete`.
* Clean callbacks and input listeners in `OnDelete`.
* Use clear maximum counts.
* Use clear cooldowns.
* Use clear scan intervals.
* Prefer event-driven logic where possible.

---

## High-priority performance review targets

The following systems are the highest priority for performance review:

```text
TFR_EnemyCommsManagerComponent
TFR_MoodComponent
TFR_SpawnDirectorComponent
TFR_PersistenceManagerComponent
TFR_PlayerInventoryPersistenceComponent
TFR_MissionManagerComponent
TFR_OperationsRadial*
```

These systems are powerful and useful, but they can affect dedicated server performance if configured badly.

---

# TFR_EnemyCommsManagerComponent

## Performance risk

Enemy communications can be expensive because it may scan for contacts, alert enemies, request support and interact with mood/aggression systems.

Potential risk areas:

* contact scan interval;
* scan radius;
* alert radius;
* maximum responders;
* reinforcement calls;
* artillery/CAS requests;
* repeated world queries;
* debug logs;
* interaction with many AI/NPC entities.

## Required performance rules

EnemyComms should:

* run server-side only unless there is a specific reason not to;
* do nothing when disabled;
* keep scan intervals safe;
* keep scan radius limited;
* keep maximum responders limited;
* keep reinforcement cooldowns limited;
* keep CAS/artillery cooldowns limited;
* avoid support spam;
* avoid debug spam;
* clean `CallLater` loops in `OnDelete`.

## Review checklist

```text
[ ] Debug logs disabled by default
[ ] Run Server Only enabled by default
[ ] Scan interval has safe minimum
[ ] Scan radius has safe maximum or recommended limit
[ ] Alert radius has safe maximum or recommended limit
[ ] Maximum responders limited
[ ] Reinforcement cooldown limited
[ ] CAS/artillery cooldown limited
[ ] No scan runs when disabled
[ ] CallLater removed in OnDelete
[ ] Warnings are useful but not spammy
```

## Goal

Enemy communications should create dynamic reactions without becoming a permanent expensive scanner across the whole map.

---

# TFR_MoodComponent

## Performance risk

Mood can be expensive if placed on too many NPCs because it uses periodic mood logic and may use scavenger scans for nearby weapons.

Potential risk areas:

* mood tick interval;
* passive stimulus interval;
* scavenger tick interval;
* ground weapon search radius;
* number of NPCs with this component;
* faction changes;
* inventory insert attempts;
* repeated failed item retries;
* debug logs.

## Required performance rules

Mood should:

* run server-side by default;
* be placed only on NPCs that need mood/aggression behaviour;
* avoid being added to every civilian without testing;
* keep mood tick interval safe;
* keep passive stimulus interval safe;
* keep scavenger tick interval safe;
* keep scavenger radius limited;
* avoid repeated pickup attempts on the same failed item;
* stop scavenger when no longer needed;
* clean `CallLater` loops in `OnDelete`.

## Review checklist

```text
[ ] Debug logs disabled by default
[ ] Run Server Only enabled by default
[ ] Mood tick interval safe
[ ] Passive stimulus interval safe
[ ] Scavenger tick interval safe
[ ] Scavenger radius limited
[ ] Scavenger stops after weapon assigned
[ ] Failed scavenger items have retry delay
[ ] CallLater removed in OnDelete
[ ] Documentation warns not to place on every NPC blindly
```

## Goal

Mood should add civilian behaviour and dynamic aggression without creating many uncontrolled per-NPC loops.

---

# TFR_SpawnDirectorComponent

## Performance risk

SpawnDirector can be expensive if it spawns too many entities, vehicles, crews, waypoints or persistent patrols.

Potential risk areas:

* auto spawn on start;
* area scan radius;
* number of spawn rules;
* number of vehicle spawn rules;
* vehicle count;
* crew count;
* waypoint count;
* persistent patrol health checks;
* patrol respawn;
* route refresh;
* world patrol destination attempts;
* cleanup of persistent patrol entities.

## Required performance rules

SpawnDirector should:

* run server-side by default;
* avoid continuous area scans;
* use the static registry from `TFR_SpawnAreaComponent`;
* only run persistent patrol checks if persistent patrols exist;
* limit vehicle count;
* limit crew count;
* limit patrol waypoint count;
* limit route refresh frequency;
* limit respawn checks;
* clean persistent patrol entities in `OnDelete`;
* avoid debug spam.

## Review checklist

```text
[ ] Debug logs disabled by default
[ ] Run Server Only enabled by default
[ ] No EOnFrame loop
[ ] Area scan only when needed
[ ] Persistent patrol check only starts when needed
[ ] Persistent patrol interval has safe minimum
[ ] Vehicle count limited
[ ] Crew count limited
[ ] Waypoint count limited
[ ] World patrol attempts limited
[ ] Route refresh interval safe
[ ] Patrol respawn delay safe
[ ] Spawned/persistent entities cleaned when needed
[ ] CallLater removed in OnDelete
```

## Goal

SpawnDirector should support map-independent spawning and patrols without causing uncontrolled entity growth or constant server load.

---

# TFR_PersistenceManagerComponent

## Performance risk

Operation persistence can affect performance or data safety if it saves too often or restores invalid state.

Potential risk areas:

* autosave interval;
* load on start delay;
* save file paths;
* repeated save calls;
* reset behaviour;
* missing MissionManager;
* debug/admin logs;
* operation restore timing.

## Required performance rules

PersistenceManager should:

* run server-side by default;
* use safe autosave intervals;
* never autosave too frequently;
* validate save paths;
* avoid save spam;
* use useful admin logs;
* keep debug logs optional;
* handle missing MissionManager safely;
* clear scheduled calls in `OnDelete`.

## Review checklist

```text
[ ] Debug logs disabled by default
[ ] Run Server Only enabled by default
[ ] Autosave interval has safe minimum
[ ] Load delay cannot be negative
[ ] Save directory fallback valid
[ ] Save file names fallback valid
[ ] Missing MissionManager logs clear error
[ ] Autosave removed in OnDelete
[ ] Load scheduled call removed in OnDelete
[ ] Admin diagnostics useful but not constant spam
```

## Goal

Operation persistence should be stable, predictable and safe for long-running dedicated servers.

---

# TFR_PlayerInventoryPersistenceComponent

## Performance risk

Player inventory persistence can be expensive because it scans player inventories and writes JSON files. It can also risk data loss if bad empty states overwrite valid saves.

Potential risk areas:

* autoscan interval;
* initial scan delay;
* restore on start delay;
* restore retry count;
* restore retry delay;
* number of players;
* number of items per player;
* duplicate entity scans;
* prefab extraction;
* cleaning inventory before restore;
* debug/admin logs.

## Required performance rules

PlayerInventoryPersistence should:

* run server-side by default;
* use safe autoscan intervals;
* skip save when no valid players exist;
* block autosave while restore is pending when configured;
* limit restore retry count;
* limit max restore items per player;
* avoid duplicate item saves;
* avoid restoring unsafe slots by default;
* keep loadout/equipment restore disabled unless tested;
* avoid debug spam.

## Review checklist

```text
[ ] Debug logs disabled by default
[ ] Run Server Only enabled by default
[ ] Autoscan interval has safe minimum
[ ] Initial scan delay safe
[ ] Restore retry count limited
[ ] Restore retry delay has safe minimum
[ ] Max restore items per player limited
[ ] Skip save when no valid players enabled
[ ] Restore blocks save until completed when needed
[ ] Loadout slot restore disabled by default
[ ] Equipment storage restore disabled by default
[ ] Duplicate entities skipped
[ ] CallLater removed in OnDelete
[ ] Admin logs useful but not spammy
```

## Goal

Player inventory persistence should preserve player inventory safely without creating heavy scans or overwriting good saves with bad empty data.

---

# TFR_MissionManagerComponent

## Performance risk

MissionManager is one of the largest systems. It controls mission flow, intel jobs, mission checks, spawns, rewards, convoy logic, reinforcements, hunt waves and restore hooks.

Potential risk areas:

* mission check loop;
* spawned enemies;
* escort count;
* reinforcement waves;
* hunt waves;
* convoy vehicle count;
* convoy waypoint count;
* reward lifetime;
* smoke lifetime;
* repeated cleanup/spawn;
* area position attempts;
* fallback placement logic;
* debug logs.

## Required performance rules

MissionManager should:

* run mission logic server-side;
* avoid mission check loops when no mission is active;
* stop mission check loop when not needed;
* limit enemy counts;
* limit escort counts;
* limit reinforcement waves;
* limit hunt waves;
* limit convoy vehicle count;
* limit route waypoint count;
* limit area position attempts;
* clean spawned entities when mission ends;
* avoid debug spam.

## Review checklist

```text
[ ] Debug logs disabled by default
[ ] Mission check interval safe
[ ] Mission check loop only runs when needed
[ ] Mission check loop stops when done
[ ] Clear enemy count limited
[ ] Defend enemy count limited
[ ] Escort count limited
[ ] Reinforcement waves limited
[ ] Hunt waves limited
[ ] Convoy vehicle count limited
[ ] Convoy route intermediate waypoints limited
[ ] Area position attempts limited
[ ] Reward lifetime safe
[ ] Smoke lifetime not longer than reward lifetime
[ ] Spawned mission entities cleaned
[ ] CallLater removed in OnDelete
```

## Goal

MissionManager should remain powerful but predictable, with clear limits and no unnecessary background work.

---

# Radial system

## Performance risk

The radial system is mostly UI and input handling, but it can spam logs or register duplicate input listeners if not cleaned correctly.

Potential risk areas:

* input listener registration;
* repeated registration retries;
* boot logs;
* radial menu update;
* callback cleanup;
* network request logs;
* showing unimplemented entries.

Current related scripts:

```text
TFR_OperationsRadialMenu
TFR_OperationsRadialController
TFR_OperationsRadialGameModeComponent
TFR_OperationsRadialComponent
TFR_OperationsRadialEntry
TFR_OperationsRadialNetworkComponent
```

## Required performance rules

Radial should:

* run UI logic only on clients;
* avoid UI logic on dedicated server;
* clean input listeners;
* clean callbacks;
* avoid boot log spam;
* avoid normal action log spam;
* hide entries that are not implemented;
* route server-sensitive actions through network validation.

## Review checklist

```text
[ ] Dedicated server UI guards exist
[ ] Input listeners removed on cleanup
[ ] CallLater registration retries are limited
[ ] Radial callbacks removed in OnDelete
[ ] Boot logs reduced or debug-gated
[ ] Entry perform logs reduced or debug-gated
[ ] Network action logs reduced or debug-gated
[ ] Unimplemented player inventory entries hidden or implemented
```

## Goal

The radial should be useful and stable without adding log spam or client/server confusion.

---

# TFR_ArtilleryImpactComponent

## Performance risk

Artillery impact wrapper is temporary, but it can cause issues if debug is enabled by default or if visual impact prefabs are heavy.

Potential risk areas:

* debug default;
* visual prefab load;
* spawn ownership;
* lifetime;
* cleanup;
* repeated impacts.

## Required performance rules

ArtilleryImpact should:

* be temporary;
* self-delete after configured lifetime;
* keep debug disabled by default;
* use optional visual prefab only when configured;
* clean scheduled calls in `OnDelete`;
* avoid spawning heavy effects repeatedly without cooldown upstream.

## Review checklist

```text
[ ] Debug disabled by default
[ ] Lifetime has safe fallback
[ ] Impact delay has safe fallback
[ ] Visual prefab optional
[ ] Visual prefab load failure logs only error
[ ] CallLater removed in OnDelete
[ ] Self-delete works
```

## Goal

Artillery/CAS impact wrappers should be safe temporary entities and never become persistent server load.

---

# Debug log rules

Debug logs are useful during development, but they should not spam normal gameplay.

## General rules

* Debug logs should be disabled by default.
* Boot confirmation logs should be removed or gated after testing.
* Admin diagnostics may print when explicitly requested.
* Errors should remain visible when something critical is missing.
* Repeated warnings should be rate-limited where possible.
* Normal successful repeated actions should not print constantly.

## Logs that should be reviewed

```text
TFR_OperationsRadialGameModeComponent boot logs
TFR_OperationsRadialComponent boot logs
TFR_OperationsRadialNetworkComponent boot logs
TFR_OperationsRadialController input registration logs
TFR_OperationsRadialEntry action perform logs
TFR_PlayerInventoryPersistenceComponent admin logs
TFR_ArtilleryImpactComponent debug default
```

## Goal

Server logs should help diagnose problems, not hide them inside spam.

---

# CallLater rules

`CallLater` is useful but dangerous if duplicated or never removed.

## Required rules

Every repeated `CallLater` should have:

* clear interval;
* clear owner;
* clear stop condition;
* cleanup in `OnDelete`;
* no duplicate scheduling;
* safe disabled behaviour.

Every retry `CallLater` should have:

* maximum retry count;
* safe delay;
* clear failure message;
* cleanup in `OnDelete`.

## Systems to review

```text
TFR_EnemyCommsManagerComponent
TFR_MoodComponent
TFR_SpawnDirectorComponent
TFR_PersistenceManagerComponent
TFR_PlayerInventoryPersistenceComponent
TFR_MissionManagerComponent
TFR_HaloJumpComponent
TFR_HaloMapSelectionComponent
TFR_OperationsRadialController
TFR_OperationsRadialComponent
TFR_OperationsRadialEntry
TFR_JournalTaskBridgeComponent
TFR_ArtilleryImpactComponent
```

---

# World scan rules

World scans and AABB queries can be expensive.

## Required rules

World scans should:

* only run when system is enabled;
* only run on server when possible;
* have safe intervals;
* have safe radius;
* have clear maximum results or early exits where possible;
* avoid scanning the whole map repeatedly;
* avoid running for every NPC unless necessary.

## Systems to review

```text
TFR_EnemyCommsManagerComponent
TFR_MoodComponent scavenger
TFR_SpawnDirectorComponent
TFR_MissionManagerComponent placement helpers
```

---

# Persistence performance rules

Persistence should never become a constant heavy process.

## Required rules

* Autosave intervals must be conservative.
* Inventory scan intervals must be conservative.
* Save operations should be explicit and predictable.
* Restore retries must be limited.
* Save should be skipped when no valid players exist.
* Good saves should not be overwritten by empty or invalid state.
* Admin diagnostics should be on-demand, not constant.

## Systems to review

```text
TFR_PersistenceManagerComponent
TFR_PlayerInventoryPersistenceComponent
TFR_MissionSettingAction
TFR_OperationsRadialNetworkComponent
```

---

# Dedicated server rules

Dedicated server must not run client UI systems.

## Required rules

* UI systems should check `System.IsConsoleApp()` where appropriate.
* Server systems should check `Replication.IsServer()` where appropriate.
* Player-local systems should only run for the local controlled entity.
* GameMode systems should not assume a local player exists.
* Radial/map UI should not initialize on dedicated server.

## Systems to review

```text
TFR_OperationsRadialGameModeComponent
TFR_OperationsRadialComponent
TFR_HaloMapSelectionComponent
TFR_OperationsRadialNetworkComponent
TFR_HaloJumpComponent
```

---

# Entity count rules

Entity count can become a performance issue.

## Required rules

Limit:

* spawned enemies;
* spawned escorts;
* spawned reinforcements;
* hunt waves;
* convoy vehicles;
* convoy crews;
* waypoints;
* reward boxes;
* smoke entities;
* persistent patrol vehicles;
* persistent patrol crew;
* artillery impact wrappers.

## Systems to review

```text
TFR_MissionManagerComponent
TFR_SpawnDirectorComponent
TFR_EnemyCommsManagerComponent
TFR_ArtilleryImpactComponent
```

---

# Recommended review order

The recommended performance review order is:

```text
1. Debug defaults and log spam
2. Radial boot/action logs
3. ArtilleryImpact debug default
4. PlayerInventoryPersistence debug/autoscan
5. Persistence autosave interval
6. Mood tick/scavenger settings
7. EnemyComms scan/cooldown settings
8. SpawnDirector persistent patrol settings
9. MissionManager mission check and entity limits
```

Reason:

Start with low-risk cleanup first.
Do not rewrite big systems before removing obvious log/debug/default problems.

---

# Do not do this

Do not optimize everything at once.

Do not change mission logic and persistence logic in the same step.

Do not change folder structure while changing code.

Do not change defaults without checking existing prefabs.

Do not remove diagnostics that are still needed for testing.

Do not disable systems silently without clear warnings.

Do not hide errors that mission makers need to see.

---

# Final performance goal

TFR Framework should be safe to run on dedicated servers for long operations.

The addon should avoid:

* major FPS drops;
* uncontrolled background work;
* excessive entity counts;
* constant scans;
* log spam;
* unsafe persistence writes;
* client UI running on server;
* optional systems consuming resources while unused.

The final goal is simple:

```text
If a system is not being used, it should not cost performance.
If a system is being used, it should have clear limits.
```
