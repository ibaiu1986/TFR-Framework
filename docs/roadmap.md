# TFR Framework Roadmap

This document defines the current development plan for **TFR Framework / TFR Operations**.

TFR Framework must be a publicable, modular Arma Reforger framework with:

- Core modular.
- No hard dependencies on ACE, RHS or Parachute Framework.
- Radial compatible with ACE/RHS/vanilla.
- Centralized Safe Position / safe placement.
- Improved Spawn Director.
- AmbientAI integrated as configurable systems.
- Clean dynamic missions.
- Ordered persistence.
- **No loose GenericEntities as gameplay configuration.**
- **Reduced number of components required from mission makers.**
- Clear TFR scripted entities/prefabs for mission makers.
- Controlled logs.
- Easy GameMode setup.

---

## General rules

- Do not delete working code.
- Do not rewrite systems blindly.
- Do not add hard dependencies for optional integrations.
- Do not use `modded SCR_BaseGameMode` for TFR startup logic.
- Do not use loose GenericEntities with magic names as required gameplay configuration.
- Do not force mission makers to manually stack many TFR components on random entities.
- Existing working TFR scripts are treated as the current good source and must be improved incrementally.
- New systems must be server-safe, modular and easy to disable.

---

## Front 1 — Safe Position / Safe Placement

Objective: ensure valid and safe positions for spawning, deploying or placing entities, avoiding broken, dangerous or invalid locations.

Planned technical work:

1. Create a central safe-position helper/component.
2. Support surface snapping, zero-vector validation, min/max radius search and terrain checks.
3. Support infantry-safe and vehicle-safe positions.
4. Support road-near placement for traffic, convoys and parked vehicles.
5. Integrate it gradually into HALO, SpawnDirector, MissionManager, AmbientAI and reinforcements.
6. Add building-interior safe placement validation so units do not spawn inside walls, blocked rooms or invalid floor space.
7. Add road direction detection so vehicles can align with road heading instead of spawning at random rotation.

---

## Front 2 — Mission System for Mission Makers

Objective: provide clean and reusable tools for creating operations and missions.

Planned technical work:

1. Keep `TFR_MissionManagerComponent` as the current mission orchestrator.
2. Add generic objective state and objective registry systems.
3. Add reusable area objective logic.
4. Convert special mission concepts such as Police Station and Village Patrol into configurable TFR objective systems.
5. Connect mission objectives with JournalBridge, rewards, persistence and reinforcements.
6. Prefer ready-to-place scripted TFR entities/prefabs over requiring mission makers to manually add many components.

---

## Front 3 — Stable Persistence

Objective: clean, stable and efficient persistence.

Current persistence must remain based on TFR persistence components and `SCR_JsonSaveContext` / `SCR_JsonLoadContext`.

Planned technical work:

1. Keep operation persistence stable.
2. Keep player inventory persistence separated.
3. Add objective persistence without breaking existing `operation_state.json`.
4. Store completed/active objectives in a generic format.
5. Do not persist runtime AmbientAI or traffic by default.
6. Keep reset actions separated: operation, inventory, vehicle cargo, objective state and full reset.

---

## Front 4 — Simplification, Cleanup and Optimization

Objective: make TFR Framework / TFR Operations easy to configure, difficult to break and lightweight on dedicated servers.

This front includes:

- fewer mandatory components;
- fewer manually placed components;
- fewer critical fields spread across different places;
- prepared scripted entities and prefabs;
- prepared presets and GameModes;
- functional example prefabs;
- separated minimal and full configuration modes;
- clear names in Workbench;
- safe default values;
- useful validation warnings;
- logs that explain exactly what is missing;
- short documentation for each TFR entity/prefab;
- a setup checklist;
- avoiding strong FPS drops;
- no unnecessary scans;
- no duplicated `CallLater` loops;
- no debug spam;
- optional systems disabled when not used;
- clear limits for persistence, EnemyComms, AmbientAI and reinforcements.

The final goal of Front 4 is that a mission maker should be able to use:

1. `TFR_GameMode_Core.et`
2. `TFR_GameMode_Operations.et`
3. `TFR_GameMode_FullPersistent.et`
4. `TFR_GameMode_AmbientWarzone.et`

and place clear TFR entities such as `TFR_Area_Town.et`, `TFR_Area_Road.et`, `TFR_Area_Military.et` or `TFR_Area_Compound.et` without having to guess which components belong on the GameMode, which ones belong on the character and which ones are optional.

---

## Additional current work streams

### Radial compatibility

The TFR radial must remain independent from vanilla, ACE and RHS radial/menu systems.

Required direction:

