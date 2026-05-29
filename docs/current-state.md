# TFR Framework Current State

This document describes the current state of TFR Framework before starting the main development phases.

---

## Project status

TFR Framework is in the initial planning and foundation stage.

The repository currently contains the basic project description, license information and initial documentation structure.

The addon is intended to become a modular framework for Arma Reforger, designed as a shared foundation for TFR systems, future TFR addons, dedicated servers and community operations.

---

## Current documentation

The repository currently includes:

- `README.md`
- `LICENSE`
- `CHANGELOG.md`
- `docs/roadmap.md`
- `docs/setup.md`
- `docs/components.md`
- `docs/performance.md`

These documents define the general direction of the addon before deeper implementation begins.

---

## Defined development fronts

The project is currently organized around four main fronts:

1. Safe Position / Safe Placement
2. Mission system for mission makers
3. Stable persistence
4. Addon simplification, cleanup and optimization

These fronts define the long-term direction of the framework.

---

## Current priorities

Before starting implementation phases, the current priority is to keep the project clean and well documented.

The framework should avoid becoming fragmented, difficult to configure or heavy on dedicated servers.

The current focus is:

- clear documentation;
- clean repository structure;
- simple setup philosophy;
- modular system planning;
- performance awareness;
- no unnecessary complexity before implementation.

---

## Not implemented yet

The following systems are planned but not yet considered complete:

- Safe placement system;
- mission-maker operation systems;
- persistence system;
- GameMode presets;
- component validation;
- example prefabs;
- performance limits;
- enemy communication limits;
- full setup checklist.

These systems should be added gradually during the development phases.

---

## Development rule

Do not start adding complex systems before the base structure is clear.

The framework must remain:

- easy to configure;
- hard to break;
- modular;
- lightweight;
- suitable for dedicated servers;
- understandable from Workbench;
- documented enough for future users and contributors.
