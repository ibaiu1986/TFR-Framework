# TFR Framework Review Notes

This document is a live working checklist for reviewing the current TFR Framework code.

TFR Framework and TFR Operations are the same addon.

* **TFR Operations** is the internal Workbench project name.
* **TFR Framework** is the public Workshop / repository name.

This document is not a final user guide.

It is used to track what needs review before deeper refactor or expansion.

---

## Review goal

The goal is to make the current addon:

* easier to configure;
* harder to break;
* safer for dedicated servers;
* clearer in Workbench;
* lighter on performance;
* better documented;
* ready for future phases.

This review should support the four roadmap fronts:

1. Safe Position / Safe Placement
2. Mission system for mission makers
3. Stable persistence
4. Simplification, cleanup and optimization

---

## Main rule

Do not rewrite everything at once.

Each review step should be small, isolated and tested.

Recommended process:

```text
1. Pick one review topic.
2. Change only the scripts related to that topic.
3. Compile.
4. Test in Workbench.
5. Test on dedicated server when relevant.
6. Commit.
7. Move to the next topic.
```

---

## Do not do this during review

Do not move folders.

Do not rename classes unless absolutely required.

Do not split large scripts before stabilizing defaults and warnings.

Do not change mission logic and persistence logic at the same time.

Do not change prefabs and scripts in the same step unless necessary.

Do not remove diagnostics that are still needed for testing.

Do not hide real errors.

Do not enable optional systems by default without a reason.

---

# Review order

Recommended review order:

```text
1. Debug/log spam
2. Dangerous or noisy defaults
3. Radial incomplete entries
4. Component validation warnings
5. Persistence safety
6. Player inventory persistence
7. Enemy communications performance
8. Mood performance
9. SpawnDirector performance
10. MissionManager setup complexity
11. HALO validation and dedicated server behaviour
12. Documentation updates after code review
```

---

# 1. Debug / log spam review

## Goal

Normal gameplay should not spam the server console.

Debug logs should be useful when enabled, but quiet when disabled.

## Scripts to review

```text
TFR_OperationsRadialGameModeComponent.c
TFR_OperationsRadialComponent.c
TFR_OperationsRadialController.c
TFR_OperationsRadialEntry.c
TFR_OperationsRadialNetworkComponent.c
TFR_PlayerInventoryPersistenceComponent.c
TFR_ArtilleryImpactComponent.c
TFR_PersistenceManagerComponent.c
```

## Known points

### Radial boot logs

Some radial scripts currently print boot or registration logs directly.

Review:

```text
[TFR Operations Radial BOOT SERVERCHECK]
[TFR Operations Radial Network BOOT]
[TFR Operations Radial] Input listener registered
[TFR Operations Radial] Entry performed
[TFR Operations Radial] Sending radial action through network component
```

Decision needed:

```text
Move normal boot/debug logs behind a Debug Logs attribute.
Keep only critical warnings/errors always visible.
```

### Player inventory persistence logs

`TFR_PlayerInventoryPersistenceComponent` currently uses admin logs for many operations.

Decision needed:

```text
Keep explicit admin diagnostics.
Reduce repeated normal save/restore logs if they spam during autoscan.
Debug should be disabled by default unless testing.
```

### Artillery impact debug default

`TFR_ArtilleryImpactComponent` should not have debug enabled by default.

Decision needed:

```text
Set Debug default to false if it is currently true.
```

## Checklist

```text
[ ] Radial boot logs debug-gated or reduced
[ ] Radial entry normal action logs debug-gated or reduced
[ ] Radial network normal logs debug-gated or reduced
[ ] Input registration logs debug-gated or reduced
[ ] Artillery impact debug disabled by default
[ ] Player inventory debug disabled by default
[ ] Admin diagnostics remain available when explicitly requested
[ ] Errors remain visible
```

---

# 2. Dangerous or noisy defaults review

## Goal

Default values should be safe.

A mission maker should not accidentally create heavy systems just by adding a component.

## Scripts to review

