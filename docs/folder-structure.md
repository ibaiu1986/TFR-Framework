# TFR Framework Folder Structure

This document defines the target script folder structure for TFR Framework.

TFR Framework and TFR Operations are the same addon.
**TFR Operations** is the internal Workbench project name.
**TFR Framework** is the public Workshop / repository name.

The goal of this structure is to make the addon easier to understand, easier to maintain and harder to break.

---

## Important rule

Do not move scripts randomly.

Moving scripts in Arma Reforger Workbench can break references, prefabs or dependencies if done too quickly.

The correct process is:

1. Document the target structure.
2. Compare it with the current real folders.
3. Move one block at a time.
4. Compile after each block.
5. Test in Workbench after each block.
6. Do not refactor logic while moving files.

Folder cleanup and code refactor must be separate steps.

---

## Target script structure

Recommended target structure:

```text
scripts/Game/TFR/
├─ Actions/
├─ Admin/
├─ Components/
├─ Core/
├─ HALO/
├─ Missions/
├─ Persistence/
├─ Radial/
├─ Spawn/
└─ Utilities/
```

---

## Actions

Target folder:

```text
scripts/Game/TFR/Actions/
```

Purpose:

User actions attached to entities through an ActionManagerComponent.

Current related scripts:

```text
TFR_Action_Arrestar
TFR_Action_Identificar
TFR_Action_Interrogar
TFR_HaloJumpUserAction
TFR_MissionAdminActions
TFR_MissionSettingAction
```

Rules:

* Actions should stay lightweight.
* Actions should not contain heavy mission logic.
* Actions should call components or managers.
* Actions should have clear names in Workbench.
* Actions should provide useful cannot-perform reasons when possible.

---

## Admin

Target folder:

```text
scripts/Game/TFR/Admin/
```

Purpose:

Admin access, admin consoles, diagnostics and safe server-side admin actions.

Current related scripts:

```text
TFR_MissionAdminConsoleComponent
TFR_MissionAdminActions
TFR_MissionSettingAction
```

Possible future scripts:

```text
TFR_AdminPermissionsComponent
TFR_AdminDiagnosticsComponent
TFR_AdminActionHelpers
```

Rules:

* Admin systems must validate permissions.
* Admin systems must avoid client-side sensitive execution.
* Admin systems should print useful diagnostics.
* Admin systems should not spam logs during normal gameplay.

---

## Components

Target folder:

```text
scripts/Game/TFR/Components/
```

Purpose:

General reusable components that do not clearly belong to a more specific folder.

Current related scripts:

```text
TFR_CivilDataComponent
TFR_InteractionComponent
TFR_MoodComponent
TFR_SurrenderControlComponent
TFR_ArtilleryImpactComponent
```

Rules:

* Component purpose must be clear.
* Component placement must be documented.
* Optional components should be disabled when unused.
* Debug should be disabled by default.
* Component names should be clear in Workbench.

---

## Core

Target folder:

```text
scripts/Game/TFR/Core/
```

Purpose:

Shared framework base, common helpers, validation, logging and global utility logic.

Current state:

No dedicated core layer has been separated yet.

Possible future scripts:

```text
TFR_Log
TFR_Validation
TFR_GameModeHelper
TFR_ComponentHelper
TFR_ServerHelper
TFR_PositionHelper
```

Rules:

* Core should not depend on mission-specific systems when possible.
* Core should provide reusable helpers.
* Core should reduce duplicated code.
* Core should avoid becoming a dumping ground.

---

## HALO

Target folder:

```text
scripts/Game/TFR/HALO/
```

Purpose:

HALO insertion systems.

Current related scripts:

```text
TFR_HaloJumpComponent
TFR_HaloMapSelectionComponent
TFR_HaloSpawnPointComponent
TFR_HaloJumpUserAction
```

Rules:

* HALO server execution must stay server-safe.
* Map selection belongs to client/player flow.
* Actual jump execution must be validated by server.
* No heavy loops.
* Retry flows must stay limited.
* Dedicated server behaviour must remain tested.

---

## Missions

Target folder:

```text
scripts/Game/TFR/Missions/
```

Purpose:

Dynamic operation flow, intel chains, mission types, rewards and journal bridge.

Current related scripts:

```text
TFR_MissionManagerComponent
TFR_JournalTaskBridgeComponent
TFR_InteractionComponent
TFR_CivilDataComponent
TFR_MoodComponent
```

Rules:

* Mission logic should stay server-side where needed.
* MissionManager should be reviewed carefully because it is the largest system.
* Journal updates should stay isolated through the journal bridge.
* Mission state should be clear for persistence.
* Mission systems should not require users to guess setup.

---

## Persistence

Target folder:

```text
scripts/Game/TFR/Persistence/
```

Purpose:

Operation persistence, player inventory persistence and future vehicle cargo persistence.

Current related scripts:

```text
TFR_PersistenceManagerComponent
TFR_PlayerInventoryPersistenceComponent
TFR_MissionSettingAction
```

