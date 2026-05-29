# TFR Framework Development Phases

This document defines the initial development phases for TFR Framework.

The goal is to organize the work before adding or refactoring systems, so the addon does not become fragmented, heavy or difficult to configure.

---

## Phase 0 — Repository and documentation base

Goal: leave the repository clean, documented and ready before implementation.

Status: Mostly done.

Includes:

- README;
- LICENSE;
- CHANGELOG;
- roadmap documentation;
- current state documentation;
- setup documentation;
- component philosophy;
- performance guidelines.

This phase must be completed before starting deeper implementation work.

---

## Phase 1 — Core framework foundation

Goal: define the minimal base structure of the framework.

This phase should prepare the common foundation used by future systems.

Includes:

- base folder structure;
- naming conventions;
- basic script organization;
- shared helpers;
- framework initialization rules;
- basic validation philosophy;
- first GameMode structure planning.

The goal is not to add big systems yet, but to make sure the base is clean.

---

## Phase 2 — Safe Position / Safe Placement

Related front: Front 1.

Goal: implement safe position logic for spawning, deployment and entity placement.

This phase should focus on avoiding invalid, dangerous or broken placement positions.

Includes:

- safe spawn position checks;
- safe deployment position checks;
- basic ground validation;
- obstacle avoidance;
- water / invalid area checks where needed;
- reusable helpers for future systems.

---

## Phase 3 — Mission and operations systems

Related front: Front 2.

Goal: create reusable systems for operations, objectives and mission flow.

This phase should focus on mission-maker tools and TFR Operations logic.

Includes:

- operation flow;
- objective logic;
- intel chains;
- mission state handling;
- reusable mission utilities;
- simple configuration rules.

---

## Phase 4 — Stable persistence

Related front: Front 3.

Goal: implement persistence in a clean, limited and server-friendly way.

Persistence must be stable, predictable and lightweight.

Includes:

- clear save rules;
- clear load rules;
- limited save frequency;
- controlled stored data;
- safe defaults;
- useful warnings;
- dedicated server performance limits.

---

## Phase 5 — Simplification, presets and optimization

Related front: Front 4.

Goal: simplify setup, prepare GameMode presets and optimize the addon.

This phase focuses on making the framework easy to configure, difficult to break and lightweight on dedicated servers.

Includes:

- fewer mandatory components;
- fewer critical fields spread across different places;
- prepared GameMode presets;
- functional example prefabs;
- separated minimal and full configuration modes;
- clear names in Workbench;
- safe default values;
- useful validation warnings;
- logs that explain exactly what is missing;
- short documentation for each component;
- setup checklist;
- no unnecessary scans;
- no duplicated `CallLater` loops;
- no debug spam;
- optional systems disabled when not used.

Planned GameMode presets:

1. `TFR_GameMode_Core.et`
2. `TFR_GameMode_Operations.et`
3. `TFR_GameMode_FullPersistent.et`

The final goal is that a mission maker should not have to guess which components belong on the GameMode, which ones belong on the character and which ones are optional.

---

## Development rule

Do not start adding complex systems before the current phase is clear.

Each phase should leave the framework cleaner than before.