- use the dedicated TFR radial menu;
- never depend on `SCR_RadialMenu.GlobalRadialMenu()`;
- avoid stealing input while another menu/radial is open;
- keep fallback admin actions on physical consoles/radios;
- move normal radial logs behind debug toggles;
- keep warnings only for real setup problems.

### Area based spawn rules and environment scanner

TFR spawning must be based on **scripted TFR area entities + rules + environment detection**, not on loose GenericEntities, magic entity names or manual component stacking.

Required direction:

- mission makers place ready TFR area entities/prefabs, such as `TFR_Area_Town.et`, `TFR_Area_Road.et`, `TFR_Area_Military.et`, `TFR_Area_Compound.et` or `TFR_Area_Checkpoint.et`;
- each area entity owns its configuration in script: type, radius, enabled state, optional tags, allowed rule sets and scan settings;
- mission makers should not need to manually add multiple components to make an area work;
- internally, the area entity may use one main script class and optional private helper classes, but the Workbench-facing setup must stay simple;
- `TFR_SpawnDirectorComponent` reads registered TFR area entities and applies spawn rules;
- area rules decide what is allowed inside the area: civilians, patrols, parked vehicles, traffic, QRF, enemies, loot or objectives;
- the framework scans the actual environment inside the area before spawning anything;
- entity names are not gameplay logic.

Planned scripted area classes/prefabs:

- `TFR_AreaEntity` / base scripted area entity;
- `TFR_Area_Town.et`;
- `TFR_Area_Road.et`;
- `TFR_Area_Military.et`;
- `TFR_Area_Industrial.et`;
- `TFR_Area_Checkpoint.et`;
- `TFR_Area_Base.et`;
- `TFR_Area_Compound.et`.

The planned scanner should detect and cache:

- roads;
- road direction / road heading;
- road-side positions;
- road intersections;
- buildings;
- building floors;
- valid building interior points;
- blocked interior points;
- doors or likely entrances when detectable;
- walls / compounds when detectable;
- signs;
- utility poles / electric poles;
- open ground;
- safe infantry positions;
- safe vehicle positions;
- parking-like positions;
- cover-like urban positions.

Spawn rule examples:

- Civilian rule: prefer building entrances, streets, courtyards and safe interior points.
- Building garrison rule: use detected floors and interior-safe points, never wall-intersection points.
- Parked vehicle rule: prefer road-side positions aligned to road heading.
- Traffic rule: use road direction and avoid cross-road static spawns.
- Patrol rule: generate patrol points between buildings, roads and open ground within the same area.
- Enemy/QRF rule: prefer area edges, roads, compounds or military buildings depending on rule type.

This system must cache scan results and avoid heavy continuous scans. Scanning should happen on startup, on demand, or in limited delayed batches.

### Marker and point configuration

Point markers are only for special exact gameplay points, not for general ambient spawning.

Allowed point-marker use cases:

- exact HALO DZ;
- main base;
- extraction point;
- fixed objective point;
- manually forced reinforcement start;
- manually forced route point.

General civilians, vehicles, traffic, patrols and ambient systems must use scripted TFR area entities, area detection and rules instead.

### Spawn Director

`TFR_SpawnDirectorComponent` is the intended spine for rule-based area spawning, vehicle spawning, persistent patrols and future AmbientAI/reinforcement support.

Required direction:

- keep rules configurable;
- use scripted TFR area entities, not magic names;
- avoid requiring mission makers to manually attach many components;
- add safe-position integration;
- add area environment scanner integration;
- use road heading for vehicle spawn orientation;
- use building floor/interior data for garrison and civilian interior placement;
- keep spawn limits and server-only defaults;
- keep persistent patrol checks controlled by `CallLater`, not `EOnFrame`.

---

## Definitive implementation order

1. Convert AmbientAI to an area + rules + scanner system.
2. Harden the TFR radial for ACE/RHS/vanilla compatibility and keep physical-action fallback.
3. Create the centralized Safe Position system.
4. Create the base TFR Area Entity and registry for owned TFR areas.
5. Create the Environment Scanner for roads, road heading, buildings, floors and safe interior points.
6. Integrate SpawnDirector with areas, scanner, safe position and rules.
7. Convert Police Station and Village Patrol into configurable TFR objectives.
8. Add Objective State and objective persistence.
9. Create a Reinforcement/QRF Manager connected to SpawnDirector, EnemyComms and missions.
10. Polish Mood/EnemyComms with performance limits.
11. Prepare TFR-owned prefabs for areas, GameModes, console, HALO and objectives.
12. Reduce required components and clean up setup for mission makers.
13. Document minimal setup, full setup and checklist.
14. Run dedicated-server tests, ACE/RHS/vanilla tests and final optimization.
15. Re-check and harden the TFR radial again after integration, keeping ACE/RHS/vanilla compatibility and physical-action fallback.
16. Prepare changelog and release candidate.