Possible future scripts:

```text
TFR_VehicleCargoPersistenceComponent
TFR_PersistencePaths
TFR_PersistenceDiagnostics
```

Rules:

* Persistence must be server-friendly.
* Autosave intervals must be safe.
* Empty player states must not overwrite valid saves.
* Restore retries must be controlled.
* Save files must be clearly separated.
* Admin reset functions must be safe.

---

## Radial

Target folder:

```text
scripts/Game/TFR/Radial/
```

Purpose:

Dedicated TFR Operations radial menu and radial network actions.

Current related scripts:

```text
TFR_OperationsRadialMenu
TFR_OperationsRadialController
TFR_OperationsRadialGameModeComponent
TFR_OperationsRadialComponent
TFR_OperationsRadialEntry
TFR_OperationsRadialNetworkComponent
```

Rules:

* The TFR radial should not use the global vanilla radial menu.
* Sensitive actions must go through server validation.
* Client UI must not run on dedicated server.
* Input registration must be cleaned up.
* Radial entries should only expose implemented systems.

Known review point:

Player inventory radial entries are declared but must be checked against the network implementation.
They should either be implemented fully or hidden until ready.

---

## Spawn

Target folder:

```text
scripts/Game/TFR/Spawn/
```

Purpose:

Spawn areas, spawn director, safe placement and map-independent placement systems.

Current related scripts:

```text
TFR_SpawnAreaComponent
TFR_SpawnDirectorComponent
TFR_HaloSpawnPointComponent
```

Rules:

* The addon must remain map-independent.
* Areas should be configured through placed components.
* No continuous scans unless required.
* Spawn logic must avoid unsafe positions where possible.
* SpawnDirector must stay server-friendly.
* Persistent patrol checks must be limited.

---

## Utilities

Target folder:

```text
scripts/Game/TFR/Utilities/
```

Purpose:

Small reusable helpers that are not tied to one gameplay system.

Possible future scripts:

```text
TFR_ArrayUtils
TFR_FactionUtils
TFR_EntityUtils
TFR_InventoryUtils
TFR_GridUtils
TFR_RandomUtils
```

Rules:

* Utility scripts should be small.
* Utility scripts should avoid hidden side effects.
* Utility scripts should not hold mission state.
* Utility scripts should be reusable across systems.

---

## Suggested move order

To reduce risk, move scripts in this order:

### Step 1 — Actions

Move action scripts first because they are mostly isolated.

```text
TFR_Action_Arrestar
TFR_Action_Identificar
TFR_Action_Interrogar
TFR_HaloJumpUserAction
TFR_MissionAdminActions
TFR_MissionSettingAction
```

Compile after this step.

---

### Step 2 — Radial

Move radial scripts next.

```text
TFR_OperationsRadialMenu
TFR_OperationsRadialController
TFR_OperationsRadialGameModeComponent
TFR_OperationsRadialComponent
TFR_OperationsRadialEntry
TFR_OperationsRadialNetworkComponent
```

Compile and test radial opening after this step.

---

### Step 3 — Spawn

Move spawn-related scripts.

```text
TFR_SpawnAreaComponent
TFR_SpawnDirectorComponent
TFR_HaloSpawnPointComponent
```

Compile and check that spawn areas still register.

---

### Step 4 — HALO

Move HALO logic.

```text
TFR_HaloJumpComponent
TFR_HaloMapSelectionComponent
TFR_HaloJumpUserAction
TFR_HaloSpawnPointComponent
```

Compile and test HALO map selection.

Note:

If `TFR_HaloJumpUserAction` or `TFR_HaloSpawnPointComponent` were already moved in previous steps, do not duplicate them.

---

### Step 5 — Persistence

Move persistence systems.

```text
TFR_PersistenceManagerComponent
TFR_PlayerInventoryPersistenceComponent
TFR_MissionSettingAction
```

Compile and test admin persistence actions after this step.

---

### Step 6 — Missions

Move mission systems last because they have the most dependencies.

```text
TFR_MissionManagerComponent
TFR_JournalTaskBridgeComponent
TFR_InteractionComponent
TFR_CivilDataComponent
TFR_MoodComponent
TFR_SurrenderControlComponent
TFR_MissionAdminConsoleComponent
```

Compile and test mission startup after this step.

---

## Do not do this yet

Do not rename classes yet.

Do not rewrite logic while moving files.

Do not change attributes while moving files.

Do not change GameMode setup while moving files.

Do not optimize systems during the folder move.

The first goal is only to make the file structure clearer without changing behaviour.

---

## Final goal

The final goal of the folder structure cleanup is that a developer or mission maker can open the addon and immediately understand where each system belongs.

The structure should support the four roadmap fronts:

1. Safe Position / Safe Placement
2. Mission system for mission makers
3. Stable persistence
4. Simplification, cleanup and optimization

Each folder should make the addon easier to configure, easier to debug and safer for dedicated servers.