```text
TFR_ArtilleryImpactComponent.c
TFR_PlayerInventoryPersistenceComponent.c
TFR_EnemyCommsManagerComponent.c
TFR_MoodComponent.c
TFR_SpawnDirectorComponent.c
TFR_MissionManagerComponent.c
TFR_PersistenceManagerComponent.c
```

## Known points

### Debug defaults

Review all debug attributes.

Expected default:

```text
false
```

### Autosave / autoscan defaults

Review:

```text
TFR_PersistenceManagerComponent
TFR_PlayerInventoryPersistenceComponent
```

Check that autosave/autoscan intervals are safe for dedicated servers.

### Optional systems

Optional systems should not create load when unused.

Review:

```text
EnemyComms
Mood/scavenger
SpawnDirector persistent patrols
Player inventory persistence
Radial admin entries
```

## Checklist

```text
[ ] Debug defaults reviewed
[ ] Autosave intervals reviewed
[ ] Autoscan intervals reviewed
[ ] Scan intervals reviewed
[ ] Persistent patrol defaults reviewed
[ ] EnemyComms defaults reviewed
[ ] Mood/scavenger defaults reviewed
[ ] Mission enemy counts reviewed
[ ] Convoy vehicle count limits reviewed
[ ] Reinforcement wave defaults reviewed
```

---

# 3. Radial incomplete entries review

## Goal

The radial should only expose working entries.

## Scripts to review

```text
TFR_OperationsRadialEntry.c
TFR_OperationsRadialComponent.c
TFR_OperationsRadialNetworkComponent.c
TFR_PlayerInventoryPersistenceComponent.c
```

## Known point

`TFR_OperationsRadialEntry.c` declares player inventory entries:

```text
PLAYER_INVENTORY_SAVE_NOW
PLAYER_INVENTORY_RESTORE_NOW
PLAYER_INVENTORY_PRINT_STATE
PLAYER_INVENTORY_RESET
```

But `TFR_OperationsRadialNetworkComponent.c` appears to handle mission and operation persistence actions only.

## Decision needed

Choose one:

```text
Option A: implement player inventory radial actions through TFR_OperationsRadialNetworkComponent.
Option B: hide/remove player inventory entries until the network side is ready.
```

Recommended first step:

```text
Hide entries until fully implemented.
```

## Checklist

```text
[ ] Confirm whether player inventory entries are displayed
[ ] Confirm whether network handles player inventory actions
[ ] Hide unimplemented entries or implement them
[ ] Add clear message if feature is disabled
[ ] Test radial after change
```

---

# 4. Component validation warnings review

## Goal

When setup is incomplete, logs should say exactly what is missing.

Bad warning:

```text
ERROR
```

Good warning:

```text
TFR_MissionManagerComponent: Intel Civil Prefab is empty. Informant intel jobs cannot spawn.
```

## Scripts to review

```text
TFR_MissionManagerComponent.c
TFR_PersistenceManagerComponent.c
TFR_PlayerInventoryPersistenceComponent.c
TFR_HaloJumpComponent.c
TFR_HaloMapSelectionComponent.c
TFR_OperationsRadialGameModeComponent.c
TFR_OperationsRadialComponent.c
TFR_SpawnDirectorComponent.c
TFR_EnemyCommsManagerComponent.c
TFR_InteractionComponent.c
TFR_JournalTaskBridgeComponent.c
```

## Checklist

```text
[ ] Missing GameMode component warnings are clear
[ ] Missing prefab warnings include field name
[ ] Missing LayerTask warning is clear
[ ] Missing PlayerController/network bridge warning is clear
[ ] Missing MissionManager warning is clear
[ ] Missing PersistenceManager warning is clear
[ ] Missing player inventory component warning is clear
[ ] Missing spawn areas warning is clear
[ ] Missing HALO component warning is clear
```

---

# 5. Persistence safety review

## Goal

Operation persistence should be stable and should not overwrite or restore bad state.

## Scripts to review

```text
TFR_PersistenceManagerComponent.c
TFR_MissionManagerComponent.c
TFR_MissionSettingAction.c
TFR_OperationsRadialNetworkComponent.c
```

## Main risks

* autosave too frequent;
* loading before MissionManager is ready;
* missing save file;
* corrupted save file;
* reset actions used accidentally;
* restore state incomplete;
* convoy route restore incomplete;
* admin logs too noisy.

## Checklist

```text
[ ] Autosave interval has safe minimum
[ ] Load delay has safe default
[ ] Missing save file handled safely
[ ] Missing MissionManager handled safely
[ ] Restore operation does not crash if fields are missing
[ ] Reset actions clearly separated
[ ] Reset all can optionally restart operation
[ ] Admin print state is useful
[ ] Save paths are clear
[ ] Save directory fallback works
```

---

# 6. Player inventory persistence review

## Goal

Player inventory persistence should protect good saves and avoid heavy scans.

## Scripts to review

```text
TFR_PlayerInventoryPersistenceComponent.c
TFR_PersistenceManagerComponent.c
TFR_MissionSettingAction.c
TFR_OperationsRadialNetworkComponent.c
```

## Main risks

* autoscan too frequent;
* save with no valid controlled players;
* restore before player entity exists;
* restoring unsafe slots;
* too many items restored;
* duplicate items;
* debug/admin log spam;
* JSON save too large.

## Checklist

```text
[ ] Debug disabled by default
[ ] Autoscan interval safe
[ ] Skip save when no valid players enabled
[ ] Restore blocks save while pending
[ ] Restore retry count limited
[ ] Restore retry delay safe
[ ] Max restore items per player limited
[ ] Loadout restore disabled by default
[ ] Equipment storage restore disabled by default
[ ] Duplicate entity skip enabled
[ ] Failed restore attempts logged clearly
[ ] Admin print state available
```

---

# 7. Enemy communications performance review

## Goal

Enemy communications should add dynamic behaviour without becoming an expensive global scanner.

## Scripts to review

```text
TFR_EnemyCommsManagerComponent.c
TFR_MoodComponent.c
TFR_ArtilleryImpactComponent.c
```

## Main risks

* scan radius too large;
* scan interval too short;
* too many responders;
* repeated reinforcements;
* CAS/artillery spam;
* alerting too many AI;
* debug spam.

## Checklist

```text
[ ] Run server only by default
[ ] Debug disabled by default
[ ] Scan interval safe
[ ] Scan radius safe
[ ] Alert radius safe
[ ] Maximum responders limited
[ ] Reinforcement cooldown safe
[ ] CAS/artillery cooldown safe
[ ] Support calls cannot spam
[ ] No scan when disabled
[ ] CallLater cleaned in OnDelete
```

---

# 8. Mood performance review

## Goal

Mood should not create uncontrolled per-NPC overhead.

## Scripts to review

```text
TFR_MoodComponent.c
TFR_EnemyCommsManagerComponent.c
```

## Main risks

* too many NPCs with mood component;
* mood tick too frequent;
* passive stimulus too frequent;
* scavenger tick too frequent;
* ground weapon radius too large;
* repeated inventory insert attempts;
* faction switching spam;
* debug spam.

## Checklist

```text
[ ] Run server only by default
[ ] Debug disabled by default
[ ] Mood tick interval safe
[ ] Passive stimulus interval safe
[ ] Scavenger tick interval safe
[ ] Scavenger radius safe
[ ] Failed scavenger retry works
[ ] Scavenger stops after weapon assigned
[ ] Hostile lock has clear duration
[ ] CallLater cleaned in OnDelete
[ ] Documentation warns not to use on every NPC blindly
```

---

# 9. SpawnDirector performance review

## Goal

SpawnDirector should not create uncontrolled entity growth or heavy patrol loops.

## Scripts to review

```text
TFR_SpawnDirectorComponent.c
TFR_SpawnAreaComponent.c
```

## Main risks

* too many spawn rules;
* too many vehicle rules;
* too many crews;
* too many waypoints;
* persistent patrol checks too frequent;
* respawn loops;
* route refresh too frequent;
* world patrol destination attempts too high.

## Checklist

```text
[ ] Run server only by default
[ ] Debug disabled by default
[ ] Auto spawn intentional
[ ] Spawn counts limited
[ ] Vehicle counts limited
[ ] Crew counts limited
[ ] Waypoint counts limited
[ ] Persistent patrol check safe
[ ] Route refresh safe
[ ] Respawn delay safe
[ ] World patrol attempts limited
[ ] Cleanup on delete works
```

---

# 10. MissionManager setup complexity review

## Goal

MissionManager should be easier to configure and harder to break.

## Scripts to review

```text
TFR_MissionManagerComponent.c
TFR_JournalTaskBridgeComponent.c
TFR_InteractionComponent.c
TFR_CivilDataComponent.c
TFR_Action_Arrestar.c
TFR_Action_Interrogar.c
TFR_Action_Identificar.c
```

## Main risks

* too many fields;
* missing prefab fields;
* unclear required/optional settings;
* mission type fails silently;
* spawn areas missing;
* journal bridge missing/misconfigured;
* restore hooks incomplete;
* convoy setup too fragile;
* reinforcements/hunt waves too heavy.

## Checklist

```text
[ ] Required prefab warnings clear
[ ] Missing spawn area warning clear
[ ] Missing JournalBridge is not fatal
[ ] Mission types fail safely
[ ] Convoy setup validates all required prefabs
[ ] Reinforcement limits safe
[ ] Hunt wave limits safe
[ ] Mission check loop controlled
[ ] Admin diagnostics clear
[ ] Restore hooks tested
[ ] Workbench field names understandable
```

---

# 11. HALO validation and dedicated server review

## Goal

HALO should work reliably on dedicated server without unsafe client execution.

## Scripts to review

```text
TFR_HaloJumpComponent.c
TFR_HaloMapSelectionComponent.c
TFR_HaloSpawnPointComponent.c
TFR_HaloJumpUserAction.c
TFR_OperationsRadialEntry.c
```

## Main risks

* client/server mismatch;
* map selection callback not cleaned;
* server response timeout;
* teleport not applied reliably;
* parachute not deployed;
* no controlled entity;
* no parachute component;
* invalid selected position.

## Checklist

```text
[ ] Server validates jump request
[ ] Client only selects/request position
[ ] Map callbacks cleaned
[ ] Response timeout works
[ ] Owner assist limited
[ ] Teleport retry limited
[ ] Auto deploy retry limited
[ ] Missing parachute warning clear
[ ] HALO spawn point fallback works
[ ] Dedicated server tested
```

---

# 12. Documentation update after review

## Goal

Docs should match the real code after each review step.

## Docs to update when needed

```text
docs/setup.md
docs/components.md
docs/component-placement.md
docs/performance.md
docs/architecture.md
docs/phases.md
docs/review-notes.md
CHANGELOG.md
```

## Checklist

```text
[ ] Update docs after changing defaults
[ ] Update docs after hiding/adding radial entries
[ ] Update docs after changing persistence behaviour
[ ] Update docs after changing component placement requirements
[ ] Update docs after adding warnings
[ ] Update changelog after meaningful changes
```

---

# Current first task

The first code review task should be:

```text
Debug/log spam cleanup
```

Why:

* low risk;
* easy to test;
* directly improves dedicated server readability;
* does not change mission flow;
* does not change persistence logic;
* does not require moving files.

Recommended first scripts:

```text
TFR_ArtilleryImpactComponent.c
TFR_PlayerInventoryPersistenceComponent.c
TFR_OperationsRadialGameModeComponent.c
TFR_OperationsRadialComponent.c
TFR_OperationsRadialController.c
TFR_OperationsRadialEntry.c
TFR_OperationsRadialNetworkComponent.c
```

---

# Review status

```text
[ ] Debug/log spam cleanup
[ ] Dangerous defaults review
[ ] Radial incomplete entries review
[ ] Component validation warnings review
[ ] Persistence safety review
[ ] Player inventory persistence review
[ ] Enemy communications performance review
[ ] Mood performance review
[ ] SpawnDirector performance review
[ ] MissionManager setup complexity review
[ ] HALO validation and dedicated server review
[ ] Documentation update after review
```

---

## Final rule

Each review must leave the addon cleaner than before.

If a system is confusing, heavy or fragile, the fix should be:

```text
better defaults
better warnings
better limits
better documentation
simpler setup
```

not more hidden complexity.
